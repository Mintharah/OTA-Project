SUMMARY = "Minimal RPI3 image with vsomeip + Qt6 IPC app"

inherit core-image

IMAGE_FEATURES += " \
    ssh-server-openssh \
    debug-tweaks \
"

IMAGE_INSTALL:append = " \
    kernel-modules \
    vsomeip \
    vsomeip-routingmanagerd \
    commonapi-core \
    commonapi-someip \
    qtbase \
    qtdeclarative \
    qtnetworkauth \
    openssh \
    iproute2 \
    ethtool \
    htop \
    liberation-fonts \
    fontconfig \
    fontconfig-utils \
    ota-flash-service \
    ota-flash-initscript \
    network-config \
    ota-client-ui \
"