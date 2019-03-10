# =====================================================================================
#
#       Filename:  EMakeSetupBuildEnvM.cmake
#
#    Description:  setup build environment
#
#        Version:  1.0
#        Created:  16/11/2017 04:38:34 PM
#       Revision:  none
#       Compiler:  cmake
#
#         Author:  Haitao Yang, joyhaitao@foxmail.com
#        Company:
#
# =====================================================================================

macro(_SetupBuildArch)

    if( CMAKE_SIZEOF_VOID_P EQUAL 8 )

        set(ARCH 64)

        if(WIN32)
            set(BUILD_ARCH "win64")
        else()
            set(BUILD_ARCH "x64")
        endif()

    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)

        set(ARCH 86)

        if(WIN32)
            set(BUILD_ARCH "win32")
        else()
            set(BUILD_ARCH "x86")
        endif()

    else()

        EMakeErrF("the build architecture is not 64 or 32, please check you system env, this should not happen")

    endif()

    if(WIN32)

        string(TOLOWER ${CMAKE_SYSTEM_NAME} _sys_name)
        set(INSTALL_POSTFIX ${_sys_name}${CMAKE_SYSTEM_VERSION}_x${ARCH})

    else()

        set(_checkos_sh ${EMAKE_DIR}/framework/sbin/checkos.sh)

        set(_os)

        execute_process(
            COMMAND "bash" ${_checkos_sh}
            RESULT_VARIABLE     RESULT_VAR
            OUTPUT_VARIABLE     _os
            ERROR_VARIABLE      _error
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(RESULT_VAR)
            EMakeErrF("Failed to obtain dependence path of ${_kit}.\n${RESULT_VAR}\n${PROJECT_BINARY_DIR}\n${_error}")
        endif()

        if(_os)
            set(INSTALL_POSTFIX ${_os}_x${ARCH})
        else()
            string(TOLOWER ${CMAKE_SYSTEM_NAME} _sys_name)
            set(INSTALL_POSTFIX ${_sys_name}${CMAKE_SYSTEM_VERSION}_x${ARCH})
        endif()

    endif()

endmacro()


macro(_SetupCXXFlags)

    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
    if(COMPILER_SUPPORTS_CXX11)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    elseif(COMPILER_SUPPORTS_CXX0X)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
         message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()

    if(NOT WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    endif()

endmacro()

if(g_QtCore_lib_type STREQUAL "STATIC")
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif()

macro(EMakeSetupBuildEnvM)

    _SetupBuildArch()
    _SetupCXXFlags()

endmacro()
