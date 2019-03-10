# =====================================================================================
#
#       Filename:  setup.cmake
#
#    Description:  set the configuration of emake frame work or subsystem
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

macro(EMakeSetupConfigM)

    # 设置 cmake 日志级别
    EMakeSetLogLevelF(2)             # 0: 关闭，1：dbg，2：inf，3：wrn

    if(UNIX)
        set(CMAKE_C_FLAGS "-g -O0 -Wall -Wextra -pedantic -Wcast-align -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wuninitialized -Wno-switch -fno-common -std=gnu99 -fPIC")
    elseif(MSVC)
        set(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:libcmtd.lib")
    endif(UNIX)

endmacro()
