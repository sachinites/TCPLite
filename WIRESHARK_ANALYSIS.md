# Wireshark Packet Analysis - TCP Lite

## Your Wireshark Capture Analysis

### Issue Found: RST Packet in 3-Way Handshake

Looking at your Wireshark capture (packets 1-10):

```
╔════════════════════════════════════════════════════════════════════════╗
║ PACKET ANALYSIS - Server: 10.0.0.2:8080, Client: 10.0.0.1:12345      ║
╚════════════════════════════════════════════════════════════════════════╝

Packet 1 (0.000000):
  10.0.0.1:12345 → 10.0.0.2:8080
  [SYN] Seq=0, Win=65535, Len=0
  ✅ CORRECT: Client initiating connection

Packet 2 (0.001464):
  10.0.0.2:8080 → 10.0.0.1:12345
  [SYN, ACK] Seq=0, Ack=1, Win=65535, Len=0
  ✅ CORRECT: Server responding to SYN

Packet 3 (0.002085):
  10.0.0.1:12345 → 10.0.0.2:8080
  [RST] Seq=1, Win=0, Len=0
  ❌ WRONG: Should be [ACK], not [RST]!
  🔴 THIS IS THE PROBLEM

Packet 4 (0.002469):
  10.0.0.1:12345 → 10.0.0.2:8080
  [ACK] Seq=1, Ack=1, Win=65535, Len=0
  ⚠️  Correct packet, but too late (connection already reset)

Packet 5 (0.003324):
  10.0.0.1:12345 → 10.0.0.2:8080
  [PSH, ACK] Seq=1, Ack=1, Win=65535, Len=14
  ⚠️  Trying to send data, but connection is dead

...

Packet 10 (12.253768):
  10.0.0.1:12345 → 10.0.0.2:8080
  [FIN, ACK] Seq=15, Ack=1, Win=65535, Len=0
  ⚠️  Trying to close, but connection was never established
```

---

## The Problem Explained

### What Should Happen (Correct 3-Way Handshake)

```
┌─────────────┐                           ┌─────────────┐
│   CLIENT    │                           │   SERVER    │
│ 10.0.0.1    │                           │ 10.0.0.2    │
└──────┬──────┘                           └──────┬──────┘
       │                                         │
       │  1. [SYN] Seq=0                         │
       │────────────────────────────────────────>│
       │                                         │
       │         2. [SYN, ACK] Seq=0, Ack=1      │
       │<────────────────────────────────────────│
       │                                         │
       │  3. [ACK] Seq=1, Ack=1                  │
       │────────────────────────────────────────>│
       │                                         │
   [ESTABLISHED]                          [ESTABLISHED]
```

### What Actually Happened (Your Capture)

```
┌─────────────┐                           ┌─────────────┐
│   CLIENT    │                           │   SERVER    │
│ 10.0.0.1    │                           │ 10.0.0.2    │
│             │                           │             │
│ User-space  │    ┌──────────────┐       │ User-space  │
│    TCP      │    │ Linux Kernel │       │    TCP      │
└──────┬──────┘    └──────┬───────┘       └──────┬──────┘
       │                  │                       │
       │  1. [SYN] Seq=0  │                       │
       │─────────────────────────────────────────>│
       │                  │                       │
       │         2. [SYN, ACK] Seq=0, Ack=1       │
       │<─────────────────┬───────────────────────│
       │                  │                       │
       │                  │ (Kernel sees SYN-ACK) │
       │                  │ "I didn't send SYN?!" │
       │                  │                       │
       │  3. [RST] ←──────┘ (Kernel sends RST)    │
       │────────────────────────────────────────> │
       │                                   [CLOSED]
       │  4. [ACK] (Too late)                     │
       │────────────────────────────────────────> X
       │                                          │
```

---

## Root Cause: Kernel TCP/IP Stack Interference

### Why the Kernel Sends RST

1. **Your user-space code sends SYN** using RAW socket
2. **Server responds with SYN-ACK**
3. **Linux kernel receives the SYN-ACK packet**
4. **Kernel checks:** "Do I have a socket listening on port 12345?"
5. **Kernel thinks:** "No! I didn't send a SYN, why am I getting SYN-ACK?"
6. **Kernel action:** Send RST to reject the unexpected packet
7. **Your user-space code** tries to send ACK, but connection is already dead

### Why This Happens

- User-space TCP implementations use RAW sockets
- RAW sockets bypass the kernel's TCP/IP stack
- The kernel doesn't know about your user-space TCP connections
- When kernel receives TCP packets for "unknown" connections, it sends RST

**This is a common issue for ALL user-space TCP implementations!**

---

## The Solution: Block Kernel RST Packets

### Step 1: Setup iptables

```bash
sudo ./setup_iptables.sh
```

This script adds iptables rules to **drop RST packets** before they leave your machine.

### Step 2: Verify Rules

```bash
sudo iptables -L OUTPUT -n -v | grep RST
```

**Expected output:**
```
    0     0 DROP  tcp  --  *  *  0.0.0.0/0  0.0.0.0/0  tcp spt:8080:9000 flags:0x04/0x04
    0     0 DROP  tcp  --  *  *  0.0.0.0/0  0.0.0.0/0  tcp dpt:8080:9000 flags:0x04/0x04
    0     0 DROP  tcp  --  *  *  0.0.0.0/0  0.0.0.0/0  tcp spt:12345:12399 flags:0x04/0x04
    0     0 DROP  tcp  --  *  *  0.0.0.0/0  0.0.0.0/0  tcp dpt:12345:12399 flags:0x04/0x04
```

### Step 3: Run Test Again

```bash
# Terminal 1
sudo ./tcp_server 10.0.0.2 8080

# Terminal 2
sudo ./tcp_client 10.0.0.2 8080
```

### Step 4: Check Wireshark Again

**Now you should see (CORRECT):**

```
Packet 1: [SYN]       Client → Server   Seq=X
Packet 2: [SYN, ACK]  Server → Client   Seq=Y, Ack=X+1
Packet 3: [ACK]       Client → Server   Seq=X+1, Ack=Y+1    ✅ NO RST!
Packet 4: [PSH, ACK]  Client → Server   (Hello, Server!)
Packet 5: [ACK]       Server → Client   Ack=(X+1+14)
Packet 6: [PSH, ACK]  Server → Client   (Echo: Hello, Server!)
...
```

---

## What iptables Does

```
┌─────────────────────────────────────────────────────────────────┐
│                        YOUR MACHINE                             │
│                                                                 │
│  ┌───────────────┐                  ┌─────────────────┐        │
│  │  User-space   │                  │  Linux Kernel   │        │
│  │   TCP Lite    │                  │   TCP/IP Stack  │        │
│  └───────┬───────┘                  └────────┬────────┘        │
│          │                                   │                 │
│          │ SYN-ACK received                  │                 │
│          │<──────────────────────────────────┤                 │
│          │                                   │                 │
│          │                                   │ Want to send RST│
│          │                                   ↓                 │
│          │                          ┌────────────────┐         │
│          │                          │   iptables     │         │
│          │                          │  DROP RST!     │         │
│          │                          └────────────────┘         │
│          │                                   ↓                 │
│          │                                 [DROPPED]           │
│          │                                                     │
│          │ Send ACK (works correctly!)                        │
│          │────────────────────────────────────>                │
│          │                                                     │
└─────────────────────────────────────────────────────────────────┘
```

---

## Testing Procedure

### Complete Test with iptables

```bash
# Step 1: Setup
sudo ./setup_iptables.sh

# Step 2: Start Wireshark
sudo wireshark -i <interface> -k -f "tcp port 8080"
# Use: -i lo  (for loopback)
# Or:  -i eth0 (for network)

# Step 3: Terminal 1 - Server
sudo ./tcp_server 10.0.0.2 8080

# Step 4: Terminal 2 - Client
sudo ./tcp_client 10.0.0.2 8080

# Step 5: Verify in Wireshark - NO RST packets!

# Step 6: Cleanup (when done)
sudo ./cleanup_iptables.sh
```

---

## Expected Wireshark Output (Correct)

### Complete Packet Sequence

```
╔═══════════════════════════════════════════════════════════════════╗
║                    CORRECT PACKET SEQUENCE                        ║
╠═══════════════════════════════════════════════════════════════════╣
║                                                                   ║
║  1. [SYN]       10.0.0.1:12345 → 10.0.0.2:8080                   ║
║     Seq=1000000, Win=65535, Len=0                                ║
║                                                                   ║
║  2. [SYN, ACK]  10.0.0.2:8080 → 10.0.0.1:12345                   ║
║     Seq=500000, Ack=1000001, Win=65535, Len=0                    ║
║                                                                   ║
║  3. [ACK]       10.0.0.1:12345 → 10.0.0.2:8080  ✅ NO RST!       ║
║     Seq=1000001, Ack=500001, Win=65535, Len=0                    ║
║                                                                   ║
║  --- Connection Established ---                                  ║
║                                                                   ║
║  4. [PSH, ACK]  10.0.0.1:12345 → 10.0.0.2:8080                   ║
║     Seq=1000001, Ack=500001, Len=14 ("Hello, Server!")           ║
║                                                                   ║
║  5. [ACK]       10.0.0.2:8080 → 10.0.0.1:12345                   ║
║     Seq=500001, Ack=1000015, Win=65535, Len=0                    ║
║                                                                   ║
║  6. [PSH, ACK]  10.0.0.2:8080 → 10.0.0.1:12345                   ║
║     Seq=500001, Ack=1000015, Len=20 ("Echo: Hello, Server!")     ║
║                                                                   ║
║  7. [ACK]       10.0.0.1:12345 → 10.0.0.2:8080                   ║
║     Seq=1000015, Ack=500021, Win=65535, Len=0                    ║
║                                                                   ║
║  --- More data exchanges ---                                     ║
║                                                                   ║
║  N. [FIN, ACK]  10.0.0.1:12345 → 10.0.0.2:8080                   ║
║  N+1. [ACK]     10.0.0.2:8080 → 10.0.0.1:12345                   ║
║  N+2. [FIN, ACK] 10.0.0.2:8080 → 10.0.0.1:12345                  ║
║  N+3. [ACK]     10.0.0.1:12345 → 10.0.0.2:8080                   ║
║                                                                   ║
║  --- Connection Closed ---                                       ║
║                                                                   ║
╚═══════════════════════════════════════════════════════════════════╝
```

---

## Wireshark Filters for Analysis

### Show only your TCP Lite traffic
```
tcp.port == 8080
```

### Show handshake packets
```
tcp.flags.syn == 1 or tcp.flags.fin == 1
```

### Show PROBLEMS (RST packets)
```
tcp.flags.reset == 1
```
**Note:** After iptables setup, this should show ZERO packets!

### Show data packets
```
tcp.flags.push == 1 and tcp.len > 0
```

### Show sequence numbers
Right-click packet → Protocol Preferences → Relative sequence numbers (uncheck for absolute)

---

## Automated Test

Use the automated test script that handles everything:

```bash
sudo ./test_with_iptables.sh
```

This will:
1. ✅ Build the project
2. ✅ Configure iptables automatically
3. ✅ Run server in background
4. ✅ Run client
5. ✅ Show results

---

## Summary

### The Issue
- ❌ Packet 3 in your capture shows **[RST]** instead of **[ACK]**
- ❌ This is caused by the Linux kernel sending RST packets
- ❌ Kernel doesn't know about user-space TCP connections

### The Solution
- ✅ Use iptables to block kernel RST packets
- ✅ Run `sudo ./setup_iptables.sh` before testing
- ✅ Verify no RST packets in Wireshark after setup

### Verification
1. Setup iptables: `sudo ./setup_iptables.sh`
2. Run test: Server and Client
3. Check Wireshark: Packet 3 should be [ACK], not [RST]
4. Cleanup: `sudo ./cleanup_iptables.sh`

---

For more details, see:
- `TROUBLESHOOTING.md` - Detailed troubleshooting guide
- `README.md` - Full documentation
- `QUICKSTART.md` - Quick start guide

