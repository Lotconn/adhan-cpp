# ###################################################
#  [adhan-cli] Subdirectory level CMakeLists [START]
# ###################################################

project("adhan-cli" VERSION 1.0.0 LANGUAGES CXX)

# ###################################
#  SOURCES + INCLUDES + CXX STANDARD
# ###################################

add_executable(${PROJECT_NAME} src/main.cpp)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(${PROJECT_NAME} PUBLIC USING_SHARED_ADHAN_LIB)
endif()
include(${CMAKE_CURRENT_LIST_DIR}/../ExampleCommon.cmake)
configure_example(${example})

# #################################################
#  [adhan-cli] Subdirectory level CMakeLists [END]
# #################################################