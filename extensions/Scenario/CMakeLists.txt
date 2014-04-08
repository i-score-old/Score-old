cmake_minimum_required(VERSION 2.6)
project(Scenario)

set(PROJECT_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/../TimePluginLib.cpp

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

include_directories("${PROJECT_SOURCE_DIR}/")
include_directories("${PROJECT_SOURCE_DIR}/..")
include_directories("${PROJECT_SOURCE_DIR}/includes")
include_directories("${PROJECT_SOURCE_DIR}/source/Gecode")
include_directories("${PROJECT_SOURCE_DIR}/source/PetriNet")
include_directories("${PROJECT_SOURCE_DIR}/../../library/includes")
include_directories("${PROJECT_SOURCE_DIR}/../../library/tests")
include_directories("${PROJECT_SOURCE_DIR}/../../../Modular/library/includes")
include_directories("${PROJECT_SOURCE_DIR}/../../../Modular/library/ProtocolLib")
include_directories("${PROJECT_SOURCE_DIR}/../../../Modular/library/SchedulerLib")
include_directories("${PROJECT_SOURCE_DIR}/../../../Modular/library/PeerObject")
include_directories("${PROJECT_SOURCE_DIR}/../../../Foundation/library/includes")
include_directories("${PROJECT_SOURCE_DIR}/../../../DSP/library/includes")

if(ANDROID)
include_directories("${CMAKE_FIND_ROOT_PATH}/include")
include_directories("${CMAKE_FIND_ROOT_PATH}/include/libxml2")
elseif(UNIX)
include_directories("/usr/include/libxml2")
endif()

add_library(${PROJECT_NAME} SHARED ${PROJECT_SRCS})

if(ANDROID OR APPLE)
target_link_libraries(${PROJECT_NAME} gecodesearch gecodedriver gecodeminimodel gecodeset gecodesupport gecodefloat gecodeint  gecodeflatzinc gecodekernel)
endif()
target_link_libraries(${PROJECT_NAME} JamomaFoundation)
target_link_libraries(${PROJECT_NAME} JamomaDSP)
target_link_libraries(${PROJECT_NAME} JamomaModular)
target_link_libraries(${PROJECT_NAME} JamomaScore)
target_link_libraries(${PROJECT_NAME} xml2)

#set(CMAKE_SHARED_LINKER_FLAGS "-F/Library/Frameworks/ -framework gecode")
setExtensionSuffix()

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "extensions")
