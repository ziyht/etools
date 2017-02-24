project(etools)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

set(CMAKE_C_FLAGS "-g -O3 -Wall -Wextra -pedantic -Wcast-align -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -fno-common -std=gnu99 -fPIC")

# -- add cnats to project
#    use ${CNATS_SOURCES} ${CNATS_LIBS} to add source file an libs
#    cnats is built-in and tls is on in default
set(CNATS_DIR      ${PROJECT_SOURCE_DIR}/src/libs/etools/libs/cnats)
set(CNATS_BUILT_IN		ON )
set(NATS_BUILD_WITH_TLS OFF)		                          # TLS needs openssl libs
include(${CNATS_DIR}/CMakeLists.txt)

# -- add libuv to project
#    use ${LIBUV_SOURCES} ${LIBUV_LIBS} to add source file an libs
#    libuv is built-in in default
set(LIBUV_DIR      ${PROJECT_SOURCE_DIR}/src/libs/etools/libs/libuv)    # set LIBUV_DIR, must
set(LIBUV_BUILT_IN      ON )                                # default is ON
set(LIBUV_EXTERNAL      OFF)                                # use external libuv lib or not
set(LIBUV_EXTERNAL_H ${EXTERN_LIBS_DIR}/libuv/1.9.1/include)# external header files dir
include(${LIBUV_DIR}/CMakeLists.txt)                        # include me

set(EJSON_FS   src/libs/etools/ejson.h src/libs/etools/ejson.c)
set(ESTR_FS    src/libs/etools/estr.h  src/libs/etools/estr.c)
set(ETHP_FS    src/libs/etools/ethp.h  src/libs/etools/ethp.c  ${EJSON_FS})
set(ENATS_FS   src/libs/etools/enats.h src/libs/etools/enats.c ${EJSON_FS} ${CNATS_SOURCES} ${LIBUV_SOURCES})

set(EJSON_LIB  m)
set(ETHP_LIB   ${EJSON_LIB} pthread )
set(ENATS_LIB  ${EJSON_LIB} ${CNATS_LIBS} ${LIBUV_LIBS})

include_directories(src/libs/etools)

add_executable(test_ejson main.c ${EJSON_FS})
add_executable(test_estr  main.c ${ESTR_FS})
add_executable(test_etp   main.c ${ETHP_FS})
add_executable(test_enats main.c ${ENATS_FS})

TARGET_LINK_LIBRARIES(test_ejson ${EJSON_LIB})
TARGET_LINK_LIBRARIES(test_etp   ${ETHP_LIB})
TARGET_LINK_LIBRARIES(test_enats ${ENATS_LIB})
