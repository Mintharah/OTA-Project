#GPU
MACHINE_FEATURES:append = " vc4graphics"

#Enable the KMS overlay in config.txt
RPI_EXTRA_CONFIG = " \
    dtoverlay=vc4-kms-v3d\n \
    gpu_mem=128\n \
    enable_uart=1\n \
"
