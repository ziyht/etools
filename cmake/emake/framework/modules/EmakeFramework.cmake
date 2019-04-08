# =====================================================================================
#
#       Filename:  EMakeFramework.cmake
#
#    Description:  to include emake framework modules
#
#        Version:  1.0
#        Created:  2018-09-15 00:50:34 PM
#       Revision:  none
#       Compiler:  cmake
#
#         Author:  Haitao Yang, joyhaitao@foxmail.com
#        Company:
#
# =====================================================================================

set(EMAKE_FRAMEWORK_VERSION "1.1.1(4)")

get_filename_component(EMAKE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../ REALPATH)

#! 加载系统模块
include(CMakeParseArguments)

#! 加载内部模块
set(_ ${CMAKE_CURRENT_LIST_DIR})

include(${_}/EMakePropertyM.cmake)          # 内部属性
include(${_}/EMakeLogF.cmake)               # 日志打印助手
include(${_}/EMakeUtilsF.cmake)             # 一些帮助小工具

include(${_}/EMakeParseArguments.cmake)     # 参数解析
include(${_}/EMakeSetupConfigM.cmake)       # 初始化配置

include(${_}/EMakeSetupBuildEnvM.cmake)     # 设置基本的编译环境

include(${_}/EMakeSubSystemM.cmake)         # 子系统

include(${_}/EMakeProjectM.cmake)           # 设置项目或工程的版本信息
include(${_}/EMakeSetBuildTypeM.cmake)      # 设置项目编译类型，Debug、Release、...

unset(_)

EMakeInfF("------------------------------------------")
EMakeInfF("EMake Version: ${EMAKE_FRAMEWORK_VERSION}")

EMakeSetupConfigInternalM()
