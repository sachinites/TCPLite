#include "tcp_lite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

// Global socket table
static tcp_socket_t socket_table[MAX_SOCKETS];
static int initialized = 0;

// Initialize the TCP stack
void tcp_init(void) {
    if (initialized) return;
    
    memset(socket_table, 0, sizeof(socket_table));
    for (int i = 0; i < MAX_SOCKETS; i++) {
        socket_table[i].fd = -1;
        socket_table[i].is_used = 0;
    }
    
    srand(time(NULL));
    initialized = 1;
}

// Calculate checksum (16-bit one's complement)
uint16_t tcp_checksum(const void *buf, size_t len) {
    const uint16_t *ptr = buf;
    uint32_t sum = 0;
    
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    
    if (len == 1) {
        sum += *(uint8_t *)ptr;
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}

// IP checksum (same algorithm)
uint16_t ip_checksum(const void *buf, size_t len) {
    return tcp_checksum(buf, len);
}

// Find free socket slot
static int find_free_socket(void) {
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (!socket_table[i].is_used) {
            return i;
        }
    }
    return -1;
}


// Send TCP packet
static int send_tcp_packet(tcp_socket_t *sock, uint8_t flags, 
                          const void *data, size_t data_len) {
    char packet[sizeof(struct ip_header) + sizeof(struct tcp_header) + data_len];
    struct ip_header *iph = (struct ip_header *)packet;
    struct tcp_header *tcph = (struct tcp_header *)(packet + sizeof(struct ip_header));
    
    // Clear packet
    memset(packet, 0, sizeof(struct ip_header) + sizeof(struct tcp_header));
    
    // Fill IP header
    iph->version_ihl = 0x45;  // IPv4, IHL = 5 (20 bytes)
    iph->tos = 0;
    iph->total_length = htons(sizeof(struct ip_header) + sizeof(struct tcp_header) + data_len);
    iph->id = htons(rand() % 65536);
    iph->frag_offset = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    iph->src_addr = sock->local_addr.sin_addr.s_addr;
    iph->dst_addr = sock->remote_addr.sin_addr.s_addr;
    iph->checksum = 0;
    iph->checksum = ip_checksum(iph, sizeof(struct ip_header));
    
    // Fill TCP header
    tcph->src_port = sock->local_addr.sin_port;
    tcph->dst_port = sock->remote_addr.sin_port;
    tcph->seq_num = htonl(sock->send_seq);
    tcph->ack_num = htonl(sock->recv_seq);
    tcph->data_offset = 0x50;  // 5 * 4 = 20 bytes
    tcph->flags = flags;
    tcph->window = htons(TCP_WINDOW_SIZE);
    tcph->checksum = 0;
    tcph->urgent_ptr = 0;
    
    // Copy data if any
    if (data_len > 0 && data != NULL) {
        memcpy(packet + sizeof(struct ip_header) + sizeof(struct tcp_header), 
               data, data_len);
    }
    
    // Calculate TCP checksum with pseudo header
    struct pseudo_header psh;
    psh.src_addr = sock->local_addr.sin_addr.s_addr;
    psh.dst_addr = sock->remote_addr.sin_addr.s_addr;
    psh.zero = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcp_header) + data_len);
    
    size_t pseudo_packet_len = sizeof(struct pseudo_header) + sizeof(struct tcp_header) + data_len;
    char *pseudo_packet = malloc(pseudo_packet_len);
    memcpy(pseudo_packet, &psh, sizeof(struct pseudo_header));
    memcpy(pseudo_packet + sizeof(struct pseudo_header), tcph, sizeof(struct tcp_header) + data_len);
    
    tcph->checksum = tcp_checksum(pseudo_packet, pseudo_packet_len);
    free(pseudo_packet);
    
    // Send packet
    int ret = sendto(sock->fd, packet, sizeof(struct ip_header) + sizeof(struct tcp_header) + data_len,
                     0, (struct sockaddr *)&sock->remote_addr, sizeof(sock->remote_addr));
    
    if (ret < 0) {
        perror("sendto failed");
        return -1;
    }
    
    return ret;
}

// Receive TCP packet with timeout
static int recv_tcp_packet(tcp_socket_t *sock, struct tcp_header *tcph, 
                          uint8_t *data, size_t *data_len, int timeout_sec) {
    char buffer[65536];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);
    
    // Set receive timeout
    struct timeval tv;
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    setsockopt(sock->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    while (1) {
        int ret = recvfrom(sock->fd, buffer, sizeof(buffer), 0,
                          (struct sockaddr *)&src_addr, &addr_len);
        
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return -2;  // Timeout
            }
            return -1;
        }
        
        struct ip_header *iph = (struct ip_header *)buffer;
        int ip_header_len = (iph->version_ihl & 0x0F) * 4;
        
        if (ret < (int)(ip_header_len + sizeof(struct tcp_header))) {
            continue;  // Packet too small
        }
        
        struct tcp_header *recv_tcph = (struct tcp_header *)(buffer + ip_header_len);
        
        // Check if packet is for this socket
        if (sock->listening) {
            // For listening sockets, only check destination port
            if (recv_tcph->dst_port != sock->local_addr.sin_port) {
                continue;
            }
        } else {
            // For connected sockets, check both ports and addresses
            if (recv_tcph->dst_port != sock->local_addr.sin_port ||
                recv_tcph->src_port != sock->remote_addr.sin_port ||
                iph->src_addr != sock->remote_addr.sin_addr.s_addr) {
                continue;
            }
        }
        
        // Copy TCP header
        memcpy(tcph, recv_tcph, sizeof(struct tcp_header));
        
        // Copy data if any
        int tcp_header_len = (recv_tcph->data_offset >> 4) * 4;
        int payload_len = ret - ip_header_len - tcp_header_len;
        
        if (payload_len > 0 && data != NULL) {
            memcpy(data, buffer + ip_header_len + tcp_header_len, payload_len);
            *data_len = payload_len;
        } else {
            *data_len = 0;
        }
        
        // Update remote address for listening sockets on SYN
        if (sock->listening && (recv_tcph->flags & TCP_SYN)) {
            sock->remote_addr.sin_addr.s_addr = iph->src_addr;
            sock->remote_addr.sin_port = recv_tcph->src_port;
        }
        
        return 0;
    }
}

// Create a TCP socket
int tcp_socket(void) {
    tcp_init();
    
    int slot = find_free_socket();
    if (slot < 0) {
        errno = EMFILE;
        return -1;
    }
    
    // Create raw socket
    int fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (fd < 0) {
        perror("Raw socket creation failed (need root privileges)");
        return -1;
    }
    
    // Enable IP_HDRINCL so we can build the IP header ourselves
    int one = 1;
    if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt IP_HDRINCL failed");
        close(fd);
        return -1;
    }
    
    // Initialize socket control block
    tcp_socket_t *sock = &socket_table[slot];
    memset(sock, 0, sizeof(tcp_socket_t));
    sock->fd = fd;
    sock->is_used = 1;
    sock->state = TCP_CLOSED;
    sock->send_seq = rand() % 1000000;  // Random initial sequence number
    sock->recv_seq = 0;
    sock->recv_len = 0;
    sock->listening = 0;
    
    return slot;
}

// Bind socket to address
int tcp_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    (void)addrlen; // Unused parameter
    
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || !socket_table[sockfd].is_used) {
        errno = EBADF;
        return -1;
    }
    
    tcp_socket_t *sock = &socket_table[sockfd];
    memcpy(&sock->local_addr, addr, sizeof(struct sockaddr_in));
    
    return 0;
}

// Listen for connections
int tcp_listen(int sockfd, int backlog) {
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || !socket_table[sockfd].is_used) {
        errno = EBADF;
        return -1;
    }
    
    tcp_socket_t *sock = &socket_table[sockfd];
    sock->state = TCP_LISTEN;
    sock->listening = 1;
    sock->backlog = backlog;
    
    printf("Socket %d listening on port %d\n", sockfd, ntohs(sock->local_addr.sin_port));
    
    return 0;
}

// Accept incoming connection
int tcp_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || !socket_table[sockfd].is_used) {
        errno = EBADF;
        return -1;
    }
    
    tcp_socket_t *listen_sock = &socket_table[sockfd];
    
    if (listen_sock->state != TCP_LISTEN) {
        errno = EINVAL;
        return -1;
    }
    
    printf("Waiting for incoming connection...\n");
    
    // Wait for SYN packet
    struct tcp_header tcph;
    uint8_t data[TCP_MSS];
    size_t data_len;
    
    while (1) {
        if (recv_tcp_packet(listen_sock, &tcph, data, &data_len, 0) < 0) {
            continue;
        }
        
        if (tcph.flags & TCP_SYN) {
            printf("Received SYN from %s:%d\n", 
                   inet_ntoa(listen_sock->remote_addr.sin_addr),
                   ntohs(listen_sock->remote_addr.sin_port));
            break;
        }
    }
    
    // Create new socket for this connection
    int new_sockfd = tcp_socket();
    if (new_sockfd < 0) {
        return -1;
    }
    
    tcp_socket_t *new_sock = &socket_table[new_sockfd];
    
    // Copy addresses
    memcpy(&new_sock->local_addr, &listen_sock->local_addr, sizeof(struct sockaddr_in));
    memcpy(&new_sock->remote_addr, &listen_sock->remote_addr, sizeof(struct sockaddr_in));
    
    // Update sequence numbers
    new_sock->recv_seq = ntohl(tcph.seq_num) + 1;
    new_sock->send_seq = rand() % 1000000;
    
    // Send SYN-ACK
    new_sock->state = TCP_SYN_RCVD;
    printf("Sending SYN-ACK...\n");
    send_tcp_packet(new_sock, TCP_SYN | TCP_ACK, NULL, 0);
    new_sock->send_seq++;
    
    // Wait for ACK
    printf("Waiting for ACK...\n");
    if (recv_tcp_packet(new_sock, &tcph, data, &data_len, 5) < 0) {
        printf("ACK timeout\n");
        tcp_close(new_sockfd);
        return -1;
    }
    
    if (tcph.flags & TCP_ACK) {
        printf("Received ACK, connection established\n");
        new_sock->state = TCP_ESTABLISHED;
        
        if (addr && addrlen) {
            memcpy(addr, &new_sock->remote_addr, sizeof(struct sockaddr_in));
            *addrlen = sizeof(struct sockaddr_in);
        }
        
        return new_sockfd;
    }
    
    tcp_close(new_sockfd);
    return -1;
}

// Connect to remote host
int tcp_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    (void)addrlen; // Unused parameter
    
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || !socket_table[sockfd].is_used) {
        errno = EBADF;
        return -1;
    }
    
    tcp_socket_t *sock = &socket_table[sockfd];
    memcpy(&sock->remote_addr, addr, sizeof(struct sockaddr_in));
    
    printf("Connecting to %s:%d...\n", 
           inet_ntoa(sock->remote_addr.sin_addr),
           ntohs(sock->remote_addr.sin_port));
    
    // Send SYN
    sock->state = TCP_SYN_SENT;
    printf("Sending SYN...\n");
    send_tcp_packet(sock, TCP_SYN, NULL, 0);
    sock->send_seq++;
    
    // Wait for SYN-ACK
    struct tcp_header tcph;
    uint8_t data[TCP_MSS];
    size_t data_len;
    
    printf("Waiting for SYN-ACK...\n");
    if (recv_tcp_packet(sock, &tcph, data, &data_len, 5) < 0) {
        printf("SYN-ACK timeout\n");
        sock->state = TCP_CLOSED;
        return -1;
    }
    
    if ((tcph.flags & (TCP_SYN | TCP_ACK)) == (TCP_SYN | TCP_ACK)) {
        printf("Received SYN-ACK\n");
        sock->recv_seq = ntohl(tcph.seq_num) + 1;
        
        // Send ACK
        printf("Sending ACK...\n");
        send_tcp_packet(sock, TCP_ACK, NULL, 0);
        
        sock->state = TCP_ESTABLISHED;
        printf("Connection established\n");
        return 0;
    }
    
    sock->state = TCP_CLOSED;
    return -1;
}

// Send data
ssize_t tcp_send(int sockfd, const void *buf, size_t len, int flags) {
    (void)flags; // Unused parameter
    
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || !socket_table[sockfd].is_used) {
        errno = EBADF;
        return -1;
    }
    
    tcp_socket_t *sock = &socket_table[sockfd];
    
    if (sock->state != TCP_ESTABLISHED) {
        errno = ENOTCONN;
        return -1;
    }
    
    // Send data in chunks if necessary
    size_t sent = 0;
    while (sent < len) {
        size_t chunk_size = (len - sent) > TCP_MSS ? TCP_MSS : (len - sent);
        
        if (send_tcp_packet(sock, TCP_PSH | TCP_ACK, (uint8_t *)buf + sent, chunk_size) < 0) {
            return -1;
        }
        
        sock->send_seq += chunk_size;
        sent += chunk_size;
        
        // Wait for ACK
        struct tcp_header tcph;
        uint8_t data[TCP_MSS];
        size_t data_len;
        
        if (recv_tcp_packet(sock, &tcph, data, &data_len, 2) < 0) {
            // Timeout, but we're not implementing retransmission
            printf("Warning: ACK timeout, continuing anyway\n");
        }
    }
    
    return sent;
}

// Receive data
ssize_t tcp_recv(int sockfd, void *buf, size_t len, int flags) {
    (void)flags; // Unused parameter
    
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || !socket_table[sockfd].is_used) {
        errno = EBADF;
        return -1;
    }
    
    tcp_socket_t *sock = &socket_table[sockfd];
    
    if (sock->state != TCP_ESTABLISHED && sock->state != TCP_CLOSE_WAIT) {
        errno = ENOTCONN;
        return -1;
    }
    
    // Receive packet
    struct tcp_header tcph;
    uint8_t data[TCP_MSS];
    size_t data_len;
    
    if (recv_tcp_packet(sock, &tcph, data, &data_len, 10) < 0) {
        return -1;
    }
    
    // Check for FIN
    if (tcph.flags & TCP_FIN) {
        printf("Received FIN\n");
        sock->recv_seq = ntohl(tcph.seq_num) + 1;
        
        // Send ACK
        send_tcp_packet(sock, TCP_ACK, NULL, 0);
        
        sock->state = TCP_CLOSE_WAIT;
        return 0;  // Connection closing
    }
    
    // Copy data to user buffer
    size_t copy_len = data_len < len ? data_len : len;
    if (copy_len > 0) {
        memcpy(buf, data, copy_len);
        sock->recv_seq += data_len;
        
        // Send ACK
        send_tcp_packet(sock, TCP_ACK, NULL, 0);
    }
    
    return copy_len;
}

// Close connection
int tcp_close(int sockfd) {
    if (sockfd < 0 || sockfd >= MAX_SOCKETS || !socket_table[sockfd].is_used) {
        errno = EBADF;
        return -1;
    }
    
    tcp_socket_t *sock = &socket_table[sockfd];
    
    if (sock->state == TCP_ESTABLISHED) {
        printf("Closing connection...\n");
        
        // Send FIN
        printf("Sending FIN...\n");
        send_tcp_packet(sock, TCP_FIN | TCP_ACK, NULL, 0);
        sock->send_seq++;
        sock->state = TCP_FIN_WAIT_1;
        
        // Wait for ACK
        struct tcp_header tcph;
        uint8_t data[TCP_MSS];
        size_t data_len;
        
        printf("Waiting for ACK...\n");
        if (recv_tcp_packet(sock, &tcph, data, &data_len, 2) >= 0) {
            if (tcph.flags & TCP_ACK) {
                printf("Received ACK\n");
                sock->state = TCP_FIN_WAIT_2;
            }
        }
        
        // Wait for FIN
        printf("Waiting for FIN...\n");
        if (recv_tcp_packet(sock, &tcph, data, &data_len, 2) >= 0) {
            if (tcph.flags & TCP_FIN) {
                printf("Received FIN\n");
                sock->recv_seq = ntohl(tcph.seq_num) + 1;
                
                // Send ACK
                printf("Sending final ACK...\n");
                send_tcp_packet(sock, TCP_ACK, NULL, 0);
                
                sock->state = TCP_TIME_WAIT;
            }
        }
    } else if (sock->state == TCP_CLOSE_WAIT) {
        // Send FIN
        printf("Sending FIN...\n");
        send_tcp_packet(sock, TCP_FIN | TCP_ACK, NULL, 0);
        sock->send_seq++;
        sock->state = TCP_LAST_ACK;
        
        // Wait for ACK
        struct tcp_header tcph;
        uint8_t data[TCP_MSS];
        size_t data_len;
        
        printf("Waiting for final ACK...\n");
        recv_tcp_packet(sock, &tcph, data, &data_len, 2);
    }
    
    // Close socket
    if (sock->fd >= 0) {
        close(sock->fd);
    }
    
    sock->is_used = 0;
    sock->state = TCP_CLOSED;
    
    printf("Connection closed\n");
    
    return 0;
}

