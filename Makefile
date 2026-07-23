CXX := g++

CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic \
            -Wno-unused-variable \
            -Wno-unused-function \
            -Wno-unused-parameter \
            -Iinclude

SRC_DIR := src
OBJ_DIR := obj

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

TARGET := libadhan.a

all: $(TARGET)

everything: all test-build

$(TARGET): $(OBJ)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

# ---- Tests -----------------------------------------------------------
# Test-only tree: never installed, never part of $(TARGET).
# The library sources are recompiled here (separately from $(OBJ_DIR))
# with -DADHAN_TESTING defined, since the spy-seam call counter in
# PolarCircleResolution.cpp only exists under that macro. This keeps
# the release build/object files completely untouched by test concerns.

TEST_DIR        := tests
TEST_INCLUDE_DIR := $(TEST_DIR)/include
TEST_SRC_DIR    := $(TEST_DIR)/src
DOCTEST_DIR     := $(TEST_DIR)/vendor

TEST_CXXFLAGS := $(CXXFLAGS) -DADHAN_TESTING \
                  -I$(TEST_INCLUDE_DIR) -I$(DOCTEST_DIR) \
                  -Wno-unused-variable -Wno-unused-parameter

TEST_OBJ_DIR := obj/tests

# Library sources, recompiled with ADHAN_TESTING defined
TEST_LIB_OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(TEST_OBJ_DIR)/lib_%.o,$(SRC))

# Test-only helper sources (e.g. MomentFormat.cpp)
TEST_UTIL_SRC := $(wildcard $(TEST_SRC_DIR)/*.cpp)
TEST_UTIL_OBJ := $(patsubst $(TEST_SRC_DIR)/%.cpp,$(TEST_OBJ_DIR)/util_%.o,$(TEST_UTIL_SRC))

# Actual test case files, e.g. tests/CalculationParameters_test.cpp
TEST_CASE_SRC := $(wildcard $(TEST_DIR)/*.cpp)
TEST_CASE_OBJ := $(patsubst $(TEST_DIR)/%.cpp,$(TEST_OBJ_DIR)/case_%.o,$(TEST_CASE_SRC))

TEST_TARGET := run_tests

test-build: $(TEST_TARGET)

test-verbose: $(TEST_TARGET)
	./$(TEST_TARGET) -s | tee ./test-verbose.log

test: $(TEST_TARGET)
	./$(TEST_TARGET) | tee ./test.log

$(TEST_TARGET): $(TEST_LIB_OBJ) $(TEST_UTIL_OBJ) $(TEST_CASE_OBJ)
	$(CXX) $(TEST_CXXFLAGS) $^ -o $@

$(TEST_OBJ_DIR)/lib_%.o: $(SRC_DIR)/%.cpp | $(TEST_OBJ_DIR)
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/util_%.o: $(TEST_SRC_DIR)/%.cpp | $(TEST_OBJ_DIR)
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/case_%.o: $(TEST_DIR)/%.cpp | $(TEST_OBJ_DIR)
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

$(TEST_OBJ_DIR):
	mkdir -p $@
# ------------------------------------------------------------------------

clean:
	rm -rf build.log $(OBJ_DIR) $(TARGET) $(TEST_TARGET)

help:
	@echo "Available targets:"
	@echo ""
	@echo "  everything     Runs 'all' and 'test-build', useful when generating the 'compile_commands.json'"
	@echo "  all            Build the static library ($(TARGET))"
	@echo "  test           Build and run all tests"
	@echo "  test-verbose   Build and run tests, also printing successful assertions"
	@echo "  test-build     Only build the tests without running them"
	@echo "  clean          Remove build artifacts"
	@echo "  help           Show this help message"
	@echo ""
	@echo "Test executable options:"
	@echo ""
	@echo "  ./$(TEST_TARGET) --help"
	@echo "  ./$(TEST_TARGET) -s	# Also show successful assertions"
	@echo "  ./$(TEST_TARGET) --list-test-cases"
	@echo "  ./$(TEST_TARGET) --list-test-suites"
	@echo "  ./$(TEST_TARGET) --list-reporters"
	@echo "  ./$(TEST_TARGET) --test-case=<pattern>"

.PHONY: everything all clean test test-verbose help