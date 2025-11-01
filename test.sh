#!/bin/bash

# TCP Lite Test Script
# This script demonstrates the usage of TCP Lite

echo "=============================="
echo "TCP Lite Test Script"
echo "=============================="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root (requires raw sockets)"
    echo "Please run: sudo ./test.sh"
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
echo "=============================="
echo "Starting Test"
echo "=============================="
echo ""
echo "This will start the server in the background and run the client."
echo "Press Ctrl+C to stop."
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

