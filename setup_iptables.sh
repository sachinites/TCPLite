#!/bin/bash

# TCP Lite - iptables Setup Script
# This script prevents the kernel from interfering with user-space TCP

echo "=========================================="
echo "TCP Lite - iptables Setup"
echo "=========================================="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root"
    echo "Please run: sudo ./setup_iptables.sh"
    exit 1
fi

echo "Setting up iptables rules to prevent kernel RST packets..."
echo ""

# Remove existing rules for these ports (if any)
echo "Cleaning up existing rules..."
iptables -D OUTPUT -p tcp --tcp-flags RST RST --sport 8080:9000 -j DROP 2>/dev/null
iptables -D OUTPUT -p tcp --tcp-flags RST RST --dport 8080:9000 -j DROP 2>/dev/null
iptables -D OUTPUT -p tcp --tcp-flags RST RST --sport 12345:12399 -j DROP 2>/dev/null
iptables -D OUTPUT -p tcp --tcp-flags RST RST --dport 12345:12399 -j DROP 2>/dev/null

# Add new rules
echo "Adding new iptables rules..."
echo ""

# Server ports (8080-9000)
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --sport 8080:9000 -j DROP
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --dport 8080:9000 -j DROP
echo "✓ Blocking RST packets for server ports 8080-9000"

# Client ports (12345-12399)
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --sport 12345:12399 -j DROP
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST --dport 12345:12399 -j DROP
echo "✓ Blocking RST packets for client ports 12345-12399"

echo ""
echo "=========================================="
echo "iptables rules successfully configured!"
echo "=========================================="
echo ""
echo "Current OUTPUT chain rules:"
iptables -L OUTPUT -n -v | grep -E "(RST|Chain)"
echo ""
echo "You can now run your TCP Lite programs:"
echo "  Terminal 1: sudo ./tcp_server 10.0.0.2 8080"
echo "  Terminal 2: sudo ./tcp_client 10.0.0.2 8080"
echo ""
echo "To remove these rules later, run:"
echo "  sudo ./cleanup_iptables.sh"
echo ""

