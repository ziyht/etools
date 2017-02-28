project(etools)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

set(CMAKE_C_FLAGS "-g -O0 -Wall -Wextra -pedantic -Wcast-align -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wuninitialized -fno-common -std=gnu99 -fPIC")

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


set(EJSON_FS   src/libs/etools/ejson.h src/libs/etools/ejson.c src/libs/etools/ejson.h src/libs/etools/etype.h)
set(ESTR_FS    src/libs/etools/estr.h  src/libs/etools/estr.c)
set(ETP_FS     src/libs/etools/etp.h   src/libs/etools/etp.c   src/libs/etools/compat.h ${EJSON_FS})
set(ENATS_FS   src/libs/etools/enats.h src/libs/etools/enats.c ${EJSON_FS} ${CNATS_SOURCES} ${LIBUV_SOURCES})

set(EJSON_LIB  m)
set(ETHP_LIB   ${EJSON_LIB} pthread )
set(ENATS_LIB  ${EJSON_LIB} ${CNATS_LIBS} ${LIBUV_LIBS})

include_directories(src/libs/etools)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/ejson TEST_EJSON_FS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/estr  TEST_ESTR_FS)


# -- bin --
add_executable(test_ejson ${TEST_EJSON_FS} ${EJSON_FS})
add_executable(test_estr  ${TEST_ESTR_FS}  ${ESTR_FS})
add_executable(test_etp   main.c ${ETP_FS})
#add_executable(test_enats main.c ${ENATS_FS})

TARGET_LINK_LIBRARIES(test_ejson ${EJSON_LIB})
TARGET_LINK_LIBRARIES(test_etp   ${ETHP_LIB})
#TARGET_LINK_LIBRARIES(test_enats ${ENATS_LIB})


# -- prev version --
set(TRANSPORT_CNAT_FS  src/libs/etools/transport_cnats.h src/libs/etools/transport_cnats.c ${EJSON_FS} ${CNATS_SOURCES} ${LIBUV_SOURCES})
set(TRANSPORT_CNAT_LIB ${EJSON_LIB} ${CNATS_LIBS} ${LIBUV_LIBS})

aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/transport_cnats TEST_TRANSPORT_CNATS)

add_executable(test_transport_cnats ${TEST_TRANSPORT_CNATS} ${TRANSPORT_CNAT_FS})
TARGET_LINK_LIBRARIES(test_transport_cnats ${TRANSPORT_CNAT_LIB})
