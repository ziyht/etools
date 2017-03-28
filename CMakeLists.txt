project(etools)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

set(CMAKE_C_FLAGS "-g -O0 -Wall -Wextra -pedantic -Wcast-align -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wuninitialized -fno-common -std=gnu99 -fPIC")

# -- add cnats to project
#    use ${CNATS_SOURCES} ${CNATS_LIBS} to add source file and libs
#    cnats is built-in and tls is on in default
set(CNATS_DIR      ${PROJECT_SOURCE_DIR}/src/libs/etools/libs/cnats)
set(CNATS_BUILT_IN		ON )
set(NATS_BUILD_WITH_TLS ON)		                          # TLS needs openssl libs
include(${CNATS_DIR}/CMakeLists.txt)

# -- add libuv to project
#    use ${LIBUV_SOURCES} ${LIBUV_LIBS} to add source file and libs
#    libuv is built-in in default
set(LIBUV_DIR      ${PROJECT_SOURCE_DIR}/src/libs/etools/libs/libuv)    # set LIBUV_DIR, must
set(LIBUV_BUILT_IN      ON )                                # default is ON
set(LIBUV_EXTERNAL      OFF)                                # use external libuv lib or not
set(LIBUV_EXTERNAL_H ${EXTERN_LIBS_DIR}/libuv/1.9.1/include)# external header files dir
include(${LIBUV_DIR}/CMakeLists.txt)                        # include me

# -- add base64 to project
#    use ${BASE64_SRCS} ${BASE64_LIBS} to add source file and libs
set(BASE64_DIR   ${PROJECT_SOURCE_DIR}/src/libs/etools/libs/base64)
include(${BASE64_DIR}/CMakeLists.txt)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/libs/etools/libs/crypt_blowfish CRYPT_SRCS)

set(EJSON_FS   src/libs/etools/ejson.h src/libs/etools/ejson.c src/libs/etools/ejson.h src/libs/etools/etype.h)
set(ESTR_FS    src/libs/etools/estr.h  src/libs/etools/estr.c)
set(ESL_FS     src/libs/etools/esl.h   src/libs/etools/esl.c)
set(ERT_FS     src/libs/etools/ert.h   src/libs/etools/ert.c   src/libs/etools/compat.h ${EJSON_FS})
set(ETM_FS     src/libs/etools/etimer.h src/libs/etools/etimer.c ${EJSON_FS} ${ESL_FS} ${ERT_FS})

set(ENATS_FS   src/libs/etools/enats.h src/libs/etools/enats.c ${EJSON_FS} ${CNATS_SOURCES})
set(EB64_FS    src/libs/etools/eb64.h  src/libs/etools/eb64.c  ${ESTR_FS} ${BASE64_SRCS})
set(EDES_FS    src/libs/etools/edes.h  src/libs/etools/edes.c  ${ESTR_FS})
set(ECRYPT_FS  src/libs/etools/ecrypt.h  src/libs/etools/ecrypt.c ${ESTR_FS} ${CRYPT_SRCS})

set(EJSON_LIB  m)
set(ETHP_LIB   ${EJSON_LIB} pthread )
set(ENATS_LIB  ${EJSON_LIB} ${CNATS_LIBS} ${LIBUV_LIBS})

include_directories(src/libs/etools)
include_directories(src/libs/etools/libs)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/ejson  TEST_EJSON_FS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/estr   TEST_ESTR_FS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/esl    TEST_ESL_FS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/ert    TEST_ERT_FS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/etimer TEST_ETM_FS)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/enats  TEST_ENATS_FS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/eb64   TEST_EB64_FS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/edes   TEST_EDES_FS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/ecrypt TEST_ECRYPT_FS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/libuv  TEST_LIBUV_FS)

# -- bin --
add_executable(test_ejson ${TEST_EJSON_FS} ${EJSON_FS})
add_executable(test_estr  ${TEST_ESTR_FS}  ${ESTR_FS})
add_executable(test_esl   ${TEST_ESL_FS}   ${ESL_FS})

add_executable(test_ert   ${TEST_ERT_FS} ${ERT_FS})
add_executable(test_etimer ${TEST_ETM_FS} ${ETM_FS})

add_executable(test_enats ${TEST_ENATS_FS} ${ENATS_FS} ${ESTR_FS})

add_executable(test_eb64  ${TEST_EB64_FS} ${EB64_FS})
add_executable(test_edes  ${TEST_EDES_FS} ${EDES_FS} ${EB64_FS})
add_executable(test_ecrypt ${TEST_ECRYPT_FS} ${ECRYPT_FS})

add_executable(test_libuv ${TEST_LIBUV_FS} ${LIBUV_SOURCES})

TARGET_LINK_LIBRARIES(test_ejson ${EJSON_LIB})
TARGET_LINK_LIBRARIES(test_ert   ${ETHP_LIB})
TARGET_LINK_LIBRARIES(test_etimer pthread m)
TARGET_LINK_LIBRARIES(test_enats ${ENATS_LIB})
TARGET_LINK_LIBRARIES(test_libuv  ${ETHP_LIB} ${BASE64_LIBS} dl)

# -- prev version --
set(TRANSPORT_CNAT_FS  src/libs/etools/transport_cnats.h src/libs/etools/transport_cnats.c ${EJSON_FS} ${CNATS_SOURCES} ${LIBUV_SOURCES})
set(TRANSPORT_CNAT_LIB ${EJSON_LIB} ${CNATS_LIBS} ${LIBUV_LIBS})

aux_source_directory(${PROJECT_SOURCE_DIR}/src/test/transport_cnats TEST_TRANSPORT_CNATS)

add_executable(test_transport_cnats ${TEST_TRANSPORT_CNATS} ${TRANSPORT_CNAT_FS})
TARGET_LINK_LIBRARIES(test_transport_cnats ${TRANSPORT_CNAT_LIB})
