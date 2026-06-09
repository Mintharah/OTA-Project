SUMMARY = "SOME/IP"
DESCRIPTION = "vsomeip implements SOME/IP including service discovery"
HOMEPAGE = "https://github.com/COVESA/vsomeip"
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=9741c346eef56131163e13b9db1241b3"

SRC_URI = "git://github.com/COVESA/vsomeip.git;protocol=https;nobranch=1 \
           file://disable-tests.patch"
SRCREV = "01a0df9e9b993297367e843b75291d9aa06909e5"

S="${WORKDIR}/git"

DEPENDS = " \
    boost \ 
    dlt-daemon \
    "

inherit cmake

EXTRA_OECMAKE = " \
    -DENABLE_TESTS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_SIGNAL_HANDLING=1 \
    -DDIAGNOSIS_ADDRESS=0x01 \
    -DVSOMEIP_INSTALL_ROUTINGMANAGERD=ON \
"
#Split package cleanly
PACKAGES += "${PN}-routingmanagerd ${PN}-tools"

#Core shared library
FILES:${PN} = " \
    ${libdir}/libvsomeip3*.so.* \
"

#Routing manager daemon, installs to /usr/bin
FILES:${PN}-routingmanagerd = " \
    ${bindir}/routingmanagerd \
    /usr/etc \
    /usr/etc/vsomeip \
    /usr/etc/vsomeip/* \
"

#Default JSON configs that vsomeip installs under /usr/etc
FILES:${PN}-config = " \
    /usr/etc/vsomeip/ \
    /usr/etc/vsomeip/*.json \
"

#pkg-config file
FILES:${PN}-dev += " \
    ${includedir} \
    ${libdir}/pkgconfig \
    ${libdir}/cmake \
"

FILES:${PN}-tools = "${bindir}/vsomeip_*"

RDEPENDS:${PN} = "boost"
RDEPENDS:${PN}-routingmanagerd = "vsomeip"

ERROR_QA:remove = "patch-fuzz"