# ###################################################
#  [run-tests] Subdirectory level CMakeLists [START]
# ###################################################

project("run-tests" VERSION 1.0.0)

# ###################################
#  SOURCES + INCLUDES + CXX STANDARD
# ###################################

file(GLOB TEST_SOURCES ${SOURCES} "src/*.cpp" "*.cpp")

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_include_directories(
  ${PROJECT_NAME} PUBLIC "../include/adhan" "../include" "vendor" "include"
)

target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

# #################
#  COMPILE OPTIONS
# #################

target_compile_definitions(${PROJECT_NAME} PRIVATE ADHAN_TESTING)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4100 /wd4101 /wd4505)
else()
  target_compile_options(
    ${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter
  )
endif()

# ###############################
#  OPTIONS + COMPILE DEFINITIONS
# ###############################

# We need a separate block here, as we are NOT linking `libadhan` in the tests
# But rather, we are using the `adhan` sources directly to make the tests
if(${ADHAN_USE_CTIME_FALLBACK})
  target_compile_definitions(${PROJECT_NAME} PUBLIC ADHAN_USE_CTIME_FALLBACK)
endif()

# ###################################
#  TEST DATA COPY + TEST INVOCATIONS
# ###################################

# Copy the JSON test data to the test build directory
file(COPY Shared DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Invoke using `cmake --build . --target test`
add_custom_target(test
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} | tee ${CMAKE_SOURCE_DIR}/test.log
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS ${PROJECT_NAME}
  USES_TERMINAL
)

# Invoke using `cmake --build . --target test-verbose`
add_custom_target(test-verbose
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} -s | tee ${CMAKE_SOURCE_DIR}/test-verbose.log
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS ${PROJECT_NAME}
  USES_TERMINAL
)

# #################################################
#  [run-tests] Subdirectory level CMakeLists [END]
# #################################################