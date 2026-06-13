# OTA Project

An Over-The-Air firmware update system that transfers a root filesystem image from a **QNX QEMU client** to a **Raspberry Pi 3 (Yocto)** target over Ethernet, using COVESA CommonAPI / SOME/IP as the IPC layer.

---
<img width="1920" height="913" alt="image" src="https://github.com/user-attachments/assets/a8ac15e6-8acf-4b2c-9a10-dfb162f5dabf" />

## Architecture Overview

```
┌──────────────────────────────┐         Ethernet (192.168.1.x)        ┌───────────────────────────────┐
│  QNX (QEMU x86-64)           │ ────────────────────────────────────► │  Raspberry Pi 3 (Yocto)       │
│                              │                                        │                               │
│  ota-client  (CLI sender)    │  SOME/IP / CommonAPI                  │  ota-flash-service (systemd)  │
│  ota-client-ui  (Qt6 GUI)    │  startTransfer / sendChunk /          │  FlashManager                 │
│                              │  finalizeTransfer                      │  (writes rootfs_b, changes   │
│  vsomeip-ota-client.json     │                                        │  boot variable, reboots)      │
└──────────────────────────────┘                                        └───────────────────────────────┘
```

The service interface is defined in `OTAFlash.fidl` and auto-generated into CommonAPI C++ stubs/proxies under `src-gen/v1/com/ota/`.

---

## Repository Layout

```
OTA-Project/
├── QNX-App/                        # Cross-compiled QNX client (ota-client CLI)
│   ├── src-gen/v1/com/ota/
│   │   ├── OTAClient.hpp / .cpp    # Sends image in 64 KB chunks over CommonAPI
│   │   └── main_client.cpp         # Entry point: usage: ota-client <image.img>
│   ├── CMakeLists.txt              # QNX cross-build config
│   ├── vsomeip-client.json         # SOME/IP config for the CLI client
│
├── QNX-Image/                      # QNX OS image customisation files
│   ├── data_files.custom.vsomeip   # IFS manifest: injects binaries/libs into QNX image
│   ├── post_start.custom           # Runs at boot: assigns 192.168.1.101 to vtnet0
│   ├── profile.custom              # Shell environment additions
│   └── run-ota-client.sh           # Convenience wrapper to launch ota-client on QNX
│
├── meta-project/                   # Yocto meta-layer (target = raspberrypi3-64, distro = scarthgap)
│   ├── conf/
│   │   ├── layer.conf              # Layer registration; depends on meta-raspberrypi, qt6-layer, etc.
│   │   └── distro/project-distro.conf
│   ├── recipes-bsp/rpi-config/     # RPi firmware config tweaks
│   ├── recipes-connectivity/
│   │   ├── vsomeip/                # vsomeip 3.4.10 recipe + patch to disable tests
│   │   ├── commonapi/              # CommonAPI-Core 3.2.4 + CommonAPI-SomeIP 3.2.4
│   │   └── network-config/         # Static IP 192.168.1.100 on eth0
│   ├── recipes-core/images/
│   │   └── project-image.bb        # Final image recipe (ssh, Qt6, vsomeip, OTA stack)
│   ├── recipes-qt/                 # qtbase append
│   └── recipes-ota/
│       ├── ota-flash-service/      # Server: receives chunks, writes rootfs_b, arms U-Boot
│       │   ├── files/src-gen/…     # FlashManager, OTAFlashServiceStubImpl, generated glue
│       │   ├── files/fidl/         # OTAFlash.fidl + .fdepl (service definition source of truth)
│       │   ├── files/config/vsomeip-ota.json
│       │   └── ota-flash-service.bb
│       ├── ota-flash-initscript/   # Systemd unit (ota-flash.service) — auto-enabled at boot
│       └── ota-client-ui/          # Qt6/QML GUI for the QNX side (monitors FlashProgress events)
│           ├── files/*.cpp / *.h / *.qml
│           ├── files/images/       # Animated GIFs: sleep / loading / success / error states
│           └── ota-client-ui.bb
│
└── copy-image-to-qnx.sh            # SCP helper: copies .ext3 image from Yocto deploy to QNX
```

---

## Network Configuration

| Host | IP | Role |
|---|---|---|
| Raspberry Pi 3 (Yocto) | `192.168.1.100` | OTA Flash Service (server) |
| QNX QEMU | `192.168.1.101` | OTA Client (sender) |

SOME/IP service discovery uses multicast `192.168.1.255:30490/udp`. The reliable (TCP) channel is on port `30510`, unreliable (UDP) on `30509`.

---

## Prerequisites

**QNX side (build host):**
- QNX SDP 8.0 (`~/qnx-qemu/qnx800/`)
- CMake ≥ 3.10
- Cross-toolchain file at `~/kms/toolchain-qnx-x86_64.cmake`
- vsomeip3, CommonAPI-Core 3.2.4, and CommonAPI-SomeIP 3.2.4 pre-built for QNX x86-64 and installed into the QNX sysroot

**Yocto side (build host):**
- Yocto Scarthgap
- `meta-raspberrypi`, `openembedded-layer`, `meta-python`, `networking-layer`, `qt6-layer`
- Add `meta-project` to `bblayers.conf`

---

## Building

### 1. Build the Yocto image (RPi3 target)

```bash
source oe-init-build-env
bitbake project-image
```

The built image lands at:
```
tmp/deploy/images/raspberrypi3-64/project-image-raspberrypi3-64.rootfs.ext3
```

Flash this to your SD card and boot the RPi3. The `ota-flash-service` systemd unit starts automatically.

### 2. Build the QNX client

```bash
source ~/qnx-qemu/qnx800/qnxsdp-env.sh

mkdir -p build && cd build

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/kms/toolchain-qnx-x86_64.cmake \
  -DCMAKE_INSTALL_PREFIX=$HOME/qnx-qemu/qnx800/target/qnx/x86_64/usr/local \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_MAKE_PROGRAM=$(which make) \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_CXX_FLAGS="-D_QNX_SOURCE -D_POSIX_C_SOURCE=200112L -Vgcc_ntox86_64 -D__BYTE_ORDER=__LITTLE_ENDIAN -D__LITTLE_ENDIAN=1234 -D__BIG_ENDIAN=4321" \
  -DCMAKE_EXE_LINKER_FLAGS="-L$HOME/qnx-qemu/qnx800/target/qnx/x86_64/usr/local/lib -Wl,-rpath-link,$HOME/qnx-qemu/qnx800/target/qnx/x86_64/usr/local/lib" \
  -DCommonAPI_DIR=$HOME/qnx-qemu/qnx800/target/qnx/x86_64/usr/local/lib/cmake/CommonAPI-3.2.4 \
  -DCommonAPI-SomeIP_DIR=$HOME/qnx-qemu/qnx800/target/qnx/x86_64/usr/local/lib/cmake/CommonAPI-SomeIP-3.2.4 \
  -Dvsomeip3_DIR=$HOME/qnx-qemu/qnx800/target/qnx/x86_64/usr/local/lib/cmake/vsomeip3 \
  -DBOOST_ROOT=$HOME/qnx-qemu/qnx800/target/qnx/x86_64/usr/local

make -j$(nproc)
```

The resulting `ota-client` binary is copied into the QNX IFS image via `data_files.custom.vsomeip`.

---

## First-Time SD Card Setup (RPi3)

After booting the freshly flashed RPi3, you need to create the inactive rootfs partition (`mmcblk0p3`) that OTA will write to:

```bash
fdisk /dev/mmcblk0
# Inside fdisk:
n        # new partition
p        # primary
3        # partition number 3
1472512  # start sector (2048-aligned, leaves gap after p2)
         # press Enter to accept default end, or use 3569663 for ~1 GB
w        # write and exit

reboot

# After reboot:
mkfs.ext4 /dev/mmcblk0p3
```

This only needs to be done once.

---

## Running an OTA Update

### Step 1 — Copy the new image to QNX

From the host:

```bash
./copy-image-to-qnx.sh
```

This SCPs `project-image-raspberrypi3-64.rootfs.ext3` to `/data/home/qnxuser/` on the QNX QEMU instance.

### Step 2 — Start the OTA service on RPi3

The service starts automatically via systemd. To check or start manually:

```bash
systemctl status ota-flash.service
# or
systemctl start ota-flash.service
```

### Step 3 — Run the QNX CLI client

On the QNX QEMU instance:

```bash
# Using the convenience wrapper:
/data/home/qnxuser/run-ota-client.sh

# Or manually:
VSOMEIP_CONFIGURATION=/data/home/qnxuser/vsomeip-ota-client.json \
VSOMEIP_APPLICATION_NAME=OTAClient \
COMMONAPI_DEFAULT_BINDING=someip \
/data/home/qnxuser/ota-client /data/home/qnxuser/project-image-raspberrypi3-64.rootfs.ext3
```

The client will:
1. Connect to `OTAFlashService` via SOME/IP service discovery
2. Send `startTransfer` with image size, chunk count, and SHA-256 checksum
3. Stream the image in 64 KB chunks
4. Call `finalizeTransfer` — the RPi3 verifies the checksum, writes to `rootfs_b`, arms U-Boot, and reboots

### Step 4 (optional) — Qt6 GUI on QNX

It automatically starts up on boot.

```bash
VSOMEIP_APPLICATION_NAME=OTAClientUI \
VSOMEIP_CONFIGURATION=/etc/vsomeip-client.json \
COMMONAPI_DEFAULT_BINDING=someip \
COMMONAPI_CONFIG=/etc/commonapi.ini \
ota-client-ui -platform linuxfb
```

The GUI subscribes to `FlashProgress` broadcast events from the service and displays animated status screens (waiting → receiving → success/error).

---

## Service Interface Summary (`OTAFlash.fidl`)

| Method | Description |
|---|---|
| `startTransfer(imageSize, totalChunks, checksum)` → `transferId, errorCode` | Opens a transfer session |
| `sendChunk(transferId, chunkIndex, chunkData)` → `errorCode` | Sends one 64 KB chunk |
| `finalizeTransfer(transferId)` → `errorCode` | Verifies checksum, flashes, arms U-Boot |
| `cancelTransfer(transferId)` → `errorCode` | Aborts an in-progress transfer |
| broadcast `FlashProgress` | Emits `FlashState` + `progressPercent` continuously during flashing |

---


**Absolute host paths in `data_files.custom.vsomeip`:**
All source paths reference `/home/yasmine/...`. These will break on any other developer's machine. They should be replaced with variables or paths relative to the build environment.
