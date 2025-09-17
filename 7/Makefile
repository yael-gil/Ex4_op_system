CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -g
INCLUDES = -I.

# Source files
SERVER_SOURCES = server.cpp Graph.cpp
CLIENT_SOURCES = client.cpp

# Header files (dependencies)
HEADERS = Graph.hpp Algorithms.hpp Factory.hpp MST.hpp SCC.hpp Hamilton.hpp MaxClique.hpp
# Executable names
SERVER = server
CLIENT = client

# Default target
all: $(SERVER) $(CLIENT)

# Server target
$(SERVER): $(SERVER_SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(SERVER) $(SERVER_SOURCES)

# Client target
$(CLIENT): $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(CLIENT) $(CLIENT_SOURCES)

# Test targets for different algorithms
test-eulerian: $(SERVER) $(CLIENT)
	@echo "Testing Eulerian Algorithm with random graph..."
	./$(CLIENT) -a EULERIAN -v 4 -e 6 -s 42

test-scc: $(SERVER) $(CLIENT)
	@echo "Testing SCC Algorithm with random graph..."
	./$(CLIENT) -a SCC -v 5 -e 8 -s 123

test-mst: $(SERVER) $(CLIENT)
	@echo "Testing MST Algorithm with random graph..."
	./$(CLIENT) -a MST -v 6 -e 10 -s 456

test-maxclique: $(SERVER) $(CLIENT)
	@echo "Testing Max Clique Algorithm with random graph..."
	./$(CLIENT) -a MAXCLIQUE -v 5 -e 7 -s 789

test-hamilton: $(SERVER) $(CLIENT)
	@echo "Testing Hamilton Circuit Algorithm with random graph..."
	./$(CLIENT) -a HAMILTON -v 4 -e 6 -s 101

# Test with matrix input
test-matrix: $(SERVER) $(CLIENT)
	@echo "Testing with adjacency matrix..."
	@echo "0 1 1 0\n1 0 1 1\n1 1 0 1\n0 1 1 0" | ./$(CLIENT) -a EULERIAN -i

# Clean target
clean:
	rm -f $(SERVER) $(CLIENT) mysocket
# Run server in foreground
run-server: $(SERVER)
	@echo "Running server in foreground..."
	./$(SERVER)


# Stop server
stop-server:
	@echo "Stopping server..."
	@pkill -f "./$(SERVER)" || echo "Server not running"
	@rm -f mysocket

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build both server and client"
	@echo "  server        - Build server only"
	@echo "  client        - Build client only"
	@echo "  test-*        - Test specific algorithms"
	@echo "  test-matrix   - Test with matrix input"
	@echo "  run-server    - Start server in background"
	@echo "  stop-server   - Stop server"
	@echo "  clean         - Remove executables and socket file"
	@echo "  help          - Show this help"

.PHONY: all clean test-eulerian test-scc test-mst test-maxclique test-hamilton test-matrix run-server stop-server help