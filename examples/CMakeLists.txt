# ##################################################
#  [examples] Subdirectory level CMakeLists [START]
# ##################################################

# ###################
#  EXAMPLES TO BUILD
# ###################

set(EXAMPLES_TO_BUILD
  adhan-cli
  jsonlib-example
  mathutils-example
)

# #####################################
#  SET OPTIONS FOR EACH EXAMPLE TARGET
# #####################################

foreach(example IN LISTS EXAMPLES_TO_BUILD)

  # ###################################
  #  SOURCES + INCLUDES + CXX STANDARD
  # ###################################

  add_subdirectory(${example})

endforeach()

# ################################################
#  [examples] Subdirectory level CMakeLists [END]
# ################################################