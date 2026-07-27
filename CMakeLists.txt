cmake_minimum_required(VERSION 4.3.0)

project("adhan" VERSION 1.0.0)

# Source files

file(GLOB SOURCES "src/*.cpp")

add_library(
  ${PROJECT_NAME} STATIC ${SOURCES}
)

target_include_directories(
  ${PROJECT_NAME} PUBLIC "include/adhan" "include"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Compilation flags

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
  target_compile_options(
    ${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter
  )
endif()

# Options

option(
  ADHAN_USE_CTIME_FALLBACK
  "Use localtime_r/mktime fallback instead of <chrono> tzdb"
  OFF
)

if(${ADHAN_USE_CTIME_FALLBACK})
  target_compile_definitions(adhan PUBLIC ADHAN_USE_CTIME_FALLBACK)
endif()

add_subdirectory(tests)

add_test(NAME run-tests COMMAND run-tests)
