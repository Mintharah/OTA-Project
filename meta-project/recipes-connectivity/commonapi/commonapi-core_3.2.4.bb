SUMMARY = "CommonAPI C++ core runtime"
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=815ca599c9df247a0c7f619bab123dad"

SRC_URI = "git://github.com/COVESA/capicxx-core-runtime.git;branch=master;protocol=https"
SRCREV = "0e1d97ef0264622194a42f20be1d6b4489b310b5"

S = "${WORKDIR}/git"

inherit cmake

EXTRA_OECMAKE = "-DCMAKE_BUILD_TYPE=Release"

FILES:${PN} = "${libdir}/libCommonAPI*.so.*"

FILES:${PN}-dev += " \
    ${includedir} \
    ${libdir}/pkgconfig \
    ${libdir}/cmake \
"
