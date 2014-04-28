cmake_minimum_required(VERSION 2.6)
project (Score)

message(STATUS "\nBuilding OSSIA Score project.\n")

if(NOT DEFINED BUILD_SCORE_FROM_JAMOMACORE)
### Configuration ###
include(${PROJECT_SOURCE_DIR}/cmake/JamomaUtilFunctions.txt NO_POLICY_SCOPE)
include(${PROJECT_SOURCE_DIR}/cmake/JamomaConfiguration.txt NO_POLICY_SCOPE)

### Platform-specific configuration ###
include(${PROJECT_SOURCE_DIR}/cmake/JamomaPlatformSpecific.txt NO_POLICY_SCOPE)

### Testing ###
include(${PROJECT_SOURCE_DIR}/cmake/JamomaTests.txt NO_POLICY_SCOPE)
endif()


# Find JamomaCore paths and libraries, can become FindJamomaCore.cmake once paths are settled
# Hints (paths) allow to find Jamoma libraries that are not yet installed but located on the same root dir as this source

include(FindPackageHandleStandardArgs)
find_path(JAMOMA_FOUNDATION_INCLUDE_DIR TTBase.h
		  HINTS ${CMAKE_SOURCE_DIR}/../../../Foundation/library/includes
				${CMAKE_SOURCE_DIR}/Foundation/library/includes
		  PATH_SUFFIXES jamoma)


find_package_handle_standard_args(JAMOMA_FOUNDATION DEFAULT_MSG
								  JAMOMA_FOUNDATION_LIBRARY
								  JAMOMA_FOUNDATION_INCLUDE_DIR)

include_directories(${JAMOMA_FOUNDATION_INCLUDE_DIR})

find_path(JAMOMA_DSP_INCLUDE_DIR TTDSP.h
		  HINTS ${CMAKE_SOURCE_DIR}/../../../DSP/library/includes
				${CMAKE_SOURCE_DIR}/DSP/library/includes
		  PATH_SUFFIXES jamoma)


find_package_handle_standard_args(JAMOMA_DSP DEFAULT_MSG JAMOMA_DSP_LIBRARY JAMOMA_DSP_INCLUDE_DIR)
include_directories(${JAMOMA_DSP_INCLUDE_DIR})

find_path(JAMOMA_MODULAR_INCLUDE_DIR TTModular.h
		  HINTS ${CMAKE_SOURCE_DIR}/../../../Modular/library/includes
				${CMAKE_SOURCE_DIR}/Modular/library/includes
		  PATH_SUFFIXES jamoma)
find_path(JAMOMA_MODULAR_PROTOCOLLIB_INCLUDE_DIR Protocol.h
		  HINTS ${CMAKE_SOURCE_DIR}/../../../Modular/library/ProtocolLib
				${CMAKE_SOURCE_DIR}/Modular/library/ProtocolLib
		  PATH_SUFFIXES jamoma)
find_path(JAMOMA_MODULAR_SCHEDULERLIB_INCLUDE_DIR Scheduler.h
		  HINTS ${CMAKE_SOURCE_DIR}/../../../Modular/library/SchedulerLib
				${CMAKE_SOURCE_DIR}/Modular/library/SchedulerLib
		  PATH_SUFFIXES jamoma)
find_path(JAMOMA_MODULAR_PEEROBJECT_INCLUDE_DIR TTMapper.h
		  HINTS ${CMAKE_SOURCE_DIR}/../../../Modular/library/PeerObject
				${CMAKE_SOURCE_DIR}/Modular/library/PeerObject
		  PATH_SUFFIXES jamoma)

find_package_handle_standard_args(JAMOMA_MODULAR DEFAULT_MSG JAMOMA_MODULAR_LIBRARY JAMOMA_MODULAR_INCLUDE_DIR)
include_directories(${JAMOMA_MODULAR_INCLUDE_DIR})

if(DEFINED BUILD_SCORE_FROM_JAMOMACORE)
	set(JAMOMA_FOUNDATION_LIBRARY JamomaFoundation)
	set(JAMOMA_DSP_LIBRARY JamomaDSP)
	set(JAMOMA_MODULAR_LIBRARY JamomaModular)
else()
	find_library(JAMOMA_FOUNDATION_LIBRARY NAMES JamomaFoundation)
	find_library(JAMOMA_DSP_LIBRARY NAMES JamomaDSP)
	find_library(JAMOMA_MODULAR_LIBRARY NAMES JamomaModular)
endif()


### Here are the Score Submodules ###
add_subdirectory(library)
add_subdirectory(extensions)

### Creation of packages ###
if(NOT DEFINED BUILD_SCORE_FROM_JAMOMACORE)
	SET(CPACK_PACKAGE_NAME "JamomaScore")
	SET(CPACK_PACKAGE_VENDOR "The Jamoma Foundation")

	SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Jamoma is a community-based project, dedicated to the development of several toolsets and frameworks for artistic creation through digital means. ")
	SET(CPACK_PACKAGE_VERSION "0.6-dev")
	SET(CPACK_PACKAGE_VERSION_MAJOR "0")
	SET(CPACK_PACKAGE_VERSION_MINOR "6")

	SET(CPACK_SOURCE_GENERATOR TGZ)
	SET(CPACK_SOURCE_PACKAGE_FILE_NAME "Jamoma-Score-0.6-dev")
	INCLUDE(CPack)
endif()
