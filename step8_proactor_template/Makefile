# Makefile to compile Main.cpp using Proactor.hpp

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
TARGET = main_proactor_demo
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
