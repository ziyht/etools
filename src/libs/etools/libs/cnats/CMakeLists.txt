# include this file to import cnats to your project, like this:
#
#   set(CNATS_DIR      ${PROJECT_SOURCE_DIR}/src/cnats)     # set CNATS_DIR, must
#   set(CNATS_BUILT_IN ON/OFF)                              # default is ON
#   include(${CNATS_DIR}/CMakeLists.txt)                    # include me
#

#---------------------------------------------------------------------------
# head added
#---------------------------------------------------------------------------
if(NOT DEFINED CNATS_BUILT_IN)
    option(CNATS_BUILT_IN "build cnats in or use cnats as static lib" ON)
endif()
set(DUP_CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
set(DUP_CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
#---------------------------------------------------------------------------
# setups of cnats in cnats src
#---------------------------------------------------------------------------
set(NATS_INSTALL_PREFIX ../install CACHE PATH "Install prefix")
set(CMAKE_INSTALL_PREFIX ${NATS_INSTALL_PREFIX} CACHE INTERNAL "")

option(NATS_UPDATE_VERSION "Update the version file" OFF)
option(NATS_COVERAGE "Code coverage" OFF)
if(NOT DEFINED NATS_BUILD_WITH_TLS)
option(NATS_BUILD_WITH_TLS "Build with TLS support" ON)
endif()
option(NATS_BUILD_WITH_TLS_CLIENT_METHOD "Use TLS_client_method()" OFF)
option(NATS_BUILD_LIBUV_EXAMPLE "Compile libuv example" OFF)
option(NATS_BUILD_LIBEVENT_EXAMPLE "Compile libevent example" OFF)

if(NATS_BUILD_WITH_TLS)
find_package(OpenSSL REQUIRED)
endif(NATS_BUILD_WITH_TLS)

set(LIBUV_DIR "/usr/local/" CACHE PATH "Libuv install directory")
set(LIBEVENT_DIR "/usr/local/" CACHE PATH "Libevent install directory")

MESSAGE(STATUS "cnats: Current Platform is ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
# Platform specific settings
if(UNIX)
  #---------------------------------------------------------------------------
  # Define NATS cache variables that override the CMAKE and MEMCHECK variables
  #---------------------------------------------------------------------------
  set(NATS_BUILD_TYPE Debug CACHE STRING "Build type: Release, Debug, RelWithDebInfo, MinRelSize")
  set(CMAKE_BUILD_TYPE ${NATS_BUILD_TYPE} CACHE INTERNAL "")

  set(NATS_BUILD_ARCH "64" CACHE STRING "32 for 32bits builds")

  set(NATS_MEMCHECK_CMD "/usr/bin/valgrind" CACHE FILE "Memcheck tool")
  set(MEMORYCHECK_COMMAND ${NATS_MEMCHECK_CMD} CACHE INTERNAL "")

  set(NATS_MEMCHECK_CMD_OPTS "--leak-check=full --track-fds=yes --show-reachable=yes --num-callers=50" CACHE STRING "Memcheck options")
  set(MEMORYCHECK_COMMAND_OPTIONS ${NATS_MEMCHECK_CMD_OPTS} CACHE INTERNAL "")

  set(NATS_COMMON_C_FLAGS "-std=c99 -pedantic")

  if(NATS_BUILD_TYPE MATCHES "Debug")
    set(NATS_COMMON_C_FLAGS "${NATS_COMMON_C_FLAGS} -ggdb")
  endif(NATS_BUILD_TYPE MATCHES "Debug")

  set(NATS_WARNINGS "-Wall -W -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wstrict-prototypes -Wwrite-strings")
  set(NATS_PLATFORM_INCLUDE "unix")

  if(APPLE)
    set(CMAKE_MACOSX_RPATH OFF)
    set(NATS_OS "DARWIN")
  else(APPLE)
    set(NATS_OS "LINUX")
    set(NATS_USE_PTHREAD "-pthread")
    set(NATS_EXTRA_LIB "rt")
    if(NATS_COVERAGE)
      set(NATS_CODE_COVERAGE "--coverage")
    endif()
  endif(APPLE)

  if (${NATS_BUILD_ARCH} MATCHES "32")
    if(NOT APPLE)
      message("-----------------------------------------------------------")
      message("If build fails, you probably need to install libc6-dev-i386")
      message("apt-get install libc6-dev-i386")
      message("-----------------------------------------------------------")
    endif(NOT APPLE)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_C_LINKER_FLAGS "${CMAKE_C_LINKER_FLAGS} -m32")
  endif(${NATS_BUILD_ARCH} MATCHES "32")

elseif(WIN32)
  set(NATS_OS "_WIN32")
  set(NATS_PLATFORM_INCLUDE "win")
endif(UNIX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${NATS_CODE_COVERAGE} ${NATS_COMMON_C_FLAGS} ${NATS_USE_PTHREAD} ${NATS_WARNINGS}")

add_definitions(-D${NATS_OS})
add_definitions(-D_REENTRANT)
if(NATS_BUILD_WITH_TLS)
add_definitions(-DNATS_HAS_TLS)
if(NATS_BUILD_WITH_TLS_CLIENT_METHOD)
add_definitions(-DNATS_USE_TLS_CLIENT_METHOD)
endif(NATS_BUILD_WITH_TLS_CLIENT_METHOD)
endif(NATS_BUILD_WITH_TLS)

#---------------------------------------------------------------------
# Add to the 'clean' target the list (and location) of files to remove

list(APPEND NATS_INSTALLED_FILES "${CMAKE_INSTALL_PREFIX}/include/nats.h")
list(APPEND NATS_INSTALLED_FILES "${CMAKE_INSTALL_PREFIX}/include/status.h")
list(APPEND NATS_INSTALLED_FILES "${CMAKE_INSTALL_PREFIX}/include/version.h")
list(APPEND NATS_INSTALLED_FILES "${CMAKE_INSTALL_PREFIX}/lib/${nats}")
list(APPEND NATS_INSTALLED_FILES "${CMAKE_INSTALL_PREFIX}/lib/${nats_static}")

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${NATS_INSTALLED_FILES}")
#---------------------------------------------------------------------

if(NATS_UPDATE_VERSION)
#------------
# Versionning

set(NATS_VERSION_MAJOR  1)
set(NATS_VERSION_MINOR  3)
set(NATS_VERSION_PATCH  6)
set(NATS_VERSION_SUFFIX "")

set(NATS_VERSION_REQUIRED_NUMBER 0x010100)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h
        @ONLY)

configure_file(
        ${CMAKE_SOURCE_DIR}/doc/DoxyFile.NATS.Client.in
        ${CMAKE_SOURCE_DIR}/doc/DoxyFile.NATS.Client
        @ONLY)
#------------
endif()

#---------------------------------------------------------------------------
# tail added
#---------------------------------------------------------------------------
if(CNATS_BUILT_IN)
    message(STATUS "cnats: built in")

    # -- head files and source files
    include_directories(${CNATS_DIR}/src)
    include_directories(${CNATS_DIR}/src/include)
    file(GLOB SOURCES    "${CNATS_DIR}/src/*.c" "${CNATS_DIR}/src/*.h")
    file(GLOB PS_SOURCES "${CNATS_DIR}/src/${NATS_PLATFORM_INCLUDE}/*.c")

    # -- TLS libs, setting in ${CNATS_DIR}/CMakeLists.txt, default is on
    if(NATS_BUILD_WITH_TLS)
    include_directories(${OPENSSL_INCLUDE_DIR})
    set(NATS_OPENSSL_LIBS ssl crypto)
    endif(NATS_BUILD_WITH_TLS)

    # -- set source files and libs, for outer use
    set(CNATS_SOURCES ${SOURCES} ${PS_SOURCES})
    set(CNATS_LIBS ${NATS_USE_PTHREAD} ${NATS_EXTRA_LIB} ${NATS_OPENSSL_LIBS})

else()
    message(STATUS "cnats: not built in, use as static lib")

    # -- add the cnats directories to build libs
    add_subdirectory(${CNATS_DIR}/src)

    # -- head files
    include_directories(${CNATS_DIR}/src)
    include_directories(${CNATS_DIR}/src/include)

    # -- set source files and libs, for outer use
    set(CNATS_LIBS ${NATS_USE_PTHREAD} ${NATS_EXTRA_LIB} nats_static)
	
    # -- retore DUP_LIBRARY_OUTPUT_PATH
    set(LIBRARY_OUTPUT_PATH ${DUP_LIBRARY_OUTPUT_PATH})
endif()

message(STATUS "cnats: linked with ${CNATS_LIBS}")

# -- restore env
set(CMAKE_C_FLAGS ${DUP_CMAKE_C_FLAGS})
set(CMAKE_INSTALL_PREFIX ${DUP_CMAKE_INSTALL_PREFIX})
