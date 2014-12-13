cmake_minimum_required(VERSION 2.6)
project(Scenario)

find_package(Gecode COMPONENTS kernel support int set driver flatzinc minimodel search)
if(NOT GECODE_FOUND)
	message(STATUS "Package Gecode required, but not found! Won't build extension Scenario.")
	return()
endif()

find_package(LibXml2)
if(NOT LIBXML2_FOUND)
  message(STATUS "Package libxml2 required, but not found! Won't build extension Scenario.")
  return()
endif()

set(PROJECT_SRCS

${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/customSpace.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/integerVariable.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/linearConstraint.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/searchEngine.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/solver_wrap.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/solver.cpp


${CMAKE_CURRENT_SOURCE_DIR}/source/Scenario.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/ScenarioSolver.cpp

)
file(GLOB_RECURSE PROJECT_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/Gecode/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/PetriNet/*.hpp                # is it needed ?
)

if(WIN32)
	set(PROJECT_SRCS ${PROJECT_SRCS}
		${CMAKE_CURRENT_SOURCE_DIR}/../TimePluginLib.cpp
		)
endif()

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
include_directories("${JAMOMA_FOUNDATION_INCLUDE_DIR}/../libiconv/include")
include_directories("${JAMOMA_DSP_INCLUDE_DIR}")


# TODO make FindPthreadWin32.cmake
if(WIN32)
	SET(PTHREAD_WIN32_PATH "C:\\Program Files (x86)\\pthread-win32" )
	FILE(TO_CMAKE_PATH ${PTHREAD_WIN32_PATH} PTHREAD_WIN32_PATH_CMAKE)
	include_directories("${PTHREAD_WIN32_PATH_CMAKE}/include")
endif()

include_directories(${LIBXML2_INCLUDE_DIR})
if(ANDROID)
include_directories("${CMAKE_FIND_ROOT_PATH}/include")
endif()

if(WIN32)
	add_definitions("/DNOMINMAX")
endif()

add_library(${PROJECT_NAME} SHARED ${PROJECT_SRCS} ${PROJECT_HDRS})

if(ANDROID)
target_link_libraries(${PROJECT_NAME} gecodesearch gecodedriver gecodeminimodel gecodeset gecodesupport gecodefloat gecodeint gecodeflatzinc gecodekernel)
else()
target_link_libraries(${PROJECT_NAME} ${GECODE_LIBRARIES}) # not that gecodefloat is missing in GECODE_LIBRARIES but not required to compile
endif()
target_link_libraries(${PROJECT_NAME} JamomaFoundation)
target_link_libraries(${PROJECT_NAME} JamomaDSP)
target_link_libraries(${PROJECT_NAME} JamomaModular)
target_link_libraries(${PROJECT_NAME} JamomaScore)
target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES})

# TODO make FindPthreadWin32.cmake
if(WIN32)
	target_link_libraries(${PROJECT_NAME} "${PTHREAD_WIN32_PATH_CMAKE}/lib/x86/pthreadVC2.lib")
endif()
### Output ###
setOutput()

### Tests ###
addTestTarget()
