# Config
CC		= g++
CFLAGS	= -std=c++20 -Wall -Wextra -Wpedantic -Wshadow -O3 -MMD -MP -fopenmp -Iinclude
BIN_DIR = bin
SRC_DIR	= src

# Core source files
TGAIMAGE = $(SRC_DIR)/tgaimage.cpp
MODEL	 = $(SRC_DIR)/model.cpp
MAIN	 = $(SRC_DIR)/main.cpp

# Sources
MAIN_SRC = $(TGAIMAGE) $(MODEL) $(MAIN)

# Binary files
MAIN_BIN = $(BIN_DIR)/renderer

# Targets
.PHONY: all build clean

all: build

# At the beginning, make sure that the bin directory exists
start:
	mkdir -p $(BIN_DIR)

# For building the binaries
$(MAIN_BIN): $(MAIN_SRC) | start
	$(CC) $(CFLAGS) $^ -o $@

build: $(MAIN_BIN)

# For cleaning up
clean:
	rm -rf $(BIN_DIR)
	rm -f *.tga