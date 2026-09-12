# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/TrueSightClient_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/TrueSightClient_autogen.dir/ParseCache.txt"
  "TrueSightClient_autogen"
  )
endif()
