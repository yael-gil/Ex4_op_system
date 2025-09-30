# ==== Config ====
CXX       := g++
CXXFLAGS  := -Wall -Wextra -std=c++17 -O2 -g -I. -pthread
LDFLAGS   := -pthread

APP_SERVER := server
APP_CLIENT := client

# Sources / Objects
SRCS_SERVER := server.cpp Graph.cpp
SRCS_CLIENT := client.cpp
OBJS_SERVER := $(SRCS_SERVER:.cpp=.o)
OBJS_CLIENT := $(SRCS_CLIENT:.cpp=.o)

# Optional headers (to trigger rebuilds when headers change)
HDRS := Graph.hpp Algorithms.hpp Factory.hpp MST.hpp SCC.hpp Hamilton.hpp MaxClique.hpp

# ==== Default ====
.PHONY: all
all: $(APP_SERVER) $(APP_CLIENT)

# ==== Binaries ====
$(APP_SERVER): $(OBJS_SERVER)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(APP_CLIENT): $(OBJS_CLIENT)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# ==== Pattern rule ====
%.o: %.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ==== Convenience targets ====
.PHONY: run-server run-client test-scc clean distclean

run-server: $(APP_SERVER)
	./$(APP_SERVER)

run-client: $(APP_CLIENT)
	@echo "Example: ./client -a SCC -v 5 -e 8 -s 123"
	./$(APP_CLIENT) -a SCC -v 5 -e 8 -s 123

test-scc: $(APP_SERVER) $(APP_CLIENT)
	@echo "Start the server in one terminal: ./server"
	@echo "Then in another terminal run:"
	@echo "./client -a SCC -v 5 -e 8 -s 123"

# ==== Cleaning ====
clean:
	$(RM) $(OBJS_SERVER) $(OBJS_CLIENT)

distclean: clean
	$(RM) $(APP_SERVER) $(APP_CLIENT) mysocket
