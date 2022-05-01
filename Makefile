
CC = g++

CXXFLAGS= -g -Wall -std=c++20

SRC_DIR = src
TARGET = MultiWriteSingleRead

all: $(TARGET)

$(TARGET):
	$(CC) $(CXXFLAGS) -o $(TARGET) $(SRC_DIR)/$(TARGET).cpp

clean:
	$(RM) $(TARGET)
	$(RM) -r $(TARGET).dSYM
