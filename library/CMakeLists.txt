cmake_minimum_required(VERSION 2.6)
project(JamomaScore)

add_definitions(-DTTSCORE_EXPORTS)

set(PROJECT_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/source/TTScoreAPI.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/TTScore.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/TTScoreSymbolCache.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/Expression.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/TTTimeCondition.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/TTTimeContainer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/TTTimeEvent.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/TTTimeProcess.cpp

${CMAKE_CURRENT_SOURCE_DIR}/tests/TTScore.test.cpp
)

include_directories("${PROJECT_SOURCE_DIR}/includes")
include_directories("${PROJECT_SOURCE_DIR}/tests")
include_directories("${PROJECT_SOURCE_DIR}/../../Modular/library/includes")
include_directories("${PROJECT_SOURCE_DIR}/../../Modular/library/ProtocolLib")
include_directories("${PROJECT_SOURCE_DIR}/../../Modular/library/SchedulerLib")
include_directories("${PROJECT_SOURCE_DIR}/../../Modular/library/PeerObject")
include_directories("${PROJECT_SOURCE_DIR}/../../Foundation/library/includes")
include_directories("${PROJECT_SOURCE_DIR}/../../DSP/library/includes")

if(ANDROID)
include_directories("${CMAKE_FIND_ROOT_PATH}/include")
include_directories("${CMAKE_FIND_ROOT_PATH}/include/libxml2")
elseif(UNIX)
include_directories("/usr/include/libxml2")
endif()

add_library(${PROJECT_NAME} SHARED ${PROJECT_SRCS})

target_link_libraries(${PROJECT_NAME} JamomaFoundation)
target_link_libraries(${PROJECT_NAME} JamomaDSP)
target_link_libraries(${PROJECT_NAME} JamomaModular)
target_link_libraries(${PROJECT_NAME} xml2)

addAppleFramework(Carbon)
