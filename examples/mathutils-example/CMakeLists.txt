# ###########################################################
#  [mathutils-example] Subdirectory level CMakeLists [START]
# ###########################################################

project("mathutils-example" VERSION 1.0.0 LANGUAGES CXX)

# ###################################
#  SOURCES + INCLUDES + CXX STANDARD
# ###################################

add_executable(${PROJECT_NAME} src/main.cpp)
include(${CMAKE_CURRENT_LIST_DIR}/../ExampleCommon.cmake)
configure_example(${example})

# #########################################################
#  [mathutils-example] Subdirectory level CMakeLists [END]
# #########################################################