SUMMARY = "OTA Flash Service -  receives imabe over CommonAPI/vSomeIP and flashes to inactive parition"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Dependencies that must be built before this recipe
DEPENDS = "vsomeip commonapi-core commonapi-someip openssl"

# Source files
SRC_URI = " \
    file://CMakeLists.txt \
    file://src-gen/v1/com/ota/main.cpp \
    file://src-gen/v1/com/ota/OTAFlashServiceStubDefault.hpp \
    file://src-gen/v1/com/ota/OTAFlashServiceStubImpl.hpp \
    file://src-gen/v1/com/ota/OTAFlashServiceStubImpl.cpp \
    file://src-gen/v1/com/ota/FlashManager.cpp \
    file://src-gen/v1/com/ota/FlashManager.hpp \
    file://src-gen/v1/com/ota/OTAFlashService.hpp \
    file://src-gen/v1/com/ota/OTAFlashServiceProxyBase.hpp \
    file://src-gen/v1/com/ota/OTAFlashServiceProxy.hpp \
    file://src-gen/v1/com/ota/OTAFlashServiceStub.hpp \
    file://src-gen/v1/com/ota/OTAFlashServiceSomeIPDeployment.hpp \
    file://src-gen/v1/com/ota/OTAFlashServiceSomeIPDeployment.cpp \
    file://src-gen/v1/com/ota/OTAFlashServiceSomeIPProxy.hpp \
    file://src-gen/v1/com/ota/OTAFlashServiceSomeIPProxy.cpp \
    file://src-gen/v1/com/ota/OTAFlashServiceSomeIPStubAdapter.hpp \
    file://src-gen/v1/com/ota/OTAFlashServiceSomeIPStubAdapter.cpp \
    file://src-gen/v1/com/ota/OTAFlashServiceSomeIPCatalog.json \
    file://config/vsomeip-ota.json \
"

# Inherit cmake
inherit cmake

EXTRA_OECMAKE = " \
    -DCMAKE_BUILD_TYPE=Release \
    -Dbindir=${bindir} \
    -Dsysconfdir=${sysconfdir} \
"
# Tells bitbake the cmakelists is at the top of the directory
S = "${WORKDIR}"

# Runtime dependencies
RDEPENDS:${PN} = "vsomeip commonapi-core commonapi-someip"

# WHich files to package
FILES:${PN} = " \
    ${bindir}/ota-flash-service \
    ${sysconfdir}/vsomeip-ota.json \
    ${sysconfdir}/commonapi-someip/OTAFlashServiceSomeIPCatalog.json \
"