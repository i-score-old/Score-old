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
file(GLOB_RECURSE PROJECT_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h)

include_directories("${PROJECT_SOURCE_DIR}/includes")
include_directories("${PROJECT_SOURCE_DIR}/tests")
include_directories("${JAMOMA_MODULAR_INCLUDE_DIR}")
include_directories("${JAMOMA_MODULAR_PROTOCOLLIB_INCLUDE_DIR}")
include_directories("${JAMOMA_MODULAR_SCHEDULERLIB_INCLUDE_DIR}")
include_directories("${JAMOMA_MODULAR_PEEROBJECT_INCLUDE_DIR}")
include_directories("${JAMOMA_FOUNDATION_INCLUDE_DIR}")
include_directories("${JAMOMA_DSP_INCLUDE_DIR}")

include_directories(${LIBXML2_INCLUDE_DIR})
if(ANDROID)
include_directories("${CMAKE_FIND_ROOT_PATH}/include")
#include_directories("${CMAKE_FIND_ROOT_PATH}/include/libxml2")
#elseif(UNIX)
#include_directories("/usr/include/libxml2")
endif()

add_library(${PROJECT_NAME} SHARED ${PROJECT_SRCS} ${PROJECT_HDRS})

target_link_libraries(${PROJECT_NAME} ${JAMOMA_FOUNDATION_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${JAMOMA_DSP_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${JAMOMA_MODULAR_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES})

addAppleFramework(Carbon)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "lib")
INSTALL(FILES ${PROJECT_HDRS} DESTINATION include)
