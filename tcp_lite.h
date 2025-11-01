#ifndef TCP_LITE_H
#define TCP_LITE_H

#include <stdint.h>
#include <netinet/in.h>

// TCP States
#define TCP_CLOSED       0
#define TCP_LISTEN       1
#define TCP_SYN_SENT     2
#define TCP_SYN_RCVD     3
#define TCP_ESTABLISHED  4
#define TCP_FIN_WAIT_1   5
#define TCP_FIN_WAIT_2   6
#define TCP_CLOSE_WAIT   7
#define TCP_CLOSING      8
#define TCP_LAST_ACK     9
#define TCP_TIME_WAIT    10

// TCP Flags
#define TCP_FIN  0x01
#define TCP_SYN  0x02
#define TCP_RST  0x04
#define TCP_PSH  0x08
#define TCP_ACK  0x10
#define TCP_URG  0x20

// Constants
#define MAX_SOCKETS      64
#define MAX_PENDING_CONN 5
#define TCP_BUFFER_SIZE  65536
#define TCP_WINDOW_SIZE  65535  // Max window size for uint16_t
#define TCP_MSS          1460  // Maximum Segment Size

// TCP Header (20 bytes without options)
struct tcp_header {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t  data_offset;  // 4 bits offset, 4 bits reserved
    uint8_t  flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent_ptr;
} __attribute__((packed));

// IP Header (20 bytes without options)
struct ip_header {
    uint8_t  version_ihl;    // 4 bits version, 4 bits IHL
    uint8_t  tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t frag_offset;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t src_addr;
    uint32_t dst_addr;
} __attribute__((packed));

// Pseudo header for TCP checksum calculation
struct pseudo_header {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint8_t  zero;
    uint8_t  protocol;
    uint16_t tcp_length;
} __attribute__((packed));

// TCP Socket Control Block
typedef struct tcp_socket {
    int fd;                           // Raw socket file descriptor
    int state;                        // TCP state
    int is_used;                      // Socket slot in use
    
    struct sockaddr_in local_addr;    // Local address and port
    struct sockaddr_in remote_addr;   // Remote address and port
    
    uint32_t send_seq;                // Send sequence number
    uint32_t recv_seq;                // Expected receive sequence number
    
    uint8_t recv_buffer[TCP_BUFFER_SIZE];
    int recv_len;
    
    int listening;                    // Is this a listening socket
    int backlog;                      // Listen backlog
} tcp_socket_t;

// API Functions
int tcp_socket(void);
int tcp_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int tcp_listen(int sockfd, int backlog);
int tcp_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int tcp_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t tcp_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t tcp_recv(int sockfd, void *buf, size_t len, int flags);
int tcp_close(int sockfd);

// Utility functions
uint16_t tcp_checksum(const void *buf, size_t len);
uint16_t ip_checksum(const void *buf, size_t len);
void tcp_init(void);

#endif // TCP_LITE_H

