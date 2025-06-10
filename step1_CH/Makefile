CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -I../common/include

COMMON_SRC = \
    ../common/src/Graph.cpp \
    ../common/src/Utils.cpp

# יעד ברירת מחדל (כמו קודם)
all: Main

# קומפילציה והרצה של main.cpp
Main: main.cpp $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o main_exec main.cpp $(COMMON_SRC)
	./main_exec

# קומפילציה והרצה של testStep1.cpp
test: testStep1.cpp $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) -o test_exec testStep1.cpp $(COMMON_SRC)
	./test_exec

clean:
	rm -f main_exec test_exec
