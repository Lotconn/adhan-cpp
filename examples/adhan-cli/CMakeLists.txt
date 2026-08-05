# ###################################################
#  [adhan-cli] Subdirectory level CMakeLists [START]
# ###################################################

project("adhan-cli" VERSION 1.0.0 LANGUAGES CXX)

# ###################################
#  SOURCES + INCLUDES + CXX STANDARD
# ###################################

add_executable(${PROJECT_NAME} src/main.cpp)
include(${CMAKE_CURRENT_LIST_DIR}/../ExampleCommon.cmake)
configure_example(${example})

# #################################################
#  [adhan-cli] Subdirectory level CMakeLists [END]
# #################################################