SUMMARY = "Systemd service unit for OTA Flash Service"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

RDEPENDS:${PN} = "ota-flash-service"

SRC_URI = "file://ota-flash.service"

S = "${WORKDIR}"

inherit systemd

#Register the unit with systemd
SYSTEMD_SERVICE:${PN} = "ota-flash.service"

# Enable it by default
SYSTEMD_AUTO_ENABLE = "enable"

do_install(){
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/ota-flash.service ${D}${systemd_system_unitdir}/ota-flash.service
}

FILES:${PN} = "${systemd_system_unitdir}/ota-flash.service"
