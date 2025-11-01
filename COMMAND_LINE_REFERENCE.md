# TCP Lite - Command Line Reference Card

## Quick Reference

```
╔═══════════════════════════════════════════════════════════════════════════╗
║                        TCP LITE - SERVER COMMANDS                         ║
╠═══════════════════════════════════════════════════════════════════════════╣
║                                                                           ║
║  SYNTAX:  sudo ./tcp_server [ip_address] [port]                          ║
║                                                                           ║
║  ┌─────────────────────────────────────────────────────────────────┐     ║
║  │ COMMAND                          │ BINDS TO                     │     ║
║  ├─────────────────────────────────────────────────────────────────┤     ║
║  │ sudo ./tcp_server                │ 0.0.0.0:8080 (default)       │     ║
║  │ sudo ./tcp_server 9000           │ 0.0.0.0:9000                 │     ║
║  │ sudo ./tcp_server 127.0.0.1 8080 │ 127.0.0.1:8080 (localhost)   │     ║
║  │ sudo ./tcp_server 192.168.1.10   │ 192.168.1.10:8080            │     ║
║  │        8080                      │                               │     ║
║  └─────────────────────────────────────────────────────────────────┘     ║
║                                                                           ║
╚═══════════════════════════════════════════════════════════════════════════╝

╔═══════════════════════════════════════════════════════════════════════════╗
║                        TCP LITE - CLIENT COMMANDS                         ║
╠═══════════════════════════════════════════════════════════════════════════╣
║                                                                           ║
║  SYNTAX:  sudo ./tcp_client [server_ip] [port]                           ║
║                                                                           ║
║  ┌─────────────────────────────────────────────────────────────────┐     ║
║  │ COMMAND                          │ CONNECTS TO                  │     ║
║  ├─────────────────────────────────────────────────────────────────┤     ║
║  │ sudo ./tcp_client                │ 127.0.0.1:8080 (default)     │     ║
║  │ sudo ./tcp_client 192.168.1.100  │ 192.168.1.100:8080           │     ║
║  │ sudo ./tcp_client 192.168.1.100  │ 192.168.1.100:9000           │     ║
║  │        9000                      │                               │     ║
║  │ sudo ./tcp_client 10.0.0.50 7777 │ 10.0.0.50:7777               │     ║
║  └─────────────────────────────────────────────────────────────────┘     ║
║                                                                           ║
╚═══════════════════════════════════════════════════════════════════════════╝
```

---

## Common Usage Patterns

### Pattern 1: Quick Local Test
```bash
Terminal 1:  sudo ./tcp_server
Terminal 2:  sudo ./tcp_client
```
→ Server on 0.0.0.0:8080, Client connects to 127.0.0.1:8080

---

### Pattern 2: Localhost Only (Secure)
```bash
Terminal 1:  sudo ./tcp_server 127.0.0.1 8080
Terminal 2:  sudo ./tcp_client 127.0.0.1 8080
```
→ Server only accepts local connections

---

### Pattern 3: Custom Port
```bash
Terminal 1:  sudo ./tcp_server 9000
Terminal 2:  sudo ./tcp_client 127.0.0.1 9000
```
→ Avoid port conflicts

---

### Pattern 4: Network Testing
```bash
Server:  sudo ./tcp_server 192.168.1.100 8080
Client:  sudo ./tcp_client 192.168.1.100 8080
```
→ Test across network

---

## Argument Rules

### Server

| Arguments | IP Address | Port | Example |
|-----------|------------|------|---------|
| 0 args | `0.0.0.0` | `8080` | `sudo ./tcp_server` |
| 1 arg (numeric) | `0.0.0.0` | arg1 | `sudo ./tcp_server 9000` |
| 2 args | arg1 | arg2 | `sudo ./tcp_server 127.0.0.1 8080` |

### Client

| Arguments | Server IP | Port | Example |
|-----------|-----------|------|---------|
| 0 args | `127.0.0.1` | `8080` | `sudo ./tcp_client` |
| 1 arg (IP) | arg1 | `8080` | `sudo ./tcp_client 192.168.1.100` |
| 2 args | arg1 | arg2 | `sudo ./tcp_client 192.168.1.100 9000` |

---

## Special IP Addresses

| IP Address | Meaning | Server Behavior |
|------------|---------|-----------------|
| `0.0.0.0` | All interfaces | Accept connections from any IP |
| `127.0.0.1` | Localhost | Only accept local connections |
| `192.168.x.x` | Private network | Only accept from same network |
| Specific IP | Single interface | Bind to one network interface |

---

## Error Messages

### "Permission denied" / "Raw socket creation failed"
```
Error: This program requires root privileges (raw sockets)
```
**Fix:** Run with `sudo`

---

### "Invalid IP address"
```
Invalid IP address: 999.999.999.999
```
**Fix:** Use valid IP format (e.g., `192.168.1.100`)

---

### "Connection timeout"
**Possible causes:**
1. Server not running
2. Wrong IP address
3. Firewall blocking
4. Wrong port number

**Fix:**
```bash
# Verify server is running
sudo netstat -tulpn | grep 8080

# Check IP/port match between server and client
```

---

## Network Interface Examples

### Single Interface System
```bash
# Loopback only (lo: 127.0.0.1)
sudo ./tcp_server 127.0.0.1 8080
```

### Multi-Interface System
```bash
# Ethernet (eth0: 192.168.1.100)
sudo ./tcp_server 192.168.1.100 8080

# WiFi (wlan0: 10.0.0.50)
sudo ./tcp_server 10.0.0.50 8080

# All interfaces
sudo ./tcp_server 0.0.0.0 8080
```

---

## Port Numbers

### Well-Known Ports (0-1023)
⚠️ Avoid: May conflict with system services

### Registered Ports (1024-49151)
✓ Safe for testing

### Dynamic Ports (49152-65535)
✓ Safe for testing

**Recommended test ports:** 8080, 8081, 9000, 9001, 12345

---

## Security Levels

```
┌──────────────────────────────────────────────────────────────┐
│ MOST SECURE                                                  │
├──────────────────────────────────────────────────────────────┤
│ sudo ./tcp_server 127.0.0.1 8080                             │
│ → Only localhost can connect                                 │
│ → No network exposure                                        │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│ MEDIUM SECURITY                                              │
├──────────────────────────────────────────────────────────────┤
│ sudo ./tcp_server 192.168.1.100 8080                         │
│ → Only specific network interface                            │
│ → Limited network exposure                                   │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│ LEAST SECURE                                                 │
├──────────────────────────────────────────────────────────────┤
│ sudo ./tcp_server 0.0.0.0 8080                               │
│ → All interfaces accept connections                          │
│ → Full network exposure                                      │
└──────────────────────────────────────────────────────────────┘
```

---

## Monitoring Commands

### Check if server is listening
```bash
sudo netstat -tulpn | grep tcp_server
```

### Watch packets in real-time
```bash
sudo tcpdump -i lo port 8080 -vv
```

### See active connections
```bash
sudo netstat -an | grep 8080
```

### Check what's using a port
```bash
sudo lsof -i :8080
```

---

## Keyboard Shortcuts

| Action | Command |
|--------|---------|
| Stop server/client | `Ctrl+C` |
| Background process | Add `&` at end |
| View help | Run without `sudo` |

---

## Troubleshooting Flow

```
┌─────────────────────────────────┐
│  Connection not working?        │
└──────────┬──────────────────────┘
           │
           ▼
┌─────────────────────────────────┐
│  Is server running with sudo?   │
└──────────┬─────┬────────────────┘
           │     │
       NO  │     │  YES
           ▼     ▼
    ┌──────────┐ ┌──────────────────────┐
    │ Run with │ │ Do IP and port match?│
    │   sudo   │ └──────────┬───────────┘
    └──────────┘            │
                        NO  │  YES
                            ▼  ▼
                  ┌──────────┐ ┌────────────────┐
                  │ Fix addr │ │ Check firewall │
                  └──────────┘ └────────────────┘
```

---

## Tips & Tricks

### 1. Run server in background
```bash
sudo ./tcp_server 127.0.0.1 8080 &
```

### 2. Run multiple servers
```bash
sudo ./tcp_server 127.0.0.1 8080 &
sudo ./tcp_server 127.0.0.1 9000 &
```

### 3. Kill background server
```bash
sudo pkill tcp_server
```

### 4. Use test script
```bash
sudo ./test.sh
```

---

## Documentation Files

| File | Description |
|------|-------------|
| `README.md` | Full documentation |
| `QUICKSTART.md` | Quick start guide |
| `USAGE_EXAMPLES.md` | 20+ usage examples |
| `UPDATE_SUMMARY.md` | Latest changes |
| `CHANGELOG.md` | Version history |
| `COMMAND_LINE_REFERENCE.md` | This file |

---

## One-Liners

```bash
# Quick test
sudo ./tcp_server & sleep 1 && sudo ./tcp_client ; sudo pkill tcp_server

# Monitor packets while testing
sudo tcpdump -i lo port 8080 -X & sudo ./tcp_server 127.0.0.1 8080

# Test with custom port
sudo ./tcp_server 9999 & sleep 1 && sudo ./tcp_client 127.0.0.1 9999

# Run and log
sudo ./tcp_server 127.0.0.1 8080 > server.log 2>&1 &
```

---

## Version Info

**TCP Lite Version:** 1.1.0  
**Features:** IP binding, port selection, BSD socket-like API  
**Status:** Production-ready for educational use  

---

**Print this page for quick reference while testing!**

