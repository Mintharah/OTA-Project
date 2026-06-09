cmake_minimum_required(VERSION 3.10)
project(ota-flash-service)

set(CMAKE_CXX_STANDARD 14)

# FInd the required packages (provided by the yocto recipes)
find_package(CommonAPI REQUIRED)
find_package(CommonAPI-SomeIP REQUIRED)
find_package(vsomeip3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost 1.83.0 REQUIRED COMPONENTS thread system)

# INclude directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src-gen
    ${COMMONAPI_INCLUDE_DIRS}
    ${COMMONAPI_SOMEIP_INCLUDE_DIRS}
    ${VSOMEIP3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# Collect source files

# None edited files
set(GENERATED_SOURCES
    src-gen/v1/com/ota/OTAFlashServiceSomeIPDeployment.cpp
    src-gen/v1/com/ota/OTAFlashServiceSomeIPStubAdapter.cpp
    src-gen/v1/com/ota/OTAFlashServiceSomeIPProxy.cpp
)

# Implementation files
set(SERVICE_SOURCES
    src-gen/v1/com/ota/main.cpp
    src-gen/v1/com/ota/OTAFlashServiceStubImpl.cpp
    src-gen/v1/com/ota/FlashManager.cpp
)

# Build the binary
add_executable(ota-flash-service
    ${GENERATED_SOURCES}
    ${SERVICE_SOURCES}
)

# Link libraries
target_link_libraries(ota-flash-service
    pthread
    OpenSSL::Crypto
    CommonAPI
    CommonAPI-SomeIP
    vsomeip3
    ${Boost_LIBRARIES}
)

# Bitbake uses these to know where to put the files in the image
install(TARGETS ota-flash-service
    DESTINATION ${bindir} #/usr/bin/
)

install(FILES config/vsomeip-ota.json
    DESTINATION ${sysconfdir} #etc/
)

install(FILES src-gen/v1/com/ota/OTAFlashServiceSomeIPCatalog.json
    DESTINATION ${sysconfdir}/commonapi-someip #etc/commonapi-someip/
)