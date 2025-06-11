CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I../common/include -I../step5_reactor_template

SRC = server.cpp \
      ../common/src/Graph.cpp \


TARGET = reactor_server

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
