# =====================================================================================
#
#       Filename:  EMakeSetupConfigM.cmake
#
#    Description:  set config for emake framework
#
#        Version:  1.0
#        Created:  2018-08-21 04:38:34 PM
#       Revision:  none
#       Compiler:  cmake
#
#         Author:  Haitao Yang, joyhaitao@foxmail.com
#        Company:
#
# =====================================================================================

macro(EMakeSetupConfigInternalM)

    EMakeGetProjectDirF(MAIN_PROJECT_ROOT_DIR MAIN_PROJECT_DIR)

    add_definitions(-DMAIN_PROJECT_ROOT_DIR="${MAIN_PROJECT_ROOT_DIR}/")
    add_definitions(-DMAIN_PROJECT_DIR="${MAIN_PROJECT_DIR}/")

    EMakeSetupBuildEnvM()

endmacro()


