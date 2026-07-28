CXX := g++

LIBNAMESPACE := adhan

INCLUDE_FLAGS := -Iinclude/$(LIBNAMESPACE) \
								-Iinclude \

# ---------------------------- TZ FALLBACK -------------------------------
# `make TZFALLBACK=1 <target>` (or one of the *-tzfallback convenience
# targets below) compiles the library/tests with ADHAN_USE_CTIME_FALLBACK
# defined, using the <ctime>-based JSDate implementation instead of the
# <chrono> calendar/tz one. OBJ_DIR/TARGET/TEST_OBJ_DIR/TEST_TARGET are all
# renamed under this mode so the two variants never share or clobber each
# other's object files or artifacts.

# In case we want a different name for our fallback
ifdef TZFALLBACK
TZ_DEFINE    := -DADHAN_USE_CTIME_FALLBACK
OBJ_DIR      := obj
TARGET       := libadhan.a
TEST_OBJ_DIR := obj/tests
TEST_TARGET  := run_tests
else
TZ_DEFINE    :=
OBJ_DIR      := obj
TARGET       := libadhan.a
TEST_OBJ_DIR := obj/tests
TEST_TARGET  := run_tests
endif
# -------------------------------------------------------------------------

ifdef RELEASE
BUILD_FLAGS := -O2 -DNDEBUG
else
BUILD_FLAGS := -O0 -g
endif

CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic \
            -Wno-unused-variable \
            -Wno-unused-function \
            -Wno-unused-parameter \
            $(TZ_DEFINE) \
						$(BUILD_FLAGS) \
            $(INCLUDE_FLAGS)

SRC_DIR := src

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

all: $(TARGET)

everything: all test-build

tzfallback: all-tzfallback

all-tzfallback:
	$(MAKE) TZFALLBACK=1 all

test-tzfallback:
	$(MAKE) TZFALLBACK=1 test

everything-tzfallback:
	$(MAKE) TZFALLBACK=1 everything

test-build-tzfallback:
	$(MAKE) TZFALLBACK=1 test-build

$(TARGET): $(OBJ)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

# ------------------------------- TESTS ----------------------------------
# Test-only tree: never installed, so its never part of $(TARGET).
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

# TEST_OBJ_DIR is set at the top (tzfallback-aware) — not redefined here.

# Library sources, recompiled with ADHAN_TESTING defined
TEST_LIB_OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(TEST_OBJ_DIR)/lib_%.o,$(SRC))

# Test-only helper sources (e.g. MomentFormat.cpp)
TEST_UTIL_SRC := $(wildcard $(TEST_SRC_DIR)/*.cpp)
TEST_UTIL_OBJ := $(patsubst $(TEST_SRC_DIR)/%.cpp,$(TEST_OBJ_DIR)/util_%.o,$(TEST_UTIL_SRC))

# Actual test case files, e.g. tests/CalculationParameters_test.cpp
TEST_CASE_SRC := $(wildcard $(TEST_DIR)/*.cpp)
TEST_CASE_OBJ := $(patsubst $(TEST_DIR)/%.cpp,$(TEST_OBJ_DIR)/case_%.o,$(TEST_CASE_SRC))

# TEST_TARGET is set at the top (tzfallback-aware) — not redefined here.

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
	rm -rf build \
	  $(OBJ_DIR) \
	  $(TARGET) \
	  $(TEST_TARGET)

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
	@echo "  --- <ctime>-fallback variants (ADHAN_USE_CTIME_FALLBACK defined) ---"
	@echo "  tzfallback             Same as all-tzfallback"
	@echo "  all-tzfallback         Build the static library, but use the ctime fallback($(TARGET))"
	@echo "  test-tzfallback        Build and run all tests"
	@echo "  test-build-tzfallback  Only build the tests without running them"
	@echo "  everything-tzfallback  Runs all-tzfallback and test-build-tzfallback"
	@echo ""
	@echo "  --- Release builds ---"
	@echo "  make RELEASE=1 <target>   Build with -O2 -DNDEBUG instead of the"
	@echo "                            default -O0 -g (asserts are disabled)"
	@echo "                            e.g. make RELEASE=1 all"
	@echo "                                 make RELEASE=1 test"
	@echo "                            Combinable with TZFALLBACK=1, e.g.:"
	@echo "                                 make RELEASE=1 TZFALLBACK=1 all"
	@echo ""
	@echo "Test executable options:"
	@echo ""
	@echo "  ./$(TEST_TARGET) --help"
	@echo "  ./$(TEST_TARGET) -s	# Also show successful assertions"
	@echo "  ./$(TEST_TARGET) --list-test-cases"
	@echo "  ./$(TEST_TARGET) --list-test-suites"
	@echo "  ./$(TEST_TARGET) --list-reporters"
	@echo "  ./$(TEST_TARGET) --test-case=<pattern>"

.PHONY: everything all clean test test-verbose help \
        tzfallback all-tzfallback test-tzfallback everything-tzfallback test-build-tzfallback