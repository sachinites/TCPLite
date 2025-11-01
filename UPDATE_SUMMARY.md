# TCP Lite - Command Line Arguments Update Summary

## Changes Made

### ✅ Server (server.c)
**Enhanced to accept both IP address and port as command line arguments.**

#### Before:
```bash
sudo ./tcp_server [port]
```
- Only port number could be specified
- Always bound to `INADDR_ANY` (all interfaces)

#### After:
```bash
sudo ./tcp_server [ip_address] [port]
sudo ./tcp_server [port]
sudo ./tcp_server
```

**New Features:**
1. **Bind to specific IP address**
   - `sudo ./tcp_server 127.0.0.1 8080` - Localhost only
   - `sudo ./tcp_server 192.168.1.100 8080` - Specific interface
   - `sudo ./tcp_server 0.0.0.0 8080` - All interfaces (explicit)

2. **Backwards compatible**
   - `sudo ./tcp_server 9000` - Still works (binds to 0.0.0.0)
   - `sudo ./tcp_server` - Default to 0.0.0.0:8080

3. **IP address validation**
   - Uses `inet_pton()` to validate IP address format
   - Shows error if invalid IP provided

4. **Better output**
   - Displays both IP and port: `Socket bound to 127.0.0.1:8080`

5. **Improved help message**
   - Shows usage syntax with examples
   - Multiple example commands provided

---

### ✅ Client (client.c)
**Enhanced help messages and consistent argument parsing.**

#### Before:
```bash
sudo ./tcp_client [server_ip] [port]
```
- Basic help message

#### After:
```bash
sudo ./tcp_client [server_ip] [port]
sudo ./tcp_client [server_ip]
sudo ./tcp_client
```

**Improvements:**
1. **Enhanced help message**
   - Clear usage syntax
   - Multiple usage examples
   - Consistent with server format

2. **Better defaults**
   - No args: connects to 127.0.0.1:8080
   - IP only: connects to IP:8080
   - IP + port: connects to IP:port

---

### ✅ Documentation Updates

#### 1. README.md
- Updated "Running the Server" section with new syntax
- Updated "Running the Client" section
- Added syntax descriptions: `sudo ./tcp_server [ip_address] [port]`
- Multiple usage examples for both server and client

#### 2. QUICKSTART.md
- Updated with new command line options
- Added examples for different binding scenarios
- More comprehensive quick start instructions

#### 3. USAGE_EXAMPLES.md (NEW)
**Comprehensive usage guide with 20+ examples including:**
- Default configurations
- Specific port usage
- IP binding scenarios
- Network testing examples
- Security considerations
- Troubleshooting tips
- Quick reference card

#### 4. CHANGELOG.md (NEW)
- Documents version 1.0.0 (initial release)
- Documents version 1.1.0 (command line arguments update)
- Lists all changes and new features

#### 5. test.sh
- Updated to use explicit IP binding
- Better output messages

---

## Usage Examples

### Server Usage

| Command | Behavior |
|---------|----------|
| `sudo ./tcp_server` | Listen on 0.0.0.0:8080 (all interfaces, default port) |
| `sudo ./tcp_server 9000` | Listen on 0.0.0.0:9000 (all interfaces, custom port) |
| `sudo ./tcp_server 127.0.0.1 8080` | Listen on 127.0.0.1:8080 (localhost only) |
| `sudo ./tcp_server 192.168.1.100 8080` | Listen on 192.168.1.100:8080 (specific interface) |

### Client Usage

| Command | Behavior |
|---------|----------|
| `sudo ./tcp_client` | Connect to 127.0.0.1:8080 (default) |
| `sudo ./tcp_client 192.168.1.100` | Connect to 192.168.1.100:8080 |
| `sudo ./tcp_client 192.168.1.100 9000` | Connect to 192.168.1.100:9000 |

---

## Benefits

### 1. Security
- Can now bind server to localhost only (`127.0.0.1`)
- Prevents external network access when testing
- More control over which interface accepts connections

### 2. Flexibility
- Support for multi-homed systems (multiple network interfaces)
- Can test on different interfaces without code changes
- Easier network testing across different subnets

### 3. Usability
- Clear help messages with examples
- Backwards compatible with previous usage
- Consistent syntax between server and client
- Better error messages

### 4. Documentation
- Comprehensive usage examples
- Multiple test scenarios documented
- Troubleshooting guide included

---

## Testing the Changes

### Test 1: Default Behavior
```bash
# Terminal 1
sudo ./tcp_server

# Terminal 2
sudo ./tcp_client
```

**Expected Output (Server):**
```
=== TCP Lite Server ===
Bind Address: 0.0.0.0
Port: 8080
```

---

### Test 2: Localhost Only
```bash
# Terminal 1
sudo ./tcp_server 127.0.0.1 8080

# Terminal 2
sudo ./tcp_client 127.0.0.1 8080
```

**Expected Output (Server):**
```
=== TCP Lite Server ===
Bind Address: 127.0.0.1
Port: 8080

Socket created: 0
Socket bound to 127.0.0.1:8080
```

---

### Test 3: Custom Port
```bash
# Terminal 1
sudo ./tcp_server 9000

# Terminal 2
sudo ./tcp_client 127.0.0.1 9000
```

**Expected Output (Server):**
```
=== TCP Lite Server ===
Bind Address: 0.0.0.0
Port: 9000
```

---

### Test 4: Invalid IP Address
```bash
sudo ./tcp_server 999.999.999.999 8080
```

**Expected Output:**
```
=== TCP Lite Server ===
Bind Address: 999.999.999.999
Port: 8080

Socket created: 0
Invalid IP address: 999.999.999.999
```

---

### Test 5: Help Message
```bash
./tcp_server
```

**Expected Output:**
```
Error: This program requires root privileges (raw sockets)
Usage: sudo ./tcp_server [ip_address] [port]
   or: sudo ./tcp_server [port]
Examples:
  sudo ./tcp_server 8080              # Listen on all interfaces, port 8080
  sudo ./tcp_server 127.0.0.1 8080    # Listen on localhost only, port 8080
  sudo ./tcp_server 0.0.0.0 9000      # Listen on all interfaces, port 9000
```

---

## Build Status

✅ Compiles cleanly with no warnings or errors
```bash
$ make
gcc -Wall -Wextra -O2 -g -c server.c -o server.o
gcc -Wall -Wextra -O2 -g -c tcp_lite.c -o tcp_lite.o
gcc -Wall -Wextra -O2 -g -o tcp_server server.o tcp_lite.o 
gcc -Wall -Wextra -O2 -g -c client.c -o client.o
gcc -Wall -Wextra -O2 -g -o tcp_client client.o tcp_lite.o
```

---

## Files Modified

1. **server.c** - Enhanced command line argument parsing
2. **client.c** - Improved help messages
3. **README.md** - Updated usage instructions
4. **QUICKSTART.md** - Updated quick start guide
5. **test.sh** - Updated with explicit IP binding

## Files Added

1. **USAGE_EXAMPLES.md** - Comprehensive usage guide
2. **CHANGELOG.md** - Version history
3. **UPDATE_SUMMARY.md** - This file

---

## Backwards Compatibility

✅ **100% Backwards Compatible**

All existing usage patterns still work:
- `sudo ./tcp_server 8080` → Works (binds to 0.0.0.0:8080)
- `sudo ./tcp_server` → Works (binds to 0.0.0.0:8080)
- `sudo ./tcp_client 127.0.0.1 8080` → Works
- `sudo ./tcp_client` → Works (connects to 127.0.0.1:8080)

---

## Quick Reference

### Server Syntax
```
sudo ./tcp_server [ip_address] [port]
                   └─ optional ─┘  └optional┘

Defaults:
  ip_address = 0.0.0.0
  port = 8080
```

### Client Syntax
```
sudo ./tcp_client [server_ip] [port]
                   └optional┘  └optional┘

Defaults:
  server_ip = 127.0.0.1
  port = 8080
```

---

## Summary

✅ Server now accepts IP address and port as arguments  
✅ Client has improved help messages  
✅ Full backwards compatibility maintained  
✅ Comprehensive documentation added  
✅ Clean build with no warnings  
✅ IP address validation added  
✅ Better security options (localhost-only binding)  
✅ Enhanced usability with clear examples  

**Version:** 1.1.0  
**Status:** Ready for use  
**Build Status:** ✅ Passing  
**Documentation:** ✅ Complete

