SUMMARY = "OTA Client Qt6 UI"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = " \
    qtbase \
    qtbase-native \
    qtdeclarative \
    qtdeclarative-native \
    commonapi-core \
    commonapi-someip \
    vsomeip \
"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI = " \
    file://main.cpp \
    file://OTAClientController.h \
    file://OTAClientController.cpp \
    file://Main.qml \
    file://WaitingView.qml \
    file://ReceivingView.qml \
    file://ResultView.qml \
    file://images/ \
    file://src-gen/ \
    file://CMakeLists.txt \
    file://vsomeip-client.json \
    file://qt_app.service \
"

S = "${WORKDIR}"

inherit cmake qt6-cmake systemd

SYSTEMD_SERVICE:${PN} = "qt_app.service"
SYSTEMD_AUTO_ENABLE   = "enable"

do_install() {
    install -d ${D}${sysconfdir}
    install -m 0644 ${WORKDIR}/vsomeip-client.json ${D}${sysconfdir}/vsomeip-client.json

    install -d ${D}${bindir}
    install -m 0755 ${B}/appOTAUpdater ${D}${bindir}/ota-client-ui

    install -d ${D}/usr/share/ota-client-ui/images
    install -m 0644 ${WORKDIR}/images/*.gif ${D}/usr/share/ota-client-ui/images/

    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/qt_app.service ${D}${systemd_unitdir}/system/qt_app.service

}

FILES:${PN} = " \
    ${bindir}/ota-client-ui \
    /usr/share/ota-client-ui/ \
    ${sysconfdir}/vsomeip-client.json \
    ${systemd_unitdir}/system/qt_app.service \
"
