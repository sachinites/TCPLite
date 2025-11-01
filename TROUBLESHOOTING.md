# TCP Lite - Troubleshooting Guide

## Critical Issue: Kernel RST Packets

### 🔴 Problem: RST Packets Killing Connections

If you see this in Wireshark:

```
Packet 1: [SYN]        Client → Server   ✓ Correct
Packet 2: [SYN, ACK]   Server → Client   ✓ Correct
Packet 3: [RST]        Client → Server   ✗ WRONG! (Should be ACK)
Packet 4: [ACK]        Client → Server   ✓ Correct (but too late)
```

**This means the Linux kernel is interfering with your user-space TCP!**

### Why This Happens

1. Your user-space client sends SYN using RAW socket
2. Server responds with SYN-ACK
3. **The Linux kernel receives the SYN-ACK**
4. Kernel thinks: "I didn't send a SYN, why am I getting SYN-ACK?"
5. **Kernel automatically sends RST** to reject the connection
6. Your user-space code tries to send ACK, but connection is already dead

This is a common issue with **all user-space TCP implementations** using RAW sockets.

---

## ✅ Solution: Block Kernel RST Packets with iptables

### Quick Fix (Recommended)

Run the automated setup script:

```bash
sudo ./setup_iptables.sh
```

This will configure iptables to drop RST packets for your test ports.

### Manual Fix

```bash
# Block RST packets for server ports (8080-9000)
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --sport 8080:9000 -j DROP
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --dport 8080:9000 -j DROP

# Block RST packets for client ports (12345-12399)
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --sport 12345:12399 -j DROP
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --dport 12345:12399 -j DROP
```

### Verify iptables Rules

```bash
sudo iptables -L OUTPUT -n -v | grep RST
```

**Expected output:**
```
    0     0 DROP       tcp  --  *      *       0.0.0.0/0            0.0.0.0/0            tcp spt:8080:9000 flags:0x04/0x04
    0     0 DROP       tcp  --  *      *       0.0.0.0/0            0.0.0.0/0            tcp dpt:8080:9000 flags:0x04/0x04
    0     0 DROP       tcp  --  *      *       0.0.0.0/0            0.0.0.0/0            tcp spt:12345:12399 flags:0x04/0x04
    0     0 DROP       tcp  --  *      *       0.0.0.0/0            0.0.0.0/0            tcp dpt:12345:12399 flags:0x04/0x04
```

---

## Testing Procedure

### Step 1: Setup iptables
```bash
sudo ./setup_iptables.sh
```

### Step 2: Run Server
```bash
sudo ./tcp_server 10.0.0.2 8080
```

### Step 3: Run Client (in another terminal)
```bash
sudo ./tcp_client 10.0.0.2 8080
```

### Step 4: Verify in Wireshark

**Expected packet sequence (CORRECT):**
```
Packet 1: [SYN]        Client → Server   Seq=X
Packet 2: [SYN, ACK]   Server → Client   Seq=Y, Ack=X+1
Packet 3: [ACK]        Client → Server   Seq=X+1, Ack=Y+1    ✓ NO RST!
Packet 4: [PSH, ACK]   Client → Server   (data)
Packet 5: [ACK]        Server → Client   
Packet 6: [PSH, ACK]   Server → Client   (echo data)
...
```

### Step 5: Cleanup (when done testing)
```bash
sudo ./cleanup_iptables.sh
```

---

## Alternative: Automated Test with iptables

Use the automated test script that handles iptables for you:

```bash
sudo ./test_with_iptables.sh
```

This script will:
1. Build the project
2. Configure iptables automatically
3. Run server in background
4. Run client
5. Show results

---

## Common Issues and Solutions

### Issue 1: "Connection timeout"

**Symptoms:**
- Client says "SYN-ACK timeout"
- Server shows "Waiting for incoming connection..."
- Wireshark shows only SYN, no SYN-ACK

**Causes:**
1. Server not running
2. Wrong IP address
3. Firewall blocking packets

**Solutions:**
```bash
# Check if server is running
ps aux | grep tcp_server

# Check IP addresses
ip addr show

# Check iptables (make sure you're not blocking everything)
sudo iptables -L -n -v

# Try on loopback first
sudo ./tcp_server 127.0.0.1 8080
sudo ./tcp_client 127.0.0.1 8080
```

---

### Issue 2: RST packets still appearing

**Symptoms:**
- Still see RST in Wireshark after setting up iptables

**Solutions:**

1. **Verify iptables rules are active:**
```bash
sudo iptables -L OUTPUT -n -v | grep RST
```

2. **Check if using correct ports:**
```bash
# If using different ports, update iptables
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --sport YOUR_PORT -j DROP
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --dport YOUR_PORT -j DROP
```

3. **Verify port numbers in code:**
```bash
# Client source port should be 12345 (see client.c)
grep "htons" client.c | grep local_addr
# Should show: local_addr.sin_port = htons(12345);
```

---

### Issue 3: "Permission denied"

**Symptoms:**
```
Error: This program requires root privileges (raw sockets)
Raw socket creation failed: Operation not permitted
```

**Solution:**
```bash
# Always run with sudo
sudo ./tcp_server 10.0.0.2 8080
sudo ./tcp_client 10.0.0.2 8080
```

---

### Issue 4: Packets not visible in Wireshark

**Symptoms:**
- Program runs but Wireshark shows no packets

**Solutions:**

1. **Capture on correct interface:**
```bash
# For loopback testing
sudo wireshark -i lo -k

# For network testing
sudo wireshark -i eth0 -k
```

2. **Apply correct filter:**
```
tcp.port == 8080
```

3. **Check if using correct IP:**
```bash
# List network interfaces
ip addr show
```

---

### Issue 5: "Invalid IP address"

**Symptoms:**
```
Invalid IP address: 10.0.0.300
```

**Solution:**
```bash
# Check IP format (must be valid IPv4)
# Bad: 10.0.0.300, 192.168.1, 999.999.999.999
# Good: 10.0.0.1, 192.168.1.100, 127.0.0.1

# Verify your IP
ip addr show
```

---

## Wireshark Analysis Tips

### Correct 3-Way Handshake
```
1. [SYN]        Seq=X
2. [SYN, ACK]   Seq=Y, Ack=X+1
3. [ACK]        Seq=X+1, Ack=Y+1
```

### Incorrect (with kernel RST)
```
1. [SYN]        Seq=X
2. [SYN, ACK]   Seq=Y, Ack=X+1
3. [RST]        ← WRONG! Kernel interfering
4. [ACK]        ← Too late, connection dead
```

### Data Transfer
```
[PSH, ACK]  Client → Server (data)
[ACK]       Server → Client (acknowledge)
[PSH, ACK]  Server → Client (echo)
[ACK]       Client → Server (acknowledge)
```

### Connection Termination
```
[FIN, ACK]  Client → Server
[ACK]       Server → Client
[FIN, ACK]  Server → Client
[ACK]       Client → Server
```

---

## Wireshark Filters

### Show only TCP Lite traffic
```
tcp.port == 8080
```

### Show only handshake packets
```
tcp.flags.syn == 1 or tcp.flags.fin == 1
```

### Show problems
```
tcp.flags.reset == 1
```

### Show data packets
```
tcp.flags.push == 1 and tcp.len > 0
```

### Filter by IP
```
ip.addr == 10.0.0.1 or ip.addr == 10.0.0.2
```

---

## Network Interface Configuration

### For Loopback Testing (Same Machine)
```bash
# Server
sudo ./tcp_server 127.0.0.1 8080

# Client
sudo ./tcp_client 127.0.0.1 8080

# Wireshark
sudo wireshark -i lo -k -f "tcp port 8080"
```

### For Network Testing (Different Machines)

**Server Machine (10.0.0.2):**
```bash
# Check interface IP
ip addr show

# Run server
sudo ./tcp_server 10.0.0.2 8080

# Setup iptables
sudo ./setup_iptables.sh
```

**Client Machine (10.0.0.1):**
```bash
# Run client
sudo ./tcp_client 10.0.0.2 8080

# Setup iptables
sudo ./setup_iptables.sh
```

**Wireshark (on either machine):**
```bash
# Capture on network interface (e.g., eth0, ens33)
sudo wireshark -i eth0 -k -f "tcp port 8080"
```

---

## Debugging Commands

### Check if port is in use
```bash
sudo netstat -tulpn | grep 8080
sudo lsof -i :8080
```

### Monitor packets in terminal
```bash
sudo tcpdump -i lo port 8080 -vv -X
```

### Check iptables
```bash
# List all OUTPUT rules
sudo iptables -L OUTPUT -n -v

# List RST-specific rules
sudo iptables -L OUTPUT -n -v | grep RST

# Count dropped packets
sudo iptables -L OUTPUT -n -v -x | grep RST
```

### Verify RAW socket capability
```bash
# Check capabilities
getcap /path/to/tcp_server

# Or just run with sudo
sudo ./tcp_server 127.0.0.1 8080
```

---

## FAQ

### Q: Why do I need iptables rules?

**A:** Because the Linux kernel's TCP stack doesn't know about your user-space TCP connections. When it receives TCP packets for ports it's not listening on, it automatically sends RST packets to reject them. The iptables rules prevent the kernel from sending these RST packets.

### Q: Will iptables rules affect other programs?

**A:** Only if they use ports 8080-9000 or 12345-12399. Our rules are specific to these port ranges. Other programs using different ports are unaffected.

### Q: Do I need to setup iptables on both client and server?

**A:** Yes, if they're on different machines. Both machines' kernels will try to send RST packets, so both need iptables rules.

### Q: Can I use different ports?

**A:** Yes, but you'll need to update the iptables rules for those ports:
```bash
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --sport YOUR_PORT -j DROP
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --dport YOUR_PORT -j DROP
```

### Q: How do I permanently remove iptables rules?

**A:** Run the cleanup script:
```bash
sudo ./cleanup_iptables.sh
```

Or manually:
```bash
sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST --sport 8080:9000 -j DROP
sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST --dport 8080:9000 -j DROP
sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST --sport 12345:12399 -j DROP
sudo iptables -D OUTPUT -p tcp --tcp-flags RST RST --dport 12345:12399 -j DROP
```

---

## Summary Checklist

Before running TCP Lite:

- [ ] Build project: `make`
- [ ] Setup iptables: `sudo ./setup_iptables.sh`
- [ ] Run server: `sudo ./tcp_server <ip> <port>`
- [ ] Run client: `sudo ./tcp_client <ip> <port>`
- [ ] Monitor with Wireshark or tcpdump
- [ ] Verify no RST packets in capture
- [ ] Cleanup when done: `sudo ./cleanup_iptables.sh`

---

## Quick Test

```bash
# One-liner to test everything
sudo ./test_with_iptables.sh
```

This handles everything automatically!

---

For more information, see:
- `README.md` - Full documentation
- `USAGE_EXAMPLES.md` - Usage examples
- `QUICKSTART.md` - Quick start guide

