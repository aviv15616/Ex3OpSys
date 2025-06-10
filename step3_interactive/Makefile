CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -I../common/include

SRC = main.cpp \
      ../common/src/Graph.cpp \
      ../common/src/Utils.cpp

TARGET = step3_CH

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
