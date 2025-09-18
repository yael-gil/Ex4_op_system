# Define compiler and flags
CXX = g++

# To run gcov, add  -fprofile-arcs -ftest-coverage to CXXFLAGS
# To run gprof, add -pg to CXXFLAGS
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

# A rule to compile and run gprof profiling 
gprof_report: $(TARGET)
	./$(TARGET) -v 499 -e 124251 -s 1
	gprof $(TARGET) gmon.out > gprof_report.txt

gprof_report_d: $(TARGET)
	./$(TARGET) -v 30 -e 870 -s 2 -d
	gprof $(TARGET) gmon.out > gprof_report.txt


# A rule to compile and run gcov coverage analysis
gcov_report: $(TARGET)
	./$(TARGET) -v 5 -e 4 -s 44
	gcov Graph.cpp main.cpp

gcov_report_d: $(TARGET)
	./$(TARGET) -v 500 -e 249500 -s 2 -d
	gcov Graph.cpp main.cpp

# A rule to run valgrind for memory leak detection
run_valgrind: $(TARGET)
	valgrind --leak-check=full  ./$(TARGET) -v 5 -e 3 -s 12

# A rule to run valgrind for memory leak detection on a directed graph
run_valgrind_d: $(TARGET)
	valgrind --leak-check=full  ./$(TARGET) -v 30 -e 870 -s 2 -d


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