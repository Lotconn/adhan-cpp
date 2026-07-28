# #######################################
#  [adhan] Root level CMakeLists [START]
# #######################################

cmake_minimum_required(VERSION 4.3.0)

project("adhan" VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ###################################
#  SOURCES + INCLUDES + CXX STANDARD
# ###################################

file(GLOB SOURCES "src/*.cpp")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
  ${PROJECT_NAME} SHARED ${SOURCES}
)

target_include_directories(
  ${PROJECT_NAME} PUBLIC "include/adhan" "include"
)



# #################
#  COMPILE OPTIONS
# #################

# For windows, `/W4` is the second-highest strictness level
# Only second to "treating all warnings as errors"
# 
# /wdnnnn 	Suppresses the compiler warning that is specified by nnnn.
# 
# For example, /wd4326 suppresses compiler warning C4326.
# 
# We suppress all unused variables + functions + params
# (compiler-warnings)      4100        4101       4505
# (dismiss flag)        -wd4100     -wd4101    -wd4505
# 
# (Replace 4XXX in the link below with the compiler warning code above)
# Ref: https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4XXX?view=msvc-170
# 
# For others, we can use the usual flags (-Wall -Wextra ...)
# Ref: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
# 
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

# Fallback to use `<ctime>` functions instead of `<chrono>`'s `tzdb` for compatibility
# Needed for unconventional systems that do not have `tzdb` as part of the stdlib,
# e.g. for Termux on AndroidOS
# 
option(
  ADHAN_USE_CTIME_FALLBACK
  "Use localtime_r/mktime fallback instead of <chrono> tzdb"
  OFF
)
# 
if(${ADHAN_USE_CTIME_FALLBACK})
  target_compile_definitions(${PROJECT_NAME} PUBLIC ADHAN_USE_CTIME_FALLBACK)
endif()

# After we are done making the library, build the tests and run them
# 
option(
  BUILD_TESTS
  "Build test binary and run test cases on build"
  ON
)
#
if(${BUILD_TESTS})
  add_subdirectory(tests)
endif()

# Build all the examples under the `examples` directory
# 
option(
  BUILD_EXAMPLES
  "Build examples (e.g. adhan-cli)"
  OFF
)
#
if(${BUILD_EXAMPLES})
  add_subdirectory(examples)
endif()

# ##############
#  INSTALLATION
# ##############

# The Library (`libadhan.so` or `libadhan.a`)
#
# For windows, we need to add the `RUNTIME` install
# For the rest, just the archive + library should be enough
# 
if(MSVC)
  install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )
else()
  install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
  )
endif()

# The Headers (all *.hpp files should be contained in an `adhan` directory)
# i.e. inside the prefix, the headers should be arranged like this:
# 
# include/
# └── adhan
#     └── *.hpp
#
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
  DESTINATION include
)

# #####################################
#  [adhan] Root level CMakeLists [END]
# #####################################
