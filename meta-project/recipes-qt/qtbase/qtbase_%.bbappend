PACKAGECONFIG:append:class-target = " linuxfb dbus fontconfig"
PACKAGECONFIG:remove:class-target = " xcb eglfs kms gbm egl vulkan wayland"

PACKAGECONFIG:class-native = "gui widgets jpeg png dbus no-opengl openssl zlib zstd"
PACKAGECONFIG:class-nativesdk = "gui widgets jpeg png dbus no-opengl openssl zlib zstd"