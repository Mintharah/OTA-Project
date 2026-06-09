#!/bin/sh
# /data/home/qnxuser/run-ota-client.sh

IMAGE=${1:-/data/home/qnxuser/project-image-raspberrypi3-64.rootfs.ext3}

export VSOMEIP_CONFIGURATION=/data/home/qnxuser/vsomeip-ota-client.json
export VSOMEIP_APPLICATION_NAME=OTAClient
export COMMONAPI_DEFAULT_BINDING=someip

exec /data/home/qnxuser/ota-client "$IMAGE"