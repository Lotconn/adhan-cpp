# ###################################################
#  [run-tests] Subdirectory level CMakeLists [START]
# ###################################################

project("run-tests" VERSION 1.1.0 LANGUAGES CXX)

# ###################################
#  SOURCES + INCLUDES + CXX STANDARD
# ###################################

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_include_directories(
  ${PROJECT_NAME} PRIVATE "vendor" "include"
)

target_link_libraries(
  ${PROJECT_NAME} PRIVATE adhan::adhan
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)
# #################
#  COMPILE OPTIONS
# #################

if(MSVC)
  # `/wd5285` is to suppress a warning only emitted by `doctest.h`
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4100 /wd4101 /wd4505 /wd5285)
else()
  target_compile_options(
    ${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter
  )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
  ADHAN_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/generated/Times"
)

# ############
#  POST BUILD 
# ############

if(MSVC AND BUILD_SHARED_LIBS)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
  )
endif()


# ###################################
#  TEST DATA COPY + TEST INVOCATIONS
# ###################################

# # Copy the JSON test data to the test build directory
# file(COPY generated DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Invoke using `cmake --build . --target test`
add_custom_target(libtest
  COMMAND ${PROJECT_NAME}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${PROJECT_NAME}
  USES_TERMINAL
)

add_test(
  NAME libtest
  COMMAND ${PROJECT_NAME}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Invoke using `cmake --build . --target test-verbose`
# add_custom_target(test-verbose
#   COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} -s
#   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#   DEPENDS ${PROJECT_NAME}
#   USES_TERMINAL
# )

# #################################################
#  [run-tests] Subdirectory level CMakeLists [END]
# #################################################