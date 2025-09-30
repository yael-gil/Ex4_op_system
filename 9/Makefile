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

# Headers (trigger rebuilds if changed)
HDRS := Graph.hpp Algorithms.hpp Factory.hpp MST.hpp SCC.hpp Hamilton.hpp MaxClique.hpp

# Runtime artifacts
RUN_ARTIFACTS := mysocket server.pid server.log client.log *.out *.tmp core core.* *.pid

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

# ==== Run helpers ====
.PHONY: run-server run-client server-bg stop-server show-logs

run-server: $(APP_SERVER)
	@echo "Starting server (foreground). Ctrl+C to stop."
	stdbuf -oL ./$(APP_SERVER) | tee server.log

run-client: $(APP_CLIENT)
	@echo "Example: ./client -a SCC -v 12 -e 24 -s 42"
	./$(APP_CLIENT) -a SCC -v 12 -e 24 -s 42

# Start server in background, wait until UDS socket is ready
server-bg: $(APP_SERVER)
	@rm -f server.log server.pid mysocket
	@echo "Starting server in background..."
	@stdbuf -oL ./$(APP_SERVER) > server.log 2>&1 & echo $$! > server.pid
	@printf "Waiting for server socket (mysocket)"
	@i=0; \
	while [ ! -S mysocket ] && [ $$i -lt 50 ]; do \
	  i=$$((i+1)); printf "."; sleep 0.1; \
	done; \
	if [ ! -S mysocket ]; then \
	  echo " not ready!"; \
	  echo "Server didn't create mysocket in time. Check server.log"; \
	  exit 1; \
	fi; \
	echo " ready."

# Stop background server (if running)
stop-server:
	@-if [ -f server.pid ]; then \
		echo "Stopping server PID $$(cat server.pid)"; \
		kill $$(cat server.pid) 2>/dev/null || true; \
		wait $$(cat server.pid) 2>/dev/null || true; \
		rm -f server.pid; \
	fi
	@rm -f mysocket

show-logs:
	@echo "===== tail -n 50 server.log ====="
	@-tail -n 50 server.log 2>/dev/null || true
	@echo "===== tail -n 50 client.log ====="
	@-tail -n 50 client.log 2>/dev/null || true

# ==== Tests for Stage 9 (Pipeline) ====
.PHONY: test9-parallel test9-serial test9-load

# Parallel test: run 8 clients concurrently (seeds 1..8)
# '|| true' so the target won't fail if a single client exits non-zero.
test9-parallel: $(APP_SERVER) $(APP_CLIENT) stop-server
	@echo "== Pipeline parallel test: 8 clients in parallel =="
	@rm -f client.log
	@$(MAKE) --no-print-directory server-bg
	@echo "Launching clients..."
	@seq 1 8 | xargs -I{} -P 8 ./$(APP_CLIENT) -a SCC -v 12 -e 24 -s {} >> client.log 2>&1 || true
	@$(MAKE) --no-print-directory stop-server
	@$(MAKE) --no-print-directory show-logs
	@echo "Done. Logs: server.log, client.log"

# ==== Cleaning ====
.PHONY: clean distclean
clean:
	$(RM) $(OBJS_SERVER) $(OBJS_CLIENT) $(RUN_ARTIFACTS)

distclean: clean
	$(RM) $(APP_SERVER) $(APP_CLIENT)
