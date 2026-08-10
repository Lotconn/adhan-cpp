cmake_minimum_required(VERSION @CMAKE_MINIMUM_VERSION@)

project(
  ConsumerCheck
  VERSION 1.0.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(
  @PACKAGE_NAME@
  REQUIRED
  PATHS "@INSTALL_PREFIX@"
  NO_DEFAULT_PATH
)

add_executable(
  consumer
  main.cpp
)

target_link_libraries(
  consumer
  PRIVATE
  @LINK_TARGET@
)

target_compile_features(
  consumer
  PRIVATE
  cxx_std_20
)
