# =====================================================================================
#
#       Filename:  ETestFileGenerator.cmake
#
#    Description:  file generators for the etest
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



macro(ETestGenTestCaseContent cases content)

    set(${content}

"/**
* this file is create by subsystem 'ETest' from EMake framework
*/

#include <etest.h>

static int ${cases}_case1()
{
    eexpect_num(1, 1)\;      // passed

    return ETEST_OK\;
}

static int ${cases}_case2()
{
    eexpect_num(1, 0)\;      // will failed

    return ETEST_OK\;
}

int ${cases}(int argc, char* argv[])
{
    ETEST_RUN( ${cases}_case1() )\;
    ETEST_RUN( ${cases}_case2() )\;

    return ETEST_OK\;
}

"
        )

endmacro()    # ETestEngineGenCaseContent


macro(ETestGenCaseFiles _dir _files)

    foreach(_file ${${_files}})

        if(NOT EXISTS ${_dir}/${_file})

            string(REGEX REPLACE "(.c|.cpp|.cxx)$" "" _name ${_file})

            ETestGenTestCaseContent(${_name} _content)

            file(WRITE ${_dir}/${_file} ${_content} )

        endif()

    endforeach()

endmacro()

macro(ETestGenHeadFile _file _cases)

    set(_content)

    list(APPEND _content
"/**
* this file is create by subsystem 'ETest' from EMake framework
*/

extern \"C\" { \n\n")

    foreach(_case ${${_cases}})
        list(APPEND _content "    int ${_case}(int, char*[])\;\n")
    endforeach()

    list(APPEND _content "\n}\n")

    file(WRITE ${_file} ${_content} )

endmacro()
