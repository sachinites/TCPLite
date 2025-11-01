# TCP Lite - Command Line Usage Examples

## Server Usage

**Syntax:**
```bash
sudo ./tcp_server [ip_address] [port]
sudo ./tcp_server [port]
sudo ./tcp_server
```

### Example 1: Default Configuration
Listen on all interfaces (0.0.0.0), port 8080:
```bash
sudo ./tcp_server
```

**Output:**
```
=== TCP Lite Server ===
Bind Address: 0.0.0.0
Port: 8080
```

### Example 2: Specific Port Only
Listen on all interfaces, custom port:
```bash
sudo ./tcp_server 9000
```

**Output:**
```
=== TCP Lite Server ===
Bind Address: 0.0.0.0
Port: 9000
```

### Example 3: Localhost Only
Listen on localhost only (127.0.0.1), port 8080:
```bash
sudo ./tcp_server 127.0.0.1 8080
```

**Output:**
```
=== TCP Lite Server ===
Bind Address: 127.0.0.1
Port: 8080
```

**Use Case:** Only allow connections from the local machine. More secure for testing.

### Example 4: Specific Network Interface
Listen on specific IP address (e.g., 192.168.1.100), port 8080:
```bash
sudo ./tcp_server 192.168.1.100 8080
```

**Output:**
```
=== TCP Lite Server ===
Bind Address: 192.168.1.100
Port: 8080
```

**Use Case:** Bind to a specific network interface in a multi-homed system.

### Example 5: All Interfaces, Custom Port
Listen on all interfaces, port 9000:
```bash
sudo ./tcp_server 0.0.0.0 9000
```

**Output:**
```
=== TCP Lite Server ===
Bind Address: 0.0.0.0
Port: 9000
```

---

## Client Usage

**Syntax:**
```bash
sudo ./tcp_client [server_ip] [port]
sudo ./tcp_client [server_ip]
sudo ./tcp_client
```

### Example 1: Default Configuration
Connect to localhost (127.0.0.1), port 8080:
```bash
sudo ./tcp_client
```

**Output:**
```
=== TCP Lite Client ===
Server: 127.0.0.1:8080
```

### Example 2: Specific Server, Default Port
Connect to specific server, port 8080:
```bash
sudo ./tcp_client 192.168.1.100
```

**Output:**
```
=== TCP Lite Client ===
Server: 192.168.1.100:8080
```

### Example 3: Specific Server and Port
Connect to 192.168.1.100, port 9000:
```bash
sudo ./tcp_client 192.168.1.100 9000
```

**Output:**
```
=== TCP Lite Client ===
Server: 192.168.1.100:9000
```

### Example 4: Localhost with Custom Port
Connect to localhost, port 9000:
```bash
sudo ./tcp_client 127.0.0.1 9000
```

**Output:**
```
=== TCP Lite Client ===
Server: 127.0.0.1:9000
```

---

## Complete Test Scenarios

### Scenario 1: Local Testing (Loopback)
Most common for testing on a single machine.

**Terminal 1 (Server):**
```bash
sudo ./tcp_server 127.0.0.1 8080
```

**Terminal 2 (Client):**
```bash
sudo ./tcp_client 127.0.0.1 8080
```

---

### Scenario 2: Network Testing (Same Network)
Server on 192.168.1.100, client on 192.168.1.101.

**Server Machine (192.168.1.100):**
```bash
sudo ./tcp_server 192.168.1.100 8080
```

**Client Machine (192.168.1.101):**
```bash
sudo ./tcp_client 192.168.1.100 8080
```

---

### Scenario 3: Multiple Servers on Same Machine
Run multiple servers on different ports.

**Terminal 1:**
```bash
sudo ./tcp_server 127.0.0.1 8080
```

**Terminal 2:**
```bash
sudo ./tcp_server 127.0.0.1 9000
```

**Terminal 3 (Client for port 8080):**
```bash
sudo ./tcp_client 127.0.0.1 8080
```

**Terminal 4 (Client for port 9000):**
```bash
sudo ./tcp_client 127.0.0.1 9000
```

---

### Scenario 4: Public Server (Accept from Any IP)
Server accepts connections from any IP address.

**Server:**
```bash
sudo ./tcp_server 0.0.0.0 8080
```

**Client (from any machine):**
```bash
sudo ./tcp_client <server_ip> 8080
```

---

## Help Messages

### Server Help
```bash
./tcp_server
```

**Output:**
```
Error: This program requires root privileges (raw sockets)
Usage: sudo ./tcp_server [ip_address] [port]
   or: sudo ./tcp_server [port]
Examples:
  sudo ./tcp_server 8080              # Listen on all interfaces, port 8080
  sudo ./tcp_server 127.0.0.1 8080    # Listen on localhost only, port 8080
  sudo ./tcp_server 0.0.0.0 9000      # Listen on all interfaces, port 9000
```

### Client Help
```bash
./tcp_client
```

**Output:**
```
Error: This program requires root privileges (raw sockets)
Usage: sudo ./tcp_client [server_ip] [port]
Examples:
  sudo ./tcp_client                        # Connect to 127.0.0.1:8080 (default)
  sudo ./tcp_client 192.168.1.100          # Connect to 192.168.1.100:8080
  sudo ./tcp_client 192.168.1.100 9000     # Connect to 192.168.1.100:9000
```

---

## Common Use Cases

### 1. Quick Local Test
```bash
# Terminal 1
sudo ./tcp_server

# Terminal 2
sudo ./tcp_client
```

### 2. Test on Custom Port (avoid conflicts)
```bash
# Terminal 1
sudo ./tcp_server 9999

# Terminal 2
sudo ./tcp_client 127.0.0.1 9999
```

### 3. Secure Local-Only Server
```bash
# Server only accepts localhost connections
sudo ./tcp_server 127.0.0.1 8080
```

### 4. Multi-Interface Server
If your server has multiple network interfaces (e.g., eth0: 192.168.1.100, wlan0: 10.0.0.50):

```bash
# Listen only on eth0
sudo ./tcp_server 192.168.1.100 8080

# Or listen only on wlan0
sudo ./tcp_server 10.0.0.50 8080

# Or listen on all interfaces
sudo ./tcp_server 0.0.0.0 8080
```

---

## Monitoring Connections

### Using tcpdump
Monitor packets on specific port:

```bash
# Terminal 1: Start tcpdump
sudo tcpdump -i lo port 8080 -vv

# Terminal 2: Start server
sudo ./tcp_server 127.0.0.1 8080

# Terminal 3: Run client
sudo ./tcp_client 127.0.0.1 8080
```

### Using netstat
Check listening ports:

```bash
# Check if server is listening
sudo netstat -tulpn | grep 8080

# Check active connections
sudo netstat -an | grep 8080
```

---

## Troubleshooting

### Error: "Invalid IP address"
**Problem:** Incorrect IP address format.

**Solution:**
```bash
# Bad
sudo ./tcp_server 192.168.1 8080

# Good
sudo ./tcp_server 192.168.1.100 8080
```

### Error: "Permission denied"
**Problem:** Not running with root privileges.

**Solution:**
```bash
# Bad
./tcp_server 8080

# Good
sudo ./tcp_server 8080
```

### Error: "Connection refused"
**Problem:** Server not running or wrong IP/port.

**Solution:**
1. Verify server is running:
   ```bash
   sudo netstat -tulpn | grep 8080
   ```

2. Check IP address and port match:
   ```bash
   # Server
   sudo ./tcp_server 127.0.0.1 8080
   
   # Client (must match)
   sudo ./tcp_client 127.0.0.1 8080
   ```

### Error: "Address already in use"
**Problem:** Port already in use by another process.

**Solution:**
```bash
# Check what's using the port
sudo netstat -tulpn | grep 8080

# Kill the process or use a different port
sudo ./tcp_server 127.0.0.1 9000
```

---

## Security Considerations

### Binding to Specific IPs

**Most Secure:** Bind to localhost only
```bash
sudo ./tcp_server 127.0.0.1 8080
```
✅ Only local connections allowed  
✅ No external exposure

**Least Secure:** Bind to all interfaces
```bash
sudo ./tcp_server 0.0.0.0 8080
```
⚠️  Accepts connections from any IP  
⚠️  Exposed to network

**Recommended:** Bind to specific interface
```bash
sudo ./tcp_server 192.168.1.100 8080
```
✓ Only specified network can connect  
✓ Better control

---

## Advanced Examples

### Testing Across VLANs
```bash
# Server on VLAN 1 (192.168.1.100)
sudo ./tcp_server 192.168.1.100 8080

# Client on VLAN 2 (192.168.2.100)
sudo ./tcp_client 192.168.1.100 8080
```

### Load Testing (Multiple Clients)
```bash
# Start server
sudo ./tcp_server 127.0.0.1 8080

# Terminal 2
sudo ./tcp_client 127.0.0.1 8080

# Note: This implementation handles one connection at a time
# For multiple simultaneous connections, modify the code
```

### Port Scanning Testing
```bash
# Start servers on different ports
for port in 8080 8081 8082 8083; do
    sudo ./tcp_server 127.0.0.1 $port &
done

# Test each port
for port in 8080 8081 8082 8083; do
    sudo ./tcp_client 127.0.0.1 $port
done
```

---

## Summary

| Argument Pattern | Server Behavior | Example |
|-----------------|----------------|---------|
| No arguments | Listen on 0.0.0.0:8080 | `sudo ./tcp_server` |
| Port only | Listen on 0.0.0.0:port | `sudo ./tcp_server 9000` |
| IP + Port | Listen on IP:port | `sudo ./tcp_server 127.0.0.1 8080` |

| Argument Pattern | Client Behavior | Example |
|-----------------|----------------|---------|
| No arguments | Connect to 127.0.0.1:8080 | `sudo ./tcp_client` |
| IP only | Connect to IP:8080 | `sudo ./tcp_client 192.168.1.100` |
| IP + Port | Connect to IP:port | `sudo ./tcp_client 192.168.1.100 9000` |

---

**Quick Reference Card:**
```
┌────────────────────────────────────────────────────────────┐
│ SERVER                                                      │
├─────────────────────────────────────────────────────────────┤
│ sudo ./tcp_server                → 0.0.0.0:8080 (default)  │
│ sudo ./tcp_server 9000           → 0.0.0.0:9000            │
│ sudo ./tcp_server 127.0.0.1 8080 → 127.0.0.1:8080          │
├─────────────────────────────────────────────────────────────┤
│ CLIENT                                                      │
├─────────────────────────────────────────────────────────────┤
│ sudo ./tcp_client                → 127.0.0.1:8080 (default)│
│ sudo ./tcp_client 192.168.1.100  → 192.168.1.100:8080      │
│ sudo ./tcp_client 10.0.0.1 9000  → 10.0.0.1:9000           │
└─────────────────────────────────────────────────────────────┘
```

