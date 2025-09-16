# Define compiler and flags
CXX = g++

# -fprofile-arcs -ftest-coverage for gcov
# -pg for gprof
CXXFLAGS = -std=c++17 -Wall -g  -fprofile-arcs -ftest-coverage

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

# A rule to compile and run gprof profiling 
gprof_report: gprof_exec
	./$(TARGET) -v 499 -e 124251 -s 1
	gprof $(TARGET) gmon.out > gprof_report.txt

gprof_report_d: gprof_exec
	./$(TARGET) -v 30 -e 870 -s 2 -d
	gprof $(TARGET) gmon.out > gprof_report.txt

gprof_exec: $(OBJS)
	$(CXX) -pg $(CXXFLAGS) $(OBJS) -o $(TARGET)

# A rule to compile and run gcov coverage analysis
gcov_report: gcov_exec
	./$(TARGET) -v 5 -e 4 -s 44
	gcov Graph.cpp main.cpp

gcov_report_d: gcov_exec
	./$(TARGET) -v 500 -e 249500 -s 2 -d
	gcov Graph.cpp main.cpp

gcov_exec: $(OBJS)
	$(CXX) $(CXXFLAGS) -fprofile-arcs -ftest-coverage $(OBJS) -o $(TARGET)

# A rule to run valgrind for memory leak detection
run_valgrind: $(TARGET)
	valgrind --leak-check=full  ./$(TARGET) -v 5 -e 4 -s 44

# A rule to run valgrind for memory leak detection on a directed graph
run_valgrind_d: $(TARGET)
	valgrind --leak-check=full  ./$(TARGET) -v 5 -e 4 -s 44 -d


run_valgrind_callgrind: $(TARGET)
	valgrind --tool=callgrind ./$(TARGET) -v 499 -e 124251 -s 1
	callgrind_annotate callgrind.out.* > callgrind_report.txt 

run_valgrind_callgrind_d: $(TARGET)
	valgrind --tool=callgrind ./$(TARGET) -v 499 -e 124251 -s 1 -d
	callgrind_annotate callgrind.out.* > callgrind_report.txt 

# Clean target: removes all generated files
clean:
	rm -f $(OBJS) $(TARGET)
	rm -f *.gcno *.gcda *.gcov *gmon.out callgrind.out.* 