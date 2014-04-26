cmake_minimum_required(VERSION 2.6)
project(Scenario)

set(PROJECT_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/../TimePluginLib.cpp # doesn't exist!

${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/customSpace.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/integerVariable.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/linearConstraint.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/searchEngine.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/solver_wrap.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/solver.cpp

${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/Arc.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/ExtendedInt.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/PetriNet.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/PetriNetItem.cpp                    # is it needed ?
${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/PetriNetNode.cpp                    # is it needed ?
${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/Place.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/PriorityTransitionAction.cpp        # is it needed ?
${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/ThreadSafeList.cpp                  # is it needed ?
${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/Token.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/Transition.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/TransitionAction.cpp                # is it needed ?
${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/TransitionBitArray.cpp              # is it needed ?

${CMAKE_CURRENT_SOURCE_DIR}/source/Scenario.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/ScenarioSolver.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/ScenarioGraph.cpp
)
file(GLOB_RECURSE PROJECT_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/*.hpp                # is it needed ?
)

include_directories("${PROJECT_SOURCE_DIR}/")
include_directories("${PROJECT_SOURCE_DIR}/..")
include_directories("${PROJECT_SOURCE_DIR}/includes")
include_directories("${GECODE_INCLUDE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/source/Gecode")
include_directories("${PROJECT_SOURCE_DIR}/source/PetriNet")
include_directories("${PROJECT_SOURCE_DIR}/../../library/includes")
include_directories("${PROJECT_SOURCE_DIR}/../../library/tests")
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

target_link_libraries(${PROJECT_NAME} ${GECODE_LIBRARIES}) # not that gecodefloat is missing in GECODE_LIBRARIES but not required to compile
target_link_libraries(${PROJECT_NAME} ${JAMOMA_FOUNDATION_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${JAMOMA_DSP_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${JAMOMA_MODULAR_LIBRARY})
target_link_libraries(${PROJECT_NAME} JamomaScore)
target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES})

#set(CMAKE_SHARED_LINKER_FLAGS "-F/Library/Frameworks/ -framework gecode")
setExtensionSuffix()

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "extensions")
