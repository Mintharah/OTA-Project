SUMMARY = "CommonAPI C++ SOME/IP binding"
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=815ca599c9df247a0c7f619bab123dad"

SRC_URI = "git://github.com/COVESA/capicxx-someip-runtime.git;branch=master;protocol=https"
SRCREV = "86dfd69802e673d00aed0062f41eddea4670b571"

S = "${WORKDIR}/git"

DEPENDS = "vsomeip commonapi-core"

inherit cmake

EXTRA_OECMAKE = " \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_INSTALLED_COMMONAPI=ON \
"

FILES:${PN}     = "${libdir}/libCommonAPI-SomeIP*.so.*"

FILES:${PN}-dev = " \
    ${includedir} ${libdir}/cmake \
    ${libdir}/libCommonAPI-SomeIP.so \
    ${libdir}/pkgconfig/CommonAPI-SomeIP.pc \
"