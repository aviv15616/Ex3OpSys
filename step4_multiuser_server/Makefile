CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -I../common/include

SRC = server.cpp \
      ../common/src/Graph.cpp \
   

TARGET = server

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
