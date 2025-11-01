#!/bin/bash

# TCP Lite - iptables Cleanup Script
# This script removes the iptables rules added by setup_iptables.sh

echo "=========================================="
echo "TCP Lite - iptables Cleanup"
echo "=========================================="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root"
    echo "Please run: sudo ./cleanup_iptables.sh"
    exit 1
fi

echo "Removing iptables rules..."
echo ""

# Remove rules
iptables -D OUTPUT -p tcp --tcp-flags RST RST --sport 8080:9000 -j DROP 2>/dev/null
if [ $? -eq 0 ]; then
    echo "✓ Removed rule: RST block for server source ports 8080-9000"
else
    echo "  (Rule not found or already removed)"
fi

iptables -D OUTPUT -p tcp --tcp-flags RST RST --dport 8080:9000 -j DROP 2>/dev/null
if [ $? -eq 0 ]; then
    echo "✓ Removed rule: RST block for server destination ports 8080-9000"
else
    echo "  (Rule not found or already removed)"
fi

iptables -D OUTPUT -p tcp --tcp-flags RST RST --sport 12345:12399 -j DROP 2>/dev/null
if [ $? -eq 0 ]; then
    echo "✓ Removed rule: RST block for client source ports 12345-12399"
else
    echo "  (Rule not found or already removed)"
fi

iptables -D OUTPUT -p tcp --tcp-flags RST RST --dport 12345:12399 -j DROP 2>/dev/null
if [ $? -eq 0 ]; then
    echo "✓ Removed rule: RST block for client destination ports 12345-12399"
else
    echo "  (Rule not found or already removed)"
fi

echo ""
echo "=========================================="
echo "Cleanup complete!"
echo "=========================================="
echo ""

