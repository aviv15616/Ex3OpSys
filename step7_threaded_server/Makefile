CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
TARGET = server

SRC = server.cpp ../common/src/Graph.cpp 
HEADERS = ../common/include/Graph.hpp ../common/include/Point.hpp ../common/include/CH.hpp


all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
