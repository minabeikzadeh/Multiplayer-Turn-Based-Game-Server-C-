CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
TARGET = server

SRC = server.cpp
OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

%.o: %.cpp server.hpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
