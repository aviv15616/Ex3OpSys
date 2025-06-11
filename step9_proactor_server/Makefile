CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

# נתיבי קבצים
INCLUDES = -I../common/include
SRC = server.cpp \
      ../common/src/Graph.cpp \

TARGET = step9_server

all: $(TARGET)	
	./$(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
