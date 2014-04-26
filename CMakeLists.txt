cmake_minimum_required(VERSION 2.6)

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -Wno-unknown-pragmas -Wno-conversion")

# macro from the JamomaCore feature/cmake github branch
function(setExtensionSuffix)
	if(APPLE)
        SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PREFIX "")
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SUFFIX ".ttdylib")
	elseif(UNIX)
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SUFFIX ".ttso")
	elseif(WIN32)
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SUFFIX ".ttdll")
	endif(APPLE)
endFunction(setExtensionSuffix)

# macro from the JamomaCore feature/cmake github branch
function(addAppleFramework FRAMEWORK_NAME)
IF(APPLE)
   INCLUDE_DIRECTORIES ( /Developer/Headers/FlatCarbon )
   FIND_LIBRARY(THE_LIBRARY ${FRAMEWORK_NAME})
   MARK_AS_ADVANCED (THE_LIBRARY)
   SET(EXTRA_LIBS ${EXTRA_LIBS} ${THE_LIBRARY})
ENDIF (APPLE)
endFunction(addAppleFramework)

# macro from the JamomaCore feature/cmake github branch
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        SET(dirlist ${dirlist} ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

find_package(LibXml2 REQUIRED)
if(NOT LIBXML2_FOUND)
  message(FATAL_ERROR "Package libxml2 required, but not found!")
endif()

# Find JamomaCore paths and libraries, can become FindJamomaCore.cmake once paths are settled
# Hints (paths) allow to find Jamoma libraries that are not yet installed but located on the same root dir as this source
include(FindPackageHandleStandardArgs)
find_path(JAMOMA_FOUNDATION_INCLUDE_DIR TTBase.h HINTS ${CMAKE_SOURCE_DIR}/../../../Foundation/library/includes PATH_SUFFIXES jamoma)
find_library(JAMOMA_FOUNDATION_LIBRARY NAMES JamomaFoundation)
find_package_handle_standard_args(JAMOMA_FOUNDATION DEFAULT_MSG JAMOMA_FOUNDATION_LIBRARY JAMOMA_FOUNDATION_INCLUDE_DIR)
include_directories(${JAMOMA_FOUNDATION_INCLUDE_DIR})
find_path(JAMOMA_DSP_INCLUDE_DIR TTDSP.h HINTS ${CMAKE_SOURCE_DIR}/../../../DSP/library/includes PATH_SUFFIXES jamoma)
find_library(JAMOMA_DSP_LIBRARY NAMES JamomaDSP)
find_package_handle_standard_args(JAMOMA_DSP DEFAULT_MSG JAMOMA_DSP_LIBRARY JAMOMA_DSP_INCLUDE_DIR)
include_directories(${JAMOMA_DSP_INCLUDE_DIR})
find_path(JAMOMA_MODULAR_INCLUDE_DIR TTModular.h HINTS ${CMAKE_SOURCE_DIR}/../../../Modular/library/includes PATH_SUFFIXES jamoma)
find_path(JAMOMA_MODULAR_PROTOCOLLIB_INCLUDE_DIR Protocol.h HINTS ${CMAKE_SOURCE_DIR}/../../../Modular/library/ProtocolLib PATH_SUFFIXES jamoma)
find_path(JAMOMA_MODULAR_SCHEDULERLIB_INCLUDE_DIR Scheduler.h HINTS ${CMAKE_SOURCE_DIR}/../../../Modular/library/SchedulerLib PATH_SUFFIXES jamoma)
find_path(JAMOMA_MODULAR_PEEROBJECT_INCLUDE_DIR TTMapper.h HINTS ${CMAKE_SOURCE_DIR}/../../../Modular/library/PeerObject PATH_SUFFIXES jamoma)
find_library(JAMOMA_MODULAR_LIBRARY NAMES JamomaModular)
find_package_handle_standard_args(JAMOMA_MODULAR DEFAULT_MSG JAMOMA_MODULAR_LIBRARY JAMOMA_MODULAR_INCLUDE_DIR)
include_directories(${JAMOMA_MODULAR_INCLUDE_DIR})

# Find Gecode paths and libraries, can become FindGecode.cmake
find_package(Gecode REQUIRED COMPONENTS kernel support int set driver flatzinc minimodel search)
if(${GECODE_VERSION} MATCHES "4.?.?")# not the best test
    message(FATAL_ERROR "Gecode 4.x is not supported")
endif()

if(APPLE)
        if(NOT CMAKE_INSTALL_PREFIX)
            set(CMAKE_INSTALL_PREFIX "/usr/local/")
        endif()
        set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_INSTALL_PREFIX}/lib")
        string(REGEX MATCH "jamoma" CMAKE_INSTALL_PREFIX_WITH_JAMOMA "${CMAKE_INSTALL_PREFIX}")
        if(NOT CMAKE_INSTALL_PREFIX_WITH_JAMOMA)
            set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/jamoma/")
        endif()
        message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
        add_definitions(-DTT_PLATFORM_MAC)

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
# Independent shared libraries
        set(CMAKE_MACOSX_RPATH 1)

# CPack
        SET(CPACK_GENERATOR "Bundle")
        SET(CPACK_BUNDLE_NAME "Jamoma")
        #SET(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/Shared/toolchains/icone.icns")
        #SET(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Shared/toolchains/jamoma-Info.plist")

elseif(UNIX)
        add_definitions(-DTT_PLATFORM_LINUX)
        SET(CPACK_GENERATOR "DEB")
        SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "The Jamoma Foundation")
        SET(CPACK_SET_DESTDIR true)
        SET(CPACK_INSTALL_PREFIX "/usr/local/lib/jamoma/")

        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
                include_directories("/usr/include/i386-linux-gnu/c++/4.8/")
        endif()
elseif(WIN32)
        add_definitions(-DTT_PLATFORM_WIN)
        SET(CPACK_GENERATOR "NSIS")
endif()


### Here are the Jamoma Score Submodules ###
add_subdirectory(library)
add_subdirectory(extensions)

### Creation of packages ###
SET(CPACK_PACKAGE_NAME "Jamoma Score Libraries")
SET(CPACK_PACKAGE_VENDOR "The Jamoma Foundation")

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Jamoma is a community-based project, dedicated to the development of several toolsets and frameworks for artistic creation through digital means. ")
SET(CPACK_PACKAGE_VERSION "0.6-dev")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "6")

SET(CPACK_SOURCE_GENERATOR TGZ)
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "Jamoma-Score-0.6-dev")
INCLUDE(CPack)



