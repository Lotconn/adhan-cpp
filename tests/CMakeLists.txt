# Source files

file(GLOB TEST_SOURCES ${SOURCES} "src/*.cpp" "*.cpp")

add_executable(run-tests ${TEST_SOURCES})

target_include_directories(
  "run-tests" PUBLIC "../include/adhan" "../include" "vendor" "include"
)

target_compile_features("run-tests" PRIVATE cxx_std_20)

# Compilation flags

target_compile_definitions("run-tests" PRIVATE ADHAN_TESTING)

if(MSVC)
  target_compile_options("run-tests" PRIVATE /W4)
else()
  target_compile_options(
    "run-tests" PRIVATE
    -Wall -Wextra -Wpedantic -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter
  )
endif()

# Copy the JSON test data to the test build directory
file(COPY Shared DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Invoke using `cmake --build . --target test`
add_custom_target(test
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run-tests | tee ${CMAKE_SOURCE_DIR}/test.log
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS run-tests
  USES_TERMINAL
)

# Invoke using `cmake --build . --target test-verbose`
add_custom_target(test-verbose
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run-tests -s | tee ${CMAKE_SOURCE_DIR}/test-verbose.log
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS run-tests
  USES_TERMINAL
)
