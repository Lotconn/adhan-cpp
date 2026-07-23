# ----------------------------------------------------------------------
# Common build rules for examples
#
# Required variables:
#   TARGET
#
# Optional variables:
#   CXX
#   CXXFLAGS
#   LIBRARY
# ----------------------------------------------------------------------

CXX ?= g++

TARGET_DIR := bin

TARGET = $(TARGET_DIR)/$(notdir $(CURDIR))

LIBRARY ?= ../../libadhan.a

CXXFLAGS ?= -std=c++20 \
            -Wall -Wextra -Wpedantic \
            -I../../include \
            -Iinclude \
            -I../vendor

SRC_DIR := src
OBJ_DIR := obj

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

all: $(TARGET)

$(TARGET): $(OBJ) $(LIBRARY) | $(TARGET_DIR)
	$(CXX) $(OBJ) $(LIBRARY) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET_DIR)

.PHONY: all clean