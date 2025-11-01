#!/bin/bash

# TCP Lite Test Script with iptables Setup
# This script automatically configures iptables and runs the test

echo "=============================="
echo "TCP Lite Test (with iptables)"
echo "=============================="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root (requires raw sockets and iptables)"
    echo "Please run: sudo ./test_with_iptables.sh"
    exit 1
fi

# Build the project
echo "Building TCP Lite..."
make clean > /dev/null 2>&1
make

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Build successful!"
echo ""

# Setup iptables
echo "=============================="
echo "Configuring iptables..."
echo "=============================="
echo ""

# Remove existing rules (if any)
iptables -D OUTPUT -p tcp --tcp-flags RST RST --sport 8080:9000 -j DROP 2>/dev/null
iptables -D OUTPUT -p tcp --tcp-flags RST RST --dport 8080:9000 -j DROP 2>/dev/null
iptables -D OUTPUT -p tcp --tcp-flags RST RST --sport 12345:12399 -j DROP 2>/dev/null
iptables -D OUTPUT -p tcp --tcp-flags RST RST --dport 12345:12399 -j DROP 2>/dev/null

# Add new rules
iptables -A OUTPUT -p tcp --tcp-flags RST RST --sport 8080:9000 -j DROP
iptables -A OUTPUT -p tcp --tcp-flags RST RST --dport 8080:9000 -j DROP
iptables -A OUTPUT -p tcp --tcp-flags RST RST --sport 12345:12399 -j DROP
iptables -A OUTPUT -p tcp --tcp-flags RST RST --dport 12345:12399 -j DROP

echo "✓ iptables configured to block kernel RST packets"
echo ""

echo "=============================="
echo "Starting Test"
echo "=============================="
echo ""
echo "This will start the server in the background and run the client."
echo "The iptables rules will prevent kernel interference."
echo ""

# Start server in background
echo "Starting server on 127.0.0.1:8080..."
./tcp_server 127.0.0.1 8080 > server.log 2>&1 &
SERVER_PID=$!

echo "Server started with PID: $SERVER_PID"
echo ""

# Wait for server to initialize
sleep 2

# Run client
echo "Starting client connecting to 127.0.0.1:8080..."
echo ""
./tcp_client 127.0.0.1 8080

# Wait a bit
sleep 2

# Kill server
echo ""
echo "Stopping server..."
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo ""
echo "=============================="
echo "Test Complete"
echo "=============================="
echo ""
echo "Server log saved to: server.log"
echo ""
echo "Note: iptables rules are still active."
echo "To remove them, run: sudo ./cleanup_iptables.sh"
echo ""

