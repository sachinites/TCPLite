# TCP Lite - Simple User Space TCP Implementation

A simplified user-space TCP protocol implementation in pure C using RAW sockets. This project demonstrates the core TCP mechanisms: 3-way handshake, data transfer, and 4-way connection termination.

## Features

✅ **TCP 3-way handshake** (SYN, SYN-ACK, ACK)  
✅ **TCP 4-way connection termination** (FIN, ACK, FIN, ACK)  
✅ **Basic data transfer** with ACK  
✅ **User-space socket API** similar to BSD sockets  
✅ **RAW socket implementation** for direct IP/TCP packet handling  

### Limitations (By Design)

❌ No congestion control  
❌ No retransmission mechanism  
❌ No complex window management  
❌ No packet reordering  
❌ No advanced TCP features  

This is intentionally kept simple for educational purposes and demonstration of core TCP concepts.

## Architecture

### Components

1. **tcp_lite.h** - Header file with data structures and API declarations
2. **tcp_lite.c** - Core TCP implementation
3. **server.c** - Example echo server
4. **client.c** - Example client

### Key Data Structures

```c
struct tcp_header    // TCP header (20 bytes)
struct ip_header     // IP header (20 bytes)
struct tcp_socket    // Socket control block
```

### API Functions

The library provides a familiar BSD socket-like interface:

- `int tcp_socket(void)` - Create a TCP socket
- `int tcp_bind(int sockfd, ...)` - Bind socket to address
- `int tcp_listen(int sockfd, int backlog)` - Listen for connections
- `int tcp_accept(int sockfd, ...)` - Accept incoming connection
- `int tcp_connect(int sockfd, ...)` - Connect to remote host
- `ssize_t tcp_send(int sockfd, ...)` - Send data
- `ssize_t tcp_recv(int sockfd, ...)` - Receive data
- `int tcp_close(int sockfd)` - Close connection

## Requirements

- Linux operating system
- GCC compiler
- Root privileges (required for RAW sockets)

## Building

```bash
# Build all
make

# Build and clean
make clean
make
```

This will create two executables:
- `tcp_server` - Echo server
- `tcp_client` - Client application

## Usage

### Running the Server

The server listens on a specified IP address and port, and echoes back received messages.

**Syntax:** `sudo ./tcp_server [ip_address] [port]` or `sudo ./tcp_server [port]`

```bash
# Run on default (all interfaces 0.0.0.0, port 8080)
sudo ./tcp_server

# Run on specific port (all interfaces)
sudo ./tcp_server 9000

# Run on specific IP and port
sudo ./tcp_server 127.0.0.1 8080

# Listen on all interfaces, port 9000
sudo ./tcp_server 0.0.0.0 9000
```

### Running the Client

The client connects to the specified server and sends test messages.

**Syntax:** `sudo ./tcp_client [server_ip] [port]`

```bash
# Connect to localhost:8080 (default)
sudo ./tcp_client

# Connect to specific server (default port 8080)
sudo ./tcp_client 192.168.1.100

# Connect to specific server and port
sudo ./tcp_client 192.168.1.100 9000
```

### Example Session

**Terminal 1 (Server):**
```bash
$ sudo ./tcp_server
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

**Terminal 2 (Client):**
```bash
$ sudo ./tcp_client
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

## How It Works

### 1. Connection Establishment (3-Way Handshake)

```
Client                    Server
  |                         |
  |-------- SYN -------->   |  (Client initiates)
  |                         |
  |<------ SYN-ACK ------   |  (Server responds)
  |                         |
  |-------- ACK -------->   |  (Client confirms)
  |                         |
  [ESTABLISHED]       [ESTABLISHED]
```

### 2. Data Transfer

```
Client                    Server
  |                         |
  |---- PSH+ACK (data) ->   |  (Send data)
  |                         |
  |<------ ACK ----------   |  (Acknowledge)
  |                         |
```

### 3. Connection Termination (4-Way Handshake)

```
Client                    Server
  |                         |
  |------ FIN+ACK ------>   |  (Client closes)
  |                         |
  |<------ ACK ----------   |  (Server ACKs)
  |                         |
  |<------ FIN+ACK ------   |  (Server closes)
  |                         |
  |-------- ACK -------->   |  (Client ACKs)
  |                         |
 [CLOSED]              [CLOSED]
```

## TCP State Machine

The implementation follows the simplified TCP state machine:

```
CLOSED → LISTEN → SYN_RCVD → ESTABLISHED → CLOSE_WAIT → CLOSED
CLOSED → SYN_SENT → ESTABLISHED → FIN_WAIT_1 → FIN_WAIT_2 → TIME_WAIT → CLOSED
```

## Implementation Details

### Raw Socket Setup

```c
// Create raw socket
int fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);

// Enable IP_HDRINCL to construct IP headers
int one = 1;
setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));
```

### Packet Structure

Each packet consists of:
1. **IP Header** (20 bytes) - Source/Dest IP, protocol, etc.
2. **TCP Header** (20 bytes) - Ports, sequence numbers, flags, etc.
3. **Data** (variable) - Application payload

### Checksum Calculation

TCP checksum includes:
- Pseudo header (source IP, dest IP, protocol)
- TCP header
- TCP data

## Debugging Tips

1. **Permission Denied**: Ensure you're running with `sudo`
2. **Connection Timeout**: Check firewall rules and IP addresses
3. **Packet Sniffing**: Use tcpdump to monitor packets:
   ```bash
   sudo tcpdump -i lo -nn port 8080
   ```

## Security Considerations

⚠️ **WARNING**: This is an educational implementation. Do NOT use in production:

- No input validation
- No security features
- Vulnerable to various attacks
- No encryption
- Requires root privileges

## Educational Value

This project demonstrates:

1. **Network Programming**: RAW socket programming in C
2. **Protocol Implementation**: TCP state machine and packet handling
3. **Binary Protocols**: Header construction and parsing
4. **Checksum Algorithms**: 16-bit one's complement checksum
5. **System Programming**: Low-level Linux networking

## Troubleshooting

### "Raw socket creation failed"
- Ensure you're running with root privileges: `sudo ./tcp_server`

### "Connection timeout"
- Verify server is running
- Check firewall settings: `sudo iptables -L`
- For testing, temporarily disable firewall or add rules

### Packets not received
- Check if another process is capturing TCP packets
- Verify IP addresses and ports
- Use tcpdump to debug: `sudo tcpdump -i any port 8080 -X`

## Future Enhancements (Not Implemented)

If you want to extend this project:

- [ ] Retransmission timeout (RTO)
- [ ] Congestion control (slow start, congestion avoidance)
- [ ] Flow control (sliding window)
- [ ] Out-of-order packet handling
- [ ] Duplicate ACK detection
- [ ] Connection pooling
- [ ] Multiple simultaneous connections
- [ ] Non-blocking I/O
- [ ] select/poll/epoll integration

## License

This is educational software provided as-is for learning purposes.

## References

- [RFC 793 - Transmission Control Protocol](https://tools.ietf.org/html/rfc793)
- [TCP/IP Illustrated, Volume 1](https://www.oreilly.com/library/view/tcpip-illustrated-volume/9780132808200/)
- [Linux Raw Sockets](https://man7.org/linux/man-pages/man7/raw.7.html)

## Author

Created as a simplified TCP implementation for educational purposes.

---

**Remember**: This is a learning tool. Use standard socket APIs for real applications!
