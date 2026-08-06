# #######################################
#  [adhan] Root level CMakeLists [START]
# #######################################

cmake_minimum_required(VERSION 3.20)

project("adhan" VERSION 0.1.3)

# #######################
#  CMake Module Includes
# #######################

include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ###################################
#  SOURCES + INCLUDES + CXX STANDARD
# ###################################

file(GLOB SOURCES "src/*.cpp")

option(BUILD_SHARED_LIBS
  "Build shared library for adhan"
  ON
)

if(BUILD_SHARED_LIBS)
  # Make a shared library (default)
  add_library(${PROJECT_NAME} SHARED ${SOURCES})

  # Version naming with symlinks
  # e.g. (libadhan.so -> libadhan.so.1 -> libadhan.so.1.0.0)
  set_target_properties(${PROJECT_NAME} PROPERTIES

    # 1.0.0  --> libadhan.so.1.0.0
    VERSION ${PROJECT_VERSION}

    # 1      --> libadhan.so.1 
    SOVERSION ${PROJECT_VERSION_MAJOR}

    # appends "-<SOVERSION>" to the DLL name on Windows
    DLL_NAME_WITH_SOVERSION ON

    # Regardless of build-type, windows needs at
    # least the symbols exported in a `.lib` file
    WINDOWS_EXPORT_ALL_SYMBOLS ON
  )
else()

  # Make a static library
  add_library(${PROJECT_NAME} STATIC ${SOURCES})

endif()

# Namespacing so that its possible to link this library using `adhan::adhan`
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

# Handles version info header population
add_subdirectory(configured)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ####################
#  BUILD TYPE DEFAULT
# ####################

set(default_build_type "Debug")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")

  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
    STRING "Choose the type of build." FORCE
  )

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
  )

endif()

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

# ############
#  CLANG_TIDY
# ############

find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-18)
if(CLANG_TIDY_EXE)
  set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
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
  ADHAN_BUILD_TESTS
  "Build test binary and run test cases on build"
  ON
)
#
if(${ADHAN_BUILD_TESTS})
  add_subdirectory(tests)
endif()

# Build all the examples under the `examples` directory
# 
option(
  ADHAN_BUILD_EXAMPLES
  "Build examples (e.g. adhan-cli)"
  OFF
)
#
if(${ADHAN_BUILD_EXAMPLES})
  add_subdirectory(examples)
endif()

# ##############
#  INSTALLATION
# ##############

# The Library (`libadhan.so` or `libadhan.a`)
#
install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
)

# The Headers (all *.hpp files should be contained in an `adhan` directory)
# i.e. inside the prefix, the headers should be arranged like this:
# 
# include/
# └── adhan
#     └── *.hpp
#
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
# Install the target file under `<LIBDIR>/cmake/adhan/`
#
install(EXPORT "${PROJECT_NAME}Targets"
  FILE "${PROJECT_NAME}Targets.cmake"
  NAMESPACE "${PROJECT_NAME}::"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
# Make a new version configuration file
#
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${PROJECT_NAME}ConfigVersion.cmake
  COMPATIBILITY AnyNewerVersion
)
# Make a new configuration file
#
configure_file(
  ${PROJECT_NAME}Config.cmake.in
  ${PROJECT_NAME}Config.cmake
  @ONLY
)
# Install the configuration and version configuration files under `<LIBDIR>/cmake/adhan/`
#
install(
  FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# #####################################
#  [adhan] Root level CMakeLists [END]
# #####################################
