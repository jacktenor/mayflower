# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/mayflower_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/mayflower_autogen.dir/ParseCache.txt"
  "mayflower_autogen"
  )
endif()
