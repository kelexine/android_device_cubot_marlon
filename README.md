# Cubot P50 (P50 / marlon) Device Tree

> Unified LineageOS / AOSP device tree for the Cubot P50 (MediaTek MT6765 / MT6762)

## What is this?
This repository contains the device configuration and hardware HAL profiles for building LineageOS and AOSP-based custom ROMs for the **Cubot P50** (`P50` / `marlon` / `v956`). It is generated and maintained by kelexine.

## Device Specifications

| Feature | Specification |
| :--- | :--- |
| **SoC** | MediaTek MT6765 (Helio P35) / MT6762 (Helio P22) |
| **CPU** | Octa-core (4x 2.3 GHz Cortex-A53 & 4x 1.8 GHz Cortex-A53) |
| **GPU** | PowerVR Rogue GE8320 |
| **Memory** | 6 GB LPDDR4X RAM |
| **Storage** | 128 GB eMMC 5.1 (microSD dedicated slot) |
| **Battery** | 4200 mAh (Removable Li-Po) |
| **Display** | 720 x 1520 pixels (~276 ppi), 6.21" IPS LCD |
| **Camera** | 20 MP Main + 0.3 MP Macro + 0.3 MP Photosensitive / 20 MP Front |
| **Security** | TrustKernel TEE (Keymaster 4.0, Gatekeeper 1.0) |
| **Stock OS** | Android 11 (RP1A.200720.011) |
| **Kernel** | Linux 4.19 (BORE-compatible / stock MTK 4.19.127) |

## Features & Integration
- Dynamic Partitions (`super.img` with `system`, `vendor`, `product` groups).
- Full 64-bit / 32-bit multilib architecture support (`arm64-v8a` + `armeabi-v7a`).
- MediaTek MT6357 Audio HAL and routing XML policy configuration.
- Dual-SIM Dual-Standby (DSDS) telephony and VoLTE/IMS stack.
- PowerVR Rogue GE8320 GPU rendering (Gralloc 4.0 / HWC 2.0).

## Building

### LineageOS
```bash
source build/envsetup.sh
lunch lineage_marlon-userdebug
mka bacon -j$(nproc)
```

### AxionOS
```bash
# Repo sync with AxionAOSP manifest
repo init -u https://github.com/AxionAOSP/android.git -b lineage-23.2 --git-lfs
repo sync -c -j$(nproc --all) --force-sync --no-clone-bundle --no-tags

# Build AxionOS target
source build/envsetup.sh
lunch axion_marlon-userdebug
m bacon -j$(nproc)
```

## Contributing
- Follow Conventional Commits: `<type>(<scope>): <summary>`
- Sign-off commits with developer identity.

## License
- SPDX-License-Identifier: Apache-2.0
