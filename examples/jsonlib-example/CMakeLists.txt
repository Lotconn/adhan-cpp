# #########################################################
#  [jsonlib-example] Subdirectory level CMakeLists [START]
# #########################################################

project("jsonlib-example" VERSION 1.0.0 LANGUAGES CXX)

# ###################################
#  SOURCES + INCLUDES + CXX STANDARD
# ###################################

add_executable(${PROJECT_NAME} src/main.cpp)
include(${CMAKE_CURRENT_LIST_DIR}/../ExampleCommon.cmake)
configure_example(${example})

# Copy the JSON dummy input data to the test build directory
file(COPY "dummy.json" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# #######################################################
#  [jsonlib-example] Subdirectory level CMakeLists [END]
# #######################################################