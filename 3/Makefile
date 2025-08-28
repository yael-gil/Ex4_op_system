# Define compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

# Define source files explicitly
SRCS = Graph.cpp main.cpp

# Define all object files explicitly
OBJS = Graph.o main.o

# Define the final executable name
TARGET = graph_project

# Default target: builds the executable
all: $(TARGET)


# Rule to link the object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# Rule to compile graph.cpp to graph.o
Graph.o: Graph.cpp Graph.hpp
	$(CXX) $(CXXFLAGS) -c Graph.cpp

# Rule to compile main.cpp to main.o
main.o: main.cpp Graph.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

# Clean target: removes all generated files
clean:
	rm -f $(OBJS) $(TARGET)