CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

SRC = main.cpp
TARGET = reactor_test

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
