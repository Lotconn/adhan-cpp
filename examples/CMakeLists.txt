# ##################################################
#  [examples] Subdirectory level CMakeLists [START]
# ##################################################

# ###################
#  EXAMPLES TO BUILD
# ###################

set(EXAMPLES_TO_BUILD
  adhan-cli
  jsonlib-example
)

# #####################################
#  SET OPTIONS FOR EACH EXAMPLE TARGET
# #####################################

foreach(example IN LISTS EXAMPLES_TO_BUILD)

  # ###################################
  #  SOURCES + INCLUDES + CXX STANDARD
  # ###################################

  add_subdirectory(${example})
  target_include_directories(${example} PRIVATE vendor ${CMAKE_SOURCE_DIR}/include)
  target_compile_features(${example} PRIVATE cxx_std_20)


  # ###############################
  #  LIBRARY LINKS (e.g. libadhan)
  # ###############################

  target_link_libraries(${example} PRIVATE adhan)

  # #################
  #  COMPILE OPTIONS
  # #################

  if(MSVC)
    target_compile_options(${example} PRIVATE /W4 /wd4100 /wd4101 /wd4505)
  else()
    target_compile_options(
      ${example} PRIVATE
      -Wall -Wextra -Wpedantic -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter
    )
  endif()

endforeach()

# ################################################
#  [examples] Subdirectory level CMakeLists [END]
# ################################################