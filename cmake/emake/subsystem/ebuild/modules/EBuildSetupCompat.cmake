# =====================================================================================
#
#       Filename:  EBuildSetupCompat.cmake
#
#    Description:  settings to compat multi platforms
#
#        Version:  1.0
#        Created:  2017.12.17 04:38:34 PM
#       Revision:  none
#       Compiler:  cmake
#
#         Author:  Haitao Yang, joyhaitao@foxmail.com
#        Company:
#
# =====================================================================================

# ----------------------------------
# 相关命令兼容设置
#
if(WIN32)
    set(COPY_CMD    xcopy /Y /Q )
else()
    set(COPY_CMD    cp   )
endif()


# ----------------------------------
# 外部库默认配置
if(WIN32)
    set(EBUILD_EXT_DF_CMAKE_GENERATOR "CodeBlocks - NMake Makefiles")
endif()

set(EBUILD_EXT_DF_CMAKE_CACHE_ARGS
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS_INIT:STRING=${CMAKE_CXX_FLAGS_INIT}
    -DCMAKE_C_FLAGS_INIT:STRING=${CMAKE_C_FLAGS_INIT}
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_MACOSX_RPATH:BOOL=${CMAKE_MACOSX_RPATH}
    )


#
# 平台设定，在进行平台依赖检查时用到
#

# -- 支持的平台
set(PLATFORMS WIN32 APPLE LINUX REDHAT REDHAT6 REDHAT7 CENTOS CENTOS6 CENTOS7 UBUNTU UBUNTU14)

# -- 当前所属主平台
if(WIN32)
    set(MY_PLATFORMS WIN32)
elseif(APPLE)
    set(MY_PLATFORMS APPLE)
else()
    set(MY_PLATFORMS LINUX)
endif()

# -- 当前所属次平台 todo

