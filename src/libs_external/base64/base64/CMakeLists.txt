cmake_minimum_required(VERSION 2.8)

#---------------------------------------------------------------------------
# -- Platform env check
#---------------------------------------------------------------------------
include (${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)
check_include_files(cpuid.h __HAVE_CPUID_H)

if(__HAVE_CPUID_H)
    add_definitions(-DHAVE_CPUID_H)
endif()

#---------------------------------------------------------------------------
# -- Platform specific settings
#---------------------------------------------------------------------------
set(BASE64_SRC
    ${BASE64_DIR}/include/libbase64.h
    ${BASE64_DIR}/lib/codec_choose.c
    ${BASE64_DIR}/lib/lib.c
)

set(BASE64_ARCH
    ${BASE64_DIR}/lib/arch/avx/codec.c
    ${BASE64_DIR}/lib/arch/avx2/codec.c
    ${BASE64_DIR}/lib/arch/generic/codec.c
    ${BASE64_DIR}/lib/arch/neon32/codec.c
    ${BASE64_DIR}/lib/arch/neon64/codec.c

    ${BASE64_DIR}/lib/arch/sse41/codec.c
    ${BASE64_DIR}/lib/arch/sse42/codec.c
    ${BASE64_DIR}/lib/arch/ssse3/codec.c
)

include_directories(${BASE64_DIR}/include)

set(BASE64_SRC ${BASE64_SRC} ${BASE64_ARCH})
set(BASE64_LIB )

add_library(base64 STATIC ${BASE64_SRC} ${BASE64_ARCH})
