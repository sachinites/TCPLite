#include "tcp_lite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int port = 8080;
    
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    
    printf("=== TCP Lite Server ===\n");
    printf("Port: %d\n\n", port);
    
    // Check if running as root
    if (geteuid() != 0) {
        fprintf(stderr, "Error: This program requires root privileges (raw sockets)\n");
        fprintf(stderr, "Please run with: sudo %s [port]\n", argv[0]);
        return 1;
    }
    
    // Create socket
    int sockfd = tcp_socket();
    if (sockfd < 0) {
        perror("tcp_socket failed");
        return 1;
    }
    printf("Socket created: %d\n", sockfd);
    
    // Bind to address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (tcp_bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("tcp_bind failed");
        tcp_close(sockfd);
        return 1;
    }
    printf("Socket bound to port %d\n", port);
    
    // Listen for connections
    if (tcp_listen(sockfd, 5) < 0) {
        perror("tcp_listen failed");
        tcp_close(sockfd);
        return 1;
    }
    
    // Accept connection
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_fd = tcp_accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("tcp_accept failed");
        tcp_close(sockfd);
        return 1;
    }
    
    printf("\n=== Connection Established ===\n");
    printf("Client: %s:%d\n\n", 
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));
    
    // Receive data in a loop
    char buffer[1024];
    while (1) {
        printf("Waiting for data...\n");
        
        ssize_t received = tcp_recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (received < 0) {
            perror("tcp_recv failed");
            break;
        }
        
        if (received == 0) {
            printf("Client closed connection\n");
            break;
        }
        
        buffer[received] = '\0';
        printf("Received %zd bytes: %s\n", received, buffer);
        
        // Echo back
        printf("Sending echo response...\n");
        char response[2048];  // Larger buffer to avoid truncation
        snprintf(response, sizeof(response), "Echo: %s", buffer);
        
        ssize_t sent = tcp_send(client_fd, response, strlen(response), 0);
        if (sent < 0) {
            perror("tcp_send failed");
            break;
        }
        printf("Sent %zd bytes\n\n", sent);
        
        // Exit if client sends "quit"
        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Quit command received\n");
            break;
        }
    }
    
    // Close connection
    printf("\nClosing connections...\n");
    tcp_close(client_fd);
    tcp_close(sockfd);
    
    printf("Server shutdown complete\n");
    
    return 0;
}

