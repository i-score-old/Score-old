cmake_minimum_required(VERSION 2.6)

set(CMAKE_INSTALL_PREFIX /usr/local) # won't work on windows
project (iscoreCommandLine)

include_directories("/usr/include/jamoma")

set(CMAKE_CXX_FLAGS "-std=c++11")


add_executable(i-score-nogui "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_link_libraries(i-score-nogui JamomaFoundation JamomaModular JamomaScore pthread)

INSTALL(TARGETS i-score-nogui
	RUNTIME DESTINATION bin)
