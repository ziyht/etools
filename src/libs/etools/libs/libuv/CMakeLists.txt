# include this file to import libuv to your project, like this:
#
#   set(LIBUV_DIR      ${PROJECT_SOURCE_DIR}/src/libs/libuv)    # set LIBUV_DIR, must
#   set(LIBUV_BUILT_IN ON/OFF)                                  # default is ON
#   set(LIBUV_EXTERNAL ON/OFF)                                  # use external libuv lib or not
#   include(${LIBUV_DIR}/CMakeLists.txt)                        # include me
#
# then use ${LIBUV_SOURCES} ${LIBUV_LIBS} to add source file and libs

if(LIBUV_EXTERNAL)
	message(STATUS "libuv: not built in, use exterval libuv lib")
	set(LIBUV_SOURCES)
	set(LIBUV_LIBS uv.a)
	include_directories(${LIBUV_EXTERNAL_H})
else()
#---------------------------------------------------------------------------
# -- store env
#---------------------------------------------------------------------------
if(NOT DEFINED LIBUV_BUILT_IN)
    option(LIBUV_BUILT_IN "build libuv in or use cnats as static lib" ON)
endif()
set(DUP_CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
set(DUP_CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

#---------------------------------------------------------------------------
# -- Platform specific settings
#---------------------------------------------------------------------------
MESSAGE(STATUS "libuv: Current Platform is ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
include_directories(${LIBUV_DIR}/include ${LIBUV_DIR}/src)
set(UV_PREV_SOURCES
    ${LIBUV_DIR}/include/uv.h
    ${LIBUV_DIR}/include/tree.h
    ${LIBUV_DIR}/include/uv-errno.h
    ${LIBUV_DIR}/include/uv-threadpool.h
    ${LIBUV_DIR}/include/uv-version.h
    ${LIBUV_DIR}/src/fs-poll.c
    ${LIBUV_DIR}/src/heap-inl.h
    ${LIBUV_DIR}/src/inet.c
    ${LIBUV_DIR}/src/queue.h
    ${LIBUV_DIR}/src/threadpool.c
    ${LIBUV_DIR}/src/uv-common.c
    ${LIBUV_DIR}/src/uv-common.h
	${LIBUV_DIR}/src/version.c)

if(UNIX)
	#-- Define UV cache variables that override the CMAKE and MEMCHECK variables
	set(UV_BUILD_TYPE Debug CACHE STRING "Build type: Release, Debug, RelWithDebInfo, MinRelSize")
	set(CMAKE_BUILD_TYPE ${UV_BUILD_TYPE} CACHE INTERNAL "")
	
	set(UV_BUILD_ARCH "64" CACHE STRING "32 for 32bits builds")
  
	set(UV_MEMCHECK_CMD "/usr/bin/valgrind" CACHE FILE "Memcheck tool")
	set(MEMORYCHECK_COMMAND ${UV_MEMCHECK_CMD} CACHE INTERNAL "")
  
	set(UV_MEMCHECK_CMD_OPTS "--leak-check=full --track-fds=yes --show-reachable=yes --num-callers=50" CACHE STRING "Memcheck options")
	set(MEMORYCHECK_COMMAND_OPTIONS ${UV_MEMCHECK_CMD_OPTS} CACHE INTERNAL "")
  
	set(UV_COMMON_C_FLAGS "-g --std=gnu89 -pedantic -Wall -Wextra -Wno-unused-parameter -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64")
	
	if(UV_BUILD_TYPE MATCHES "Debug")
		set(UV_COMMON_C_FLAGS "${UV_COMMON_C_FLAGS} -ggdb")
	endif(UV_BUILD_TYPE MATCHES "Debug")
	
	set(UV_WARNINGS "-Wall -W -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wstrict-prototypes -Wwrite-strings ")
	
	find_library(M_LIB NAMES m)
	find_package(Threads)
	set(UV_CORE_LIBRARY ${CMAKE_THREAD_LIBS_INIT} ${M_LIB})
	
	include_directories(${LIBUV_DIR}/src/unix)
	set(UV_CORE_SOURCES ${LIBUV_DIR}/src/unix/async.c 
					    ${LIBUV_DIR}/src/unix/atomic-ops.h 
					    ${LIBUV_DIR}/src/unix/core.c 
					    ${LIBUV_DIR}/src/unix/dl.c 
					    ${LIBUV_DIR}/src/unix/fs.c 
					    ${LIBUV_DIR}/src/unix/getaddrinfo.c 
					    ${LIBUV_DIR}/src/unix/getnameinfo.c 
					    ${LIBUV_DIR}/src/unix/internal.h 
					    ${LIBUV_DIR}/src/unix/loop-watcher.c 
					    ${LIBUV_DIR}/src/unix/loop.c 
					    ${LIBUV_DIR}/src/unix/pipe.c 
					    ${LIBUV_DIR}/src/unix/poll.c 
					    ${LIBUV_DIR}/src/unix/process.c 
					    ${LIBUV_DIR}/src/unix/signal.c 
					    ${LIBUV_DIR}/src/unix/spinlock.h 
					    ${LIBUV_DIR}/src/unix/stream.c 
					    ${LIBUV_DIR}/src/unix/tcp.c 
					    ${LIBUV_DIR}/src/unix/thread.c 
					    ${LIBUV_DIR}/src/unix/timer.c 
					    ${LIBUV_DIR}/src/unix/tty.c 
					    ${LIBUV_DIR}/src/unix/udp.c)
	
	if (APPLE)
		set(UV_OS "DARWIN")
		set(UV_APPD_DEFINIT "-D_DARWIN_USE_64_BIT_INODE=1 -D_DARWIN_UNLIMITED_SELECT=1")
		set(UV_APPD_SOURCES ${LIBUV_DIR}/src/unix/darwin.c
	                        ${LIBUV_DIR}/src/unix/darwin-proctitle.c
	                        ${LIBUV_DIR}/src/unix/fsevents.c
	                        ${LIBUV_DIR}/src/unix/kqueue.c
	                        ${LIBUV_DIR}/src/unix/proctitle.c
	                        ${LIBUV_DIR}/src/unix/pthread-barrier.c)
		set_source_files_properties(${LIBUV_DIR}/src/unix/stream.c PROPERTIES COMPILE_FLAGS -Wno-gnu-folding-constant)
	endif()			
	
	if (NOT APPLE)
			set(UV_APPD_DEFINIT "-Wstrict-aliasing")
	endif()
	
	if (CMAKE_SYSTEM_NAME MATCHES "Linux")
		find_library(DL_LIB NAMES dl)
		find_library(RT_LIB NAMES rt)
		
		set(UV_OS "LINUX")
		set(UV_APPD_LIBRARY ${DL_LIB} ${RT_LIB})
		set(UV_APPD_DEFINIT "-D_GNU_SOURCE -D_POSIX_C_SOURCE=200112")
		set(UV_APPD_SOURCES ${LIBUV_DIR}/src/unix/linux-core.c 
						    ${LIBUV_DIR}/src/unix/linux-inotify.c 
						    ${LIBUV_DIR}/src/unix/linux-syscalls.c 
						    ${LIBUV_DIR}/src/unix/linux-syscalls.h 
						    ${LIBUV_DIR}/src/unix/proctitle.c)
	
	elseif(CMAKE_SYSTEM_NAME MATCHES "SunOS")
		find_library(KSTAT_LIB    NAMES kstat)
		find_library(NSL_LIB      NAMES nsl)
		find_library(SENDFILE_LIB NAMES sendfile)
		find_library(SOCKET_LIB   NAMES socket)
		
		set(UV_OS "SunOS")
		set(UV_APPD_LIBRARY ${KSTAT_LIB} ${NSL_LIB} ${SENDFILE_LIB} ${SOCKET_LIB})
		set(UV_APPD_DEFINIT "-D__EXTENSIONS__ -D_XOPEN_SOURCE=500")
		set(UV_APPD_SOURCES ${LIBUV_DIR}/src/unix/sunos.c)
					   
	elseif(CMAKE_SYSTEM_NAME MATCHES "AIX")
		find_library(PERFSTAT_LIB NAMES perfstat)
		
		set(UV_OS "AIX")
		set(UV_APPD_LIBRARY ${PERFSTAT_LIB})
		set(UV_APPD_DEFINIT "-D_XOPEN_SOURCE=500 -D_LINUX_SOURCE_COMPAT")
	    set(UV_APPD_SOURCES ${LIBUV_DIR}/include/uv-aix.h ${LIBUV_DIR}/src/unix/aix.c)
		
	elseif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD" or CMAKE_SYSTEM_NAME MATCHES "DragonFlyBSD")
		find_library(KVM_LIB NAMES kvm)
		
		set(UV_OS "FreeBSD")
		set(UV_APPD_LIBRARY ${KVM_LIB})
		set(UV_APPD_SOURCES ${LIBUV_DIR}/src/unix/freebsd.c 
						    ${LIBUV_DIR}/src/unix/kqueue.c)
	
   elseif(CMAKE_SYSTEM_NAME MATCHES "NetBSD")
	   find_library(KVM_LIB NAMES kvm)
	   
	   set(UV_OS "NetBSD")
	   set(UV_APPD_LIBRARY ${KVM_LIB})
	   set(UV_APPD_SOURCES ${LIBUV_DIR}/src/unix/netbsd.c 
						   ${LIBUV_DIR}/src/unix/kqueue.c)			   
    
   elseif(CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
	   find_library(KVM_LIB NAMES kvm)
	   set(UV_OS "OpenBSD")
	   set(UV_APPD_LIBRARY ${KVM_LIB})
	   set(UV_APPD_SOURCES ${LIBUV_DIR}/src/unix/openbsd.c 
						   ${LIBUV_DIR}/src/unix/kqueue.c)			   
	endif()
		
	
	if (${UV_BUILD_ARCH} MATCHES "32")
	  if(NOT APPLE)
		message("-----------------------------------------------------------")
		message("If build fails, you probably need to install libc6-dev-i386")
		message("apt-get install libc6-dev-i386")
		message("-----------------------------------------------------------")
	  endif(NOT APPLE)
  
	  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	  set(CMAKE_C_LINKER_FLAGS "${CMAKE_C_LINKER_FLAGS} -m32")
	endif()
	
	
elseif(WIN32)
	set(UV_OS "_WIN32")
	include_directories(${LIBUV_DIR}/src/win)
	set(UV_CORE_LIBRARY advapi32 iphlpapi psapi userenv shell32 ws2_32)
	set(UV_CORE_DEFINIT "-DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0600 -D_CRT_SECURE_NO_WARNINGS")
	set(UV_CORE_SOURCES ${LIBUV_DIR}/src/win/async.c 
		                 ${LIBUV_DIR}/src/win/atomicops-inl.h 
		                 ${LIBUV_DIR}/src/win/core.c 
		                 ${LIBUV_DIR}/src/win/detect-wakeup.c 
		                 ${LIBUV_DIR}/src/win/dl.c 
		                 ${LIBUV_DIR}/src/win/error.c 
		                 ${LIBUV_DIR}/src/win/fs-event.c 
		                 ${LIBUV_DIR}/src/win/fs.c 
		                 ${LIBUV_DIR}/src/win/getaddrinfo.c 
		                 ${LIBUV_DIR}/src/win/getnameinfo.c 
		                 ${LIBUV_DIR}/src/win/handle.c 
		                 ${LIBUV_DIR}/src/win/handle-inl.h 
		                 ${LIBUV_DIR}/src/win/internal.h 
		                 ${LIBUV_DIR}/src/win/loop-watcher.c 
		                 ${LIBUV_DIR}/src/win/pipe.c 
		                 ${LIBUV_DIR}/src/win/poll.c 
		                 ${LIBUV_DIR}/src/win/process-stdio.c 
		                 ${LIBUV_DIR}/src/win/process.c 
		                 ${LIBUV_DIR}/src/win/req.c 
		                 ${LIBUV_DIR}/src/win/req-inl.h 
		                 ${LIBUV_DIR}/src/win/signal.c 
		                 ${LIBUV_DIR}/src/win/stream.c 
		                 ${LIBUV_DIR}/src/win/stream-inl.h 
		                 ${LIBUV_DIR}/src/win/tcp.c 
		                 ${LIBUV_DIR}/src/win/thread.c 
		                 ${LIBUV_DIR}/src/win/timer.c 
		                 ${LIBUV_DIR}/src/win/tty.c 
		                 ${LIBUV_DIR}/src/win/udp.c 
		                 ${LIBUV_DIR}/src/win/util.c 
		                 ${LIBUV_DIR}/src/win/winapi.c 
		                 ${LIBUV_DIR}/src/win/winapi.h 
		                 ${LIBUV_DIR}/src/win/winsock.c 
		                 ${LIBUV_DIR}/src/win/winsock.h)
endif()	# UNIX


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${UV_CODE_COVERAGE} ${UV_COMMON_C_FLAGS} ${UV_WARNINGS}")
add_definitions(-D${UV_OS} ${UV_PREV_DEFINIT} ${UV_CORE_DEFINIT} ${UV_APPD_DEFINIT})

#---------------------------------------------------------------------------
# tail added
#---------------------------------------------------------------------------
if(LIBUV_BUILT_IN)
    message(STATUS "libuv: built in")
	set(LIBUV_SOURCES ${UV_PREV_SOURCES} ${UV_CORE_SOURCES} ${UV_APPD_SOURCES})
	set(LIBUV_LIBS    ${UV_PREV_LIBRARY} ${UV_CORE_LIBRARY} ${UV_APPD_LIBRARY})
	message(STATUS "libuv: linked with ${LIBUV_LIBS}")
else()
    message(STATUS "libuv: not built in, use as static lib")
	
	add_library(inner_libuv STATIC ${UV_PREV_SOURCES} ${UV_CORE_SOURCES} ${UV_APPD_SOURCES})
	TARGET_LINK_LIBRARIES(inner_libuv ${UV_PREV_LIBRARY} ${UV_CORE_LIBRARY} ${UV_APPD_LIBRARY})
	
	set(LIBUV_SOURCES)
	set(LIBUV_LIBS inner_libuv)
endif()

# -- restore env
set(CMAKE_C_FLAGS ${DUP_CMAKE_C_FLAGS})
set(CMAKE_INSTALL_PREFIX ${DUP_CMAKE_INSTALL_PREFIX})

endif() #LIBUV_EXTERNAL
