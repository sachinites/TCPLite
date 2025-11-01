#include "tcp_lite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    char *server_ip = "127.0.0.1";
    int server_port = 8080;
    
    // Parse command line arguments
    if (argc >= 2) {
        server_ip = argv[1];
    }
    if (argc >= 3) {
        server_port = atoi(argv[2]);
    }
    
    printf("=== TCP Lite Client ===\n");
    printf("Server: %s:%d\n\n", server_ip, server_port);
    
    // Check if running as root
    if (geteuid() != 0) {
        fprintf(stderr, "Error: This program requires root privileges (raw sockets)\n");
        fprintf(stderr, "Usage: sudo %s [server_ip] [port]\n", argv[0]);
        fprintf(stderr, "Examples:\n");
        fprintf(stderr, "  sudo %s                        # Connect to 127.0.0.1:8080 (default)\n", argv[0]);
        fprintf(stderr, "  sudo %s 192.168.1.100          # Connect to 192.168.1.100:8080\n", argv[0]);
        fprintf(stderr, "  sudo %s 192.168.1.100 9000     # Connect to 192.168.1.100:9000\n", argv[0]);
        return 1;
    }
    
    // Create socket
    int sockfd = tcp_socket();
    if (sockfd < 0) {
        perror("tcp_socket failed");
        return 1;
    }
    printf("Socket created: %d\n", sockfd);
    
    // Bind to local address (required for raw sockets)
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(12345);  // Client source port
    
    if (tcp_bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("tcp_bind failed");
        tcp_close(sockfd);
        return 1;
    }
    printf("Socket bound to port 12345\n");
    
    // Connect to server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP address\n");
        tcp_close(sockfd);
        return 1;
    }
    
    if (tcp_connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("tcp_connect failed");
        tcp_close(sockfd);
        return 1;
    }
    
    printf("\n=== Connected to Server ===\n\n");
    
    // Send and receive data
    char buffer[1024];
    char *messages[] = {
        "Hello, Server!",
        "This is TCP Lite",
        "A simple TCP implementation",
        "quit"
    };
    
    int num_messages = sizeof(messages) / sizeof(messages[0]);
    
    for (int i = 0; i < num_messages; i++) {
        printf("Sending: %s\n", messages[i]);
        
        ssize_t sent = tcp_send(sockfd, messages[i], strlen(messages[i]), 0);
        if (sent < 0) {
            perror("tcp_send failed");
            break;
        }
        printf("Sent %zd bytes\n", sent);
        
        // Receive response
        printf("Waiting for response...\n");
        ssize_t received = tcp_recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (received < 0) {
            perror("tcp_recv failed");
            break;
        }
        
        if (received == 0) {
            printf("Server closed connection\n");
            break;
        }
        
        buffer[received] = '\0';
        printf("Received: %s\n\n", buffer);
        
        // Small delay between messages
        usleep(500000);  // 500ms
    }
    
    // Close connection
    printf("Closing connection...\n");
    tcp_close(sockfd);
    
    printf("Client shutdown complete\n");
    
    return 0;
}

