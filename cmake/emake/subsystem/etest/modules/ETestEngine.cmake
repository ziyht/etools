# =====================================================================================
#
#       Filename:  ETestEngine.cmake
#
#    Description:  engine for the etest
#
#        Version:  1.0
#        Created:  2018.08.28 21:29:34 PM
#       Revision:  none
#       Compiler:  cmake
#
#         Author:  Haitao Yang, joyhaitao@foxmail.com
#        Company:
#
# =====================================================================================

enable_testing()

set(ETEST_SYSTEM_DIR   ${SUB_SYSTEM_DIR})
set(ETEST_MODULE_DIR   ${SUB_SYSTEM_DIR}/cmake)
set(ETEST_TEMPLATE_DIR ${SUB_SYSTEM_DIR}/template)

include(${ETEST_TEMPLATE_DIR}/CMakeLists.txt)

macro(ETestEngineInitM)
    if(NOT TARGET etest)
        add_custom_target(etest)
        #set_target_properties(etest PROPERTIES UnitTestOption ON ModuleTestOption ON)

    endif()

    enable_testing()
    add_definitions(-DETEST_OK=0 -DETEST_ERR=1)
endmacro()

#
# 将会生成如下变量:
#    M_DEST 疏理过的传入路径
#    M_FULL 疏理过的完整路径
#
macro(ETestEngineCheckPath i_dest)

    set(M_DEST ${i_dest})
    set(M_FULL ${CMAKE_CURRENT_LIST_DIR}/${M_DEST})

    if(EXISTS ${M_FULL})
        EMakeValidatePath(${M_DEST} M_DEST)
        EMakeValidatePath(${M_FULL} M_FULL)
    elseif(EXISTS ${M_DEST})
        EMakeValidatePath(${M_DEST} M_DEST)
        set(M_FULL ${M_DEST})
    elseif(M_CREATE)
        file(MAKE_DIRECTORY ${M_FULL})
    else()
        EMakeErrF("the DEST '${i_dest}' not exsit when adding a test for ${PROJECT_NAME}")
    endif()

    if(NOT EXISTS ${M_FULL}/CMakeLists.txt)
        if(M_CREATE)
            file(COPY ${ETEST_TEMPLATE_DIR}/CMakeLists.txt DESTINATION ${M_FULL})
        else()
            EMakeErrF("the file '${M_FULL}/CMakeLists.txt' not exist, please check you testing project files\n"
                      "NOTE: you can using 'CREATE' option in 'ETestAdd()' to create a template file ")
        endif()
    endif()

    set(M_DEST ${M_DEST} PARENT_SCOPE)
    set(M_FULL ${M_FULL} PARENT_SCOPE)

endmacro()

#
# 须预先设置以下值:
#   M_TEST      测试名称
#   M_TYPE      测试类型
#   M_CASES     测试用例(*.c|*.cpp)
#   M_SRCS      额外文件
#   M_DEPENDS   依赖
#   M_ON        开关
#
function(ETestEngineAddTest i_target)

    # 检查 开关
    if(M_ON)
        EMakeInfF("  |- [ON ] Test:${i_target}" PREFIX "   ")
    else()
        EMakeInfF("  |- [OFF] Test:${i_target}" PREFIX "   ")
        return()
    endif()

    set(_main_cxx ${i_target}_main.cxx)
    set(_head_h   ${i_target}_main.h)

    ETestEngineInitM()

    # 生成 ctest 主函数(main)文件
    create_test_sourcelist(_src_list ${_main_cxx} ${M_CASES} EXTRA_INCLUDE ${_head_h})

    # 添加测试可执行文件生成
    project(${i_target})
    add_executable(${i_target} ${_src_list} ${M_SRCS})

    # 检查和设置依赖
    EBuildCheckDependsM(${i_target} ${M_DEPENDS})
    target_link_libraries(${i_target} ${KIT_NAME_LIB} ${KIT_DEPENDS} ${KIT_LIBRARIES_SYS} ${KIT_LINK_DIRS})

    if(KIT_IS_QT_PROJECT)
        set(CMAKE_AUTOMOC ON)
    else()
        set(CMAKE_AUTOMOC OFF)
    endif()

    # 添加自动测试步骤 并 筛选 gcc 的 case
    set(_gcc_cases)
    set(_id 1)
    foreach(_case ${M_CASES})

        set(_is_gcc)
        if(${_case} MATCHES ".c$")
            set(_is_gcc 1)
        endif()

        string(REGEX REPLACE ".[cpp|c]$" "" _case ${_case})

        if(_is_gcc)
            list(APPEND _gcc_cases ${_case})
        endif()

        add_test(NAME "${PROJECT_NAME}:${_id}.${_case}" COMMAND ${i_target} ${_case})

        math(EXPR _id "${_id} + 1")
    endforeach()

    # 为 gcc 的 case 创建头文件
    if(_gcc_cases)

        set(_lines)

        list(APPEND _lines
"/**
* this file is create by etest of emake
*/

extern \"C\" { \n\n")

        foreach(_case ${_gcc_cases})
            list(APPEND _lines "    int ${_case}(int, char*[])\;\n")
        endforeach()

        list(APPEND _lines "\n}\n")

        file(WRITE ${PROJECT_BINARY_DIR}/${_head_h} ${_lines} )

    endif()

    set(M_TEST)

endfunction()
