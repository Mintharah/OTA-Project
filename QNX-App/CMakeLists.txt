cmake_minimum_required(VERSION 3.10)
project(ota-client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Find packages ────────────────────────────────────────────────────────────
find_package(CommonAPI REQUIRED)
find_package(CommonAPI-SomeIP REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# ─── Source files ─────────────────────────────────────────────────────────────
set(SRC_GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SOURCES
    # Your client code
    ${SRC_GEN_DIR}/OTAClient.cpp
    ${SRC_GEN_DIR}/main_client.cpp

    # Generated CommonAPI SomeIP glue
    ${SRC_GEN_DIR}/OTAFlashServiceSomeIPDeployment.cpp
    ${SRC_GEN_DIR}/OTAFlashServiceSomeIPProxy.cpp
    ${SRC_GEN_DIR}/OTAFlashServiceSomeIPStubAdapter.cpp
)

# ─── Executable ───────────────────────────────────────────────────────────────
add_executable(ota-client ${SOURCES})

# ─── Include directories ──────────────────────────────────────────────────────
target_include_directories(ota-client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../meta-project/recipes-ota/ota-flash-service/files/src-gen
    ${SRC_GEN_DIR}
    ${COMMONAPI_INCLUDE_DIRS}
    ${COMMONAPI_SOMEIP_INCLUDE_DIRS}
    $ENV{QNX_TARGET}/usr/include
)

# ─── Link libraries ───────────────────────────────────────────────────────────
target_link_libraries(ota-client PRIVATE
    CommonAPI
    CommonAPI-SomeIP
    vsomeip3
    ${CMAKE_THREAD_LIBS_INIT}
    OpenSSL::Crypto
    socket        
)

# ─── Copy vsomeip config next to the binary ───────────────────────────────────
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/vsomeip-client.json
    ${CMAKE_CURRENT_BINARY_DIR}/vsomeip-client.json
    COPYONLY
)

# ─── Install ──────────────────────────────────────────────────────────────────
install(TARGETS ota-client DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/vsomeip-client.json DESTINATION bin)

# ─── Print summary ────────────────────────────────────────────────────────────
message(STATUS "CommonAPI     : ${COMMONAPI_INCLUDE_DIRS}")
message(STATUS "CommonAPI-SIP : ${COMMONAPI_SOMEIP_INCLUDE_DIRS}")
message(STATUS "QNX target    : $ENV{QNX_TARGET}")
