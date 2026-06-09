#!/bin/bash
# copy-image-to-qnx.sh

QNX_USER="qnxuser"
QNX_IP="192.168.1.101"
QNX_DEST="/data/home/qnxuser"
IMAGE_DIR="$HOME/yocto/share/tmp/deploy/images/raspberrypi3-64"
IMAGE_NAME="project-image-raspberrypi3-64.rootfs.ext3"

echo "Copying $IMAGE_NAME to QNX at $QNX_IP..."

scp "$IMAGE_DIR/$IMAGE_NAME" "$QNX_USER@$QNX_IP:$QNX_DEST/$IMAGE_NAME"

if [ $? -eq 0 ]; then
    echo "Done — image is at $QNX_DEST/$IMAGE_NAME"
else
    echo "Failed to copy image"
    exit 1
fi