# TCP Lite - Project Summary

## Overview
A simplified user-space TCP protocol implementation in pure C using RAW sockets. This project demonstrates the fundamental TCP mechanisms without complex features like congestion control or retransmission.

## Project Structure

```
TCPLite/
├── tcp_lite.h          # Header file with data structures and API declarations
├── tcp_lite.c          # Core TCP implementation (17KB, ~550 lines)
├── server.c            # Example echo server application
├── client.c            # Example client application
├── Makefile            # Build configuration
├── README.md           # Comprehensive documentation
├── QUICKSTART.md       # Quick start guide
├── test.sh             # Automated test script
├── .gitignore          # Git ignore rules
└── LICENSE             # License file

Binaries (after build):
├── tcp_server          # Server executable
└── tcp_client          # Client executable
```

## Core Components Implemented

### 1. TCP State Machine
Implements the following TCP states:
- `TCP_CLOSED` - Initial/final state
- `TCP_LISTEN` - Server waiting for connections
- `TCP_SYN_SENT` - Client sent SYN, waiting for SYN-ACK
- `TCP_SYN_RCVD` - Server received SYN, sent SYN-ACK
- `TCP_ESTABLISHED` - Connection active, data transfer
- `TCP_FIN_WAIT_1` - Sent FIN, waiting for ACK
- `TCP_FIN_WAIT_2` - Received ACK, waiting for FIN
- `TCP_CLOSE_WAIT` - Received FIN, application closing
- `TCP_CLOSING` - Both sides closing simultaneously
- `TCP_LAST_ACK` - Sent final FIN, waiting for ACK
- `TCP_TIME_WAIT` - Waiting before final close

### 2. TCP 3-Way Handshake (Connection Establishment)
```
Client                    Server
  |                         |
  |-------- SYN -------->   |  tcp_connect()
  |                         |  tcp_accept()
  |<------ SYN-ACK ------   |  
  |                         |
  |-------- ACK -------->   |  
  |                         |
  [ESTABLISHED]       [ESTABLISHED]
```

**Implementation:**
- Client: `tcp_connect()` sends SYN, waits for SYN-ACK, sends ACK
- Server: `tcp_accept()` waits for SYN, sends SYN-ACK, waits for ACK

### 3. Data Transfer
```
Sender                   Receiver
  |                         |
  |---- PSH+ACK (data) ->   |  tcp_send()
  |                         |  tcp_recv()
  |<------ ACK ----------   |  
  |                         |
```

**Features:**
- Automatic data segmentation (MSS = 1460 bytes)
- Sequence number tracking
- ACK generation and validation
- Simple window management (fixed 65535 bytes)

### 4. TCP 4-Way Handshake (Connection Termination)
```
Initiator                 Receiver
  |                         |
  |------ FIN+ACK ------>   |  tcp_close()
  |                         |
  |<------ ACK ----------   |  
  |                         |
  |<------ FIN+ACK ------   |  tcp_close()
  |                         |
  |-------- ACK -------->   |  
  |                         |
 [CLOSED]              [CLOSED]
```

**Implementation:**
- Handles both active close (FIN_WAIT states) and passive close (CLOSE_WAIT state)
- Proper sequence number management during closing

### 5. Packet Construction

#### IP Header (20 bytes)
- Version: IPv4
- Header Length: 5 (20 bytes)
- TTL: 64
- Protocol: TCP (6)
- Source/Destination IP addresses
- IP header checksum

#### TCP Header (20 bytes)
- Source/Destination ports
- Sequence number
- Acknowledgment number
- Data offset: 5 (20 bytes)
- Flags: SYN, ACK, FIN, PSH, RST, URG
- Window size: 65535
- TCP checksum (includes pseudo header)
- Urgent pointer

### 6. Checksum Calculation
- **IP Checksum**: 16-bit one's complement of the one's complement sum
- **TCP Checksum**: Calculated over pseudo header + TCP header + data
  - Pseudo header includes: source IP, dest IP, protocol, TCP length

### 7. Socket Management
- Socket table with up to 64 concurrent sockets
- Socket control block stores:
  - Raw socket file descriptor
  - TCP state
  - Local and remote addresses
  - Send and receive sequence numbers
  - Receive buffer (65536 bytes)
  - Listening flag

## API Functions

### Complete Socket API
1. **`int tcp_socket(void)`**
   - Creates a new TCP socket
   - Returns socket descriptor (0-63) or -1 on error
   - Requires root privileges (RAW socket)

2. **`int tcp_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)`**
   - Binds socket to local address and port
   - Required for both client and server

3. **`int tcp_listen(int sockfd, int backlog)`**
   - Sets socket to listening state
   - Server-side only

4. **`int tcp_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)`**
   - Waits for and accepts incoming connection
   - Performs 3-way handshake
   - Returns new socket descriptor for the connection

5. **`int tcp_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)`**
   - Connects to remote server
   - Performs 3-way handshake
   - Client-side only

6. **`ssize_t tcp_send(int sockfd, const void *buf, size_t len, int flags)`**
   - Sends data over established connection
   - Automatically segments data into MSS-sized chunks
   - Waits for ACK after each segment

7. **`ssize_t tcp_recv(int sockfd, void *buf, size_t len, int flags)`**
   - Receives data from established connection
   - Automatically sends ACK
   - Handles FIN flag for connection termination

8. **`int tcp_close(int sockfd)`**
   - Closes connection gracefully
   - Performs 4-way handshake
   - Cleans up resources

## Technical Details

### Raw Socket Usage
```c
// Create raw socket
int fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);

// Enable IP_HDRINCL to construct IP headers manually
int one = 1;
setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

// Send packet
sendto(fd, packet, packet_len, 0, &dest_addr, addr_len);

// Receive packet
recvfrom(fd, buffer, buffer_len, 0, &src_addr, &addr_len);
```

### Sequence Number Management
- **Initial Sequence Number (ISN)**: Random value (0-1000000)
- **Send Sequence**: Incremented by data length + 1 for SYN/FIN
- **Receive Sequence**: Tracks expected next byte from peer
- **Acknowledgment**: Always set to receive sequence number

### Packet Flow Example
```
Client                                    Server
SEQ=1000 ----- SYN ------>               
                            <---- SYN-ACK ----- SEQ=2000, ACK=1001
SEQ=1001 ----- ACK ------>               ACK=2001
                           
SEQ=1001 -- PSH+ACK(14B) ->              ACK=2001
                            <----- ACK -------- SEQ=2001, ACK=1015
                           
                            <- PSH+ACK(20B) --- SEQ=2001, ACK=1015
SEQ=1015 ----- ACK ------>               ACK=2021
```

## Features Implemented ✅

- [x] TCP 3-way handshake (SYN, SYN-ACK, ACK)
- [x] TCP 4-way connection termination (FIN, ACK, FIN, ACK)
- [x] Data transfer with PSH+ACK
- [x] Acknowledgment generation and handling
- [x] Sequence number management
- [x] IP header construction
- [x] TCP header construction
- [x] Checksum calculation (IP and TCP)
- [x] Socket state management
- [x] Multiple socket support (up to 64)
- [x] BSD socket-like API
- [x] Data segmentation (MSS handling)
- [x] Echo server example
- [x] Client example with multiple messages

## Features NOT Implemented ❌ (By Design)

- [ ] Congestion control (slow start, congestion avoidance)
- [ ] Flow control (sliding window)
- [ ] Retransmission timeout (RTO)
- [ ] Fast retransmit / fast recovery
- [ ] Out-of-order packet handling
- [ ] Duplicate ACK detection
- [ ] TCP options (timestamps, SACK, window scaling)
- [ ] Nagle's algorithm
- [ ] Keep-alive
- [ ] Urgent data (URG flag)
- [ ] Reset handling (RST flag)
- [ ] Simultaneous open
- [ ] Multiple simultaneous connections per listening socket
- [ ] Non-blocking I/O
- [ ] select/poll/epoll integration

## Build Instructions

### Requirements
- Linux OS (tested on Ubuntu/Debian)
- GCC compiler
- Root privileges (for RAW sockets)

### Compile
```bash
make            # Build both server and client
make clean      # Remove build artifacts
```

### Run
```bash
# Terminal 1: Start server
sudo ./tcp_server 8080

# Terminal 2: Run client
sudo ./tcp_client 127.0.0.1 8080

# Or use automated test
sudo ./test.sh
```

## Code Metrics
- **Total Lines**: ~700 lines of code
- **tcp_lite.c**: ~550 lines (core implementation)
- **server.c**: ~120 lines
- **client.c**: ~120 lines
- **tcp_lite.h**: ~90 lines

## Key Functions by Size
1. `recv_tcp_packet()` - ~100 lines (packet reception and filtering)
2. `send_tcp_packet()` - ~70 lines (packet construction and transmission)
3. `tcp_accept()` - ~60 lines (server-side handshake)
4. `tcp_connect()` - ~50 lines (client-side handshake)
5. `tcp_close()` - ~60 lines (connection termination)
6. `tcp_send()` - ~40 lines (data transmission)
7. `tcp_recv()` - ~40 lines (data reception)

## Memory Usage
- Socket table: 64 sockets × ~66 KB each = ~4 MB max
- Per-socket receive buffer: 65536 bytes
- Packet buffers: Allocated on stack (~2 KB per send/recv)

## Limitations
1. **No retransmission**: Lost packets are not retransmitted
2. **No congestion control**: May flood the network
3. **Single-threaded**: Blocking I/O only
4. **Fixed window**: No dynamic window adjustment
5. **Limited error handling**: Basic error checking only
6. **Local testing only**: Best used on loopback interface
7. **Requires root**: RAW sockets need elevated privileges
8. **No security**: No encryption or authentication

## Testing
```bash
# Run automated test
sudo ./test.sh

# Manual testing with packet capture
sudo tcpdump -i lo port 8080 -vv -X

# Monitor in separate terminal while running test
```

## Educational Value
This project demonstrates:
1. **Network protocol implementation** from scratch
2. **RAW socket programming** in C
3. **TCP state machine** and transitions
4. **Binary protocol design** and parsing
5. **Checksum algorithms** (16-bit one's complement)
6. **System programming** on Linux
7. **API design** (BSD socket-like interface)

## Use Cases
- ✅ Learning TCP internals
- ✅ Understanding socket programming
- ✅ Protocol experimentation
- ✅ Network debugging
- ✅ Teaching tool
- ❌ Production use (not secure or reliable)
- ❌ High-performance applications
- ❌ Mission-critical systems

## Future Enhancements (Optional)
If extending this project, consider adding:
1. Retransmission timer (RTO) with exponential backoff
2. Congestion window (cwnd) management
3. Selective Acknowledgment (SACK)
4. Out-of-order queue
5. Connection pooling
6. Non-blocking I/O with epoll
7. Multi-threading support
8. Better error handling
9. Statistics and monitoring
10. Unit tests

## References
- RFC 793: Transmission Control Protocol
- RFC 1122: Requirements for Internet Hosts
- TCP/IP Illustrated, Volume 1 by W. Richard Stevens
- Linux Socket Programming by Example

## License
See LICENSE file for details.

## Author
Created as an educational implementation of TCP for learning purposes.

---

**Disclaimer**: This is a simplified implementation for educational purposes only. Do not use in production environments. Use standard kernel TCP stack for real applications.

