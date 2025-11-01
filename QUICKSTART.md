# TCP Lite - Quick Start Guide

## Prerequisites
- Linux system
- GCC compiler
- Root/sudo access (required for RAW sockets)

## Quick Build & Run

### 1. Build
```bash
make
```

### 2. Run Server (Terminal 1)
```bash
sudo ./tcp_server 8080
```

### 3. Run Client (Terminal 2)
```bash
sudo ./tcp_client 127.0.0.1 8080
```

## Using the Automated Test Script

```bash
sudo ./test.sh
```

This will:
- Build the project
- Start the server in the background
- Run the client
- Clean up automatically

## What You'll See

### Server Output
```
=== TCP Lite Server ===
Port: 8080

Socket created: 0
Socket bound to port 8080
Socket 0 listening on port 8080
Waiting for incoming connection...
Received SYN from 127.0.0.1:12345
Sending SYN-ACK...
Waiting for ACK...
Received ACK, connection established

=== Connection Established ===
Client: 127.0.0.1:12345

Waiting for data...
Received 14 bytes: Hello, Server!
Sending echo response...
Sent 20 bytes
```

### Client Output
```
=== TCP Lite Client ===
Server: 127.0.0.1:8080

Socket created: 0
Socket bound to port 12345
Connecting to 127.0.0.1:8080...
Sending SYN...
Waiting for SYN-ACK...
Received SYN-ACK
Sending ACK...
Connection established

=== Connected to Server ===

Sending: Hello, Server!
Sent 14 bytes
Waiting for response...
Received: Echo: Hello, Server!
```

## Monitoring with tcpdump

Watch the TCP packets in real-time:

```bash
# In a separate terminal
sudo tcpdump -i lo port 8080 -vv
```

You'll see:
- SYN packets (3-way handshake)
- PSH+ACK packets (data transfer)
- FIN packets (4-way termination)

## API Usage Example

```c
#include "tcp_lite.h"

// Server side
int server_fd = tcp_socket();
tcp_bind(server_fd, &addr, sizeof(addr));
tcp_listen(server_fd, 5);
int client_fd = tcp_accept(server_fd, NULL, NULL);
tcp_recv(client_fd, buffer, sizeof(buffer), 0);
tcp_send(client_fd, response, strlen(response), 0);
tcp_close(client_fd);

// Client side
int sock_fd = tcp_socket();
tcp_bind(sock_fd, &local_addr, sizeof(local_addr));
tcp_connect(sock_fd, &server_addr, sizeof(server_addr));
tcp_send(sock_fd, message, strlen(message), 0);
tcp_recv(sock_fd, buffer, sizeof(buffer), 0);
tcp_close(sock_fd);
```

## Common Issues

### "Permission denied"
**Solution**: Run with sudo
```bash
sudo ./tcp_server
```

### "Connection timeout"
**Problem**: Firewall blocking packets

**Solution** (for testing only):
```bash
# Check current rules
sudo iptables -L

# Allow loopback traffic
sudo iptables -A INPUT -i lo -j ACCEPT
sudo iptables -A OUTPUT -o lo -j ACCEPT
```

### Server won't start on port
**Problem**: Port already in use

**Solution**: 
```bash
# Check what's using the port
sudo netstat -tulpn | grep 8080

# Use a different port
sudo ./tcp_server 9000
sudo ./tcp_client 127.0.0.1 9000
```

## Understanding the Flow

### Connection Setup (3-Way Handshake)
1. Client sends SYN
2. Server responds with SYN-ACK
3. Client sends ACK
4. Connection ESTABLISHED

### Data Transfer
1. Sender: PSH+ACK with data
2. Receiver: ACK acknowledgment

### Connection Teardown (4-Way Handshake)
1. Initiator sends FIN+ACK
2. Receiver sends ACK
3. Receiver sends FIN+ACK
4. Initiator sends ACK
5. Connection CLOSED

## Performance Notes

This is a **simplified implementation** for educational purposes:

✅ Works for basic communication  
✅ Demonstrates core TCP concepts  
⚠️  No retransmission (packets may be lost)  
⚠️  No congestion control  
⚠️  Not suitable for production  

## Clean Up

```bash
make clean
```

This removes all compiled binaries and object files.

## Next Steps

1. Read `README.md` for detailed documentation
2. Examine `tcp_lite.c` to understand the implementation
3. Modify `client.c` or `server.c` for your experiments
4. Use `tcpdump` to watch packets on the wire

## Debugging Commands

```bash
# Watch packets
sudo tcpdump -i lo port 8080 -XX

# Check active connections
netstat -an | grep 8080

# Monitor system calls
sudo strace -e trace=socket,bind,sendto,recvfrom ./tcp_server

# Check for errors
dmesg | tail
```

---

Happy hacking! 🚀

