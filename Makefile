# Makefile for TX3 Mini VFD Controller
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
TARGET = vfd_controller
SOURCE = vfd_controller.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all install clean
