# ################################################
#  [examples] ExampleCommon CMake Module [START]
# ################################################

#  Function to set options, includes, and library links for the example target
#
function(configure_example target)

  # #########################
  #  INCLUDES + CXX STANDARD
  # #########################

  target_include_directories(${target} PRIVATE
    ${CMAKE_SOURCE_DIR}/examples/vendor
    ${CMAKE_SOURCE_DIR}/include
  )
  target_compile_features(${target} PRIVATE cxx_std_20)


  # ###############################
  #  LIBRARY LINKS (e.g. libadhan)
  # ###############################

  target_link_libraries(${target} PRIVATE adhan::adhan)

  # For Windows, the shared libraries are copied to the example build directory
  # This is not good practice, as ideally one should "install" the library
  # But for the needs of our experiment, this is good enough
  # Not MSVC, as WIN32 will also include mingw, which also needs the .dll copy
  # 
  if(WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND}
      -E $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:${target}>>,copy_if_different,true>
      $<TARGET_RUNTIME_DLLS:${target}>
      $<TARGET_FILE_DIR:${target}>
      COMMAND_EXPAND_LISTS
    )
  endif()

  # #################
  #  COMPILE OPTIONS
  # #################

  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /wd4100 /wd4101 /wd4505)
  else()
    target_compile_options(
      ${target} PRIVATE
      -Wall -Wextra -Wpedantic
      -Wno-unused-variable
      -Wno-unused-function
      -Wno-unused-parameter
    )
  endif()
endfunction()
