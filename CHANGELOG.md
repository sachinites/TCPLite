# Changelog

All notable changes to TCP Lite will be documented in this file.

## [1.1.0] - 2025-11-01

### Added
- **Enhanced Server Command Line Arguments**
  - Server now accepts IP address and port: `sudo ./tcp_server [ip_address] [port]`
  - Backwards compatible: `sudo ./tcp_server [port]` still works
  - Can bind to specific network interface (e.g., `127.0.0.1`, `192.168.1.100`)
  - Default behavior: binds to all interfaces (`0.0.0.0:8080`)
  
- **Improved Client Command Line Arguments**
  - Enhanced help messages with usage examples
  - Consistent argument parsing with server
  
- **Better Error Messages**
  - Both server and client display helpful usage information
  - Clear examples for common use cases
  
- **New Documentation**
  - `USAGE_EXAMPLES.md`: Comprehensive guide with 20+ usage examples
  - Updated `README.md` with new command line syntax
  - Updated `QUICKSTART.md` with new usage patterns

### Changed
- Server now validates IP address format
- Server displays bound IP address on startup
- Test script updated to use explicit IP binding
- Error messages now show multiple usage examples

### Examples

**Server - Before:**
```bash
sudo ./tcp_server 8080
```

**Server - After (multiple options):**
```bash
sudo ./tcp_server                  # All interfaces, port 8080
sudo ./tcp_server 9000             # All interfaces, port 9000
sudo ./tcp_server 127.0.0.1 8080   # Localhost only, port 8080
sudo ./tcp_server 0.0.0.0 8080     # All interfaces, port 8080 (explicit)
```

## [1.0.0] - 2025-11-01

### Initial Release
- Complete TCP 3-way handshake implementation
- TCP 4-way connection termination
- Basic data transfer with ACK
- RAW socket implementation
- BSD socket-like API
- Echo server example
- Client example
- Comprehensive documentation
- Build system with Makefile
- Automated test script

### Features
- `tcp_socket()` - Create socket
- `tcp_bind()` - Bind to address
- `tcp_listen()` - Listen for connections
- `tcp_accept()` - Accept connections
- `tcp_connect()` - Connect to server
- `tcp_send()` - Send data
- `tcp_recv()` - Receive data
- `tcp_close()` - Close connection

### Documentation
- `README.md` - Comprehensive documentation
- `QUICKSTART.md` - Quick start guide
- `PROJECT_SUMMARY.md` - Technical summary
- Inline code comments

### Known Limitations
- No congestion control
- No retransmission
- No complex window management
- Single connection at a time
- Requires root privileges
- Educational purpose only

