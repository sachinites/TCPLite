CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = 

# Source files
LIB_SRC = tcp_lite.c
LIB_OBJ = $(LIB_SRC:.c=.o)
LIB_HDR = tcp_lite.h

SERVER_SRC = server.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_BIN = tcp_server

CLIENT_SRC = client.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
CLIENT_BIN = tcp_client

# Build targets
all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJ) $(LIB_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_OBJ) $(LIB_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(LIB_HDR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN) $(LIB_OBJ) $(SERVER_OBJ) $(CLIENT_OBJ)

# Run targets (require root)
run-server: $(SERVER_BIN)
	@echo "Starting server (requires root)..."
	sudo ./$(SERVER_BIN)

run-client: $(CLIENT_BIN)
	@echo "Starting client (requires root)..."
	sudo ./$(CLIENT_BIN)

# Help
help:
	@echo "TCP Lite - Simple User Space TCP Implementation"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build server and client (default)"
	@echo "  clean        - Remove all build artifacts"
	@echo "  run-server   - Build and run server (requires root)"
	@echo "  run-client   - Build and run client (requires root)"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make"
	@echo "  sudo ./tcp_server [port]"
	@echo "  sudo ./tcp_client [server_ip] [port]"

.PHONY: all clean run-server run-client help

