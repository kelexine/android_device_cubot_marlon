#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

DEVICE_PATH := device/cubot/marlon

# Architecture
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 := 
TARGET_CPU_VARIANT := generic
TARGET_CPU_VARIANT_RUNTIME := cortex-a53

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv7-a-neon
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi
TARGET_2ND_CPU_VARIANT := generic
TARGET_2ND_CPU_VARIANT_RUNTIME := cortex-a53

# Bootloader
TARGET_BOOTLOADER_BOARD_NAME := v956
TARGET_NO_BOOTLOADER := true

# Display
TARGET_SCREEN_DENSITY := 320

# Kernel
BOARD_BOOT_HEADER_VERSION := 2
BOARD_KERNEL_BASE := 0x40078000
BOARD_KERNEL_CMDLINE := bootopt=64S3,32N2,64N2
BOARD_KERNEL_PAGESIZE := 2048
BOARD_RAMDISK_OFFSET := 0x11a88000
BOARD_KERNEL_TAGS_OFFSET := 0x07808000
BOARD_DTB_OFFSET := 0x07808000
BOARD_MKBOOTIMG_ARGS += --header_version $(BOARD_BOOT_HEADER_VERSION)
BOARD_MKBOOTIMG_ARGS += --ramdisk_offset $(BOARD_RAMDISK_OFFSET)
BOARD_MKBOOTIMG_ARGS += --tags_offset $(BOARD_KERNEL_TAGS_OFFSET)
BOARD_MKBOOTIMG_ARGS += --dtb_offset $(BOARD_DTB_OFFSET)
BOARD_KERNEL_IMAGE_NAME := Image.gz
BOARD_KERNEL_SEPARATED_DTBO := true
BOARD_INCLUDE_DTB_IN_BOOTIMG := true

# Kernel - build from source (marlon_defconfig)
TARGET_KERNEL_CONFIG := marlon_defconfig
TARGET_KERNEL_SOURCE := kernel/cubot/marlon
TARGET_KERNEL_CLANG_COMPILE := true
TARGET_KERNEL_ADDITIONAL_FLAGS := \
    LLVM=1 \
    LLVM_IAS=1 \
    KCFLAGS="-Wno-error -Wno-unused-command-line-argument"

TARGET_KERNEL_EXT_MODULE_ROOT := kernel/cubot/marlon-modules/source/vendor/mediatek/kernel_modules
TARGET_KERNEL_EXT_MODULES := \
    connectivity/common:kbuild \
    connectivity/gps:kbuild \
    connectivity/bt/mt66xx/legacy:kbuild \
    connectivity/wlan/adaptor:kbuild \
    connectivity/wlan/core/gen4m:kbuild \
    met_drv_v2:kbuild

BOARD_VENDOR_KERNEL_MODULES_LOAD := \
    wmt_drv.ko \
    bt_drv.ko \
    gps_drv.ko \
    wmt_chrdev_wifi.ko \
    wlan_drv_gen4m.ko \
    met.ko \
    ax_dragonite.ko \
    trace_mmstat.ko \
    kheaders.ko

# DTB & DTBO packaging configs for inline compilation (primary route)
BOARD_DTB_CFG := $(DEVICE_PATH)/configs/dtb.cfg
BOARD_DTBO_CFG := $(DEVICE_PATH)/configs/dtbo.cfg

# Allow forcing prebuilt kernel binary and trees if explicitly requested (secondary fallback)
ifeq ($(TARGET_FORCE_PREBUILT_KERNEL),true)
TARGET_PREBUILT_KERNEL := $(DEVICE_PATH)/prebuilts/kernel
TARGET_PREBUILT_DTB := $(DEVICE_PATH)/prebuilts/dtb.img
BOARD_PREBUILT_DTBOIMAGE := $(DEVICE_PATH)/prebuilts/dtbo.img
BOARD_INCLUDE_DTB_IN_BOOTIMG :=
BOARD_KERNEL_SEPARATED_DTBO :=
BOARD_MKBOOTIMG_ARGS += --dtb $(TARGET_PREBUILT_DTB)
endif

# Partitions
BOARD_FLASH_BLOCK_SIZE := 131072 # (BOARD_KERNEL_PAGESIZE * 64)
BOARD_BOOTIMAGE_PARTITION_SIZE := 33554432
BOARD_DTBOIMG_PARTITION_SIZE := 8388608
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 67108864
BOARD_CACHEIMAGE_PARTITION_SIZE := 385875968
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_SUPER_PARTITION_SIZE := 4294967296
BOARD_SUPER_PARTITION_GROUPS := main
BOARD_MAIN_PARTITION_LIST := system vendor product
BOARD_MAIN_SIZE := 4290772992

# A-only / Non-A/B
TARGET_OTA_ASSERT_DEVICE := marlon
TARGET_RELEASETOOLS_EXTENSIONS := $(DEVICE_PATH)
AB_OTA_UPDATER := false

BOARD_SYSTEMIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_PRODUCTIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_USES_METADATA_PARTITION := true
TARGET_USES_MKE2FS := true

TARGET_COPY_OUT_PRODUCT := product
TARGET_COPY_OUT_SYSTEM_EXT := system/system_ext
TARGET_COPY_OUT_VENDOR := vendor

# Platform
TARGET_BOARD_PLATFORM := mt6765
BOARD_USES_MTK_HARDWARE := true
TARGET_PROVIDES_MTK_PROPRIETARY := true

# SELinux
include device/mediatek/sepolicy_vndr/SEPolicy.mk
BOARD_VENDOR_SEPOLICY_DIRS += $(DEVICE_PATH)/sepolicy/vendor
SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += $(DEVICE_PATH)/sepolicy/system_ext/public
SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += $(DEVICE_PATH)/sepolicy/system_ext/private
SELINUX_IGNORE_NEVERALLOWS := true

# Lineage Hardware
BOARD_HARDWARE_CLASS += \
    $(DEVICE_PATH)/lineagehw

# Gestures KeyHandler
TARGET_KEY_HANDLER_LIBS := MarlonKeyHandler
TARGET_KEY_HANDLER_CLASS := org.lineageos.settings.device.KeyHandler

# Properties
TARGET_SYSTEM_PROP += $(DEVICE_PATH)/system.prop
TARGET_VENDOR_PROP += $(DEVICE_PATH)/vendor.prop
TARGET_PRODUCT_PROP += $(DEVICE_PATH)/product.prop
TARGET_SYSTEM_EXT_PROP += $(DEVICE_PATH)/system_ext.prop
TARGET_ODM_PROP += $(DEVICE_PATH)/odm.prop

# Recovery
TARGET_RECOVERY_FSTAB := $(DEVICE_PATH)/rootdir/etc/fstab.mt6765
BOARD_INCLUDE_RECOVERY_DTBO := true
TARGET_USERIMAGES_USE_EXT4 := true

TARGET_USERIMAGES_USE_F2FS := true

# Security patch level
VENDOR_SECURITY_PATCH := 2022-08-05

# Verified Boot
BOARD_AVB_ENABLE := true
BOARD_AVB_MAKE_VBMETA_IMAGE_ARGS += --flags 3
ifneq ($(wildcard vendor/lineage-priv/keys/avb.pem),)
BOARD_AVB_RECOVERY_KEY_PATH := vendor/lineage-priv/keys/avb.pem
else
BOARD_AVB_RECOVERY_KEY_PATH := external/avb/test/data/testkey_rsa4096.pem
endif
BOARD_AVB_RECOVERY_ALGORITHM := SHA256_RSA4096
BOARD_AVB_RECOVERY_ROLLBACK_INDEX := 1
BOARD_AVB_RECOVERY_ROLLBACK_INDEX_LOCATION := 1

# VINTF
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/manifest.xml
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE += $(DEVICE_PATH)/framework_compatibility_matrix.xml

# Wi-Fi
WIFI_HIDL_FEATURE_DUAL_INTERFACE := true
WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_HOSTAPD_DRIVER := NL80211
WIFI_DRIVER_FW_PATH_PARAM := "/dev/wmtWifi"
WIFI_DRIVER_FW_PATH_STA := "STA"
WIFI_DRIVER_FW_PATH_AP := "AP"
WIFI_DRIVER_FW_PATH_P2P := "P2P"
WIFI_DRIVER_STATE_CTRL_PARAM := "/dev/wmtWifi"
WIFI_DRIVER_STATE_ON := "1"
WIFI_DRIVER_STATE_OFF := "0"
WIFI_HIDL_UNIFIED_SUPPLICANT_SERVICE_RC_ENTRY := true

# Shims
TARGET_LD_SHIM_LIBS += \
    /vendor/lib/libnvram.so|libshim_base.so \
    /vendor/lib/libsysenv.so|libshim_base.so \
    /vendor/lib64/libnvram.so|libshim_base.so \
    /vendor/lib64/libsysenv.so|libshim_base.so \
    /vendor/lib/libaudiofoundation.so|libshim_audio.so \
    /vendor/lib64/libaudiofoundation.so|libshim_audio.so \
    /vendor/lib/hw/audio.primary.mt6765.so|libshim_audio.so \
    /vendor/lib64/hw/audio.primary.mt6765.so|libshim_audio.so \
    /vendor/lib64/libmtk-ril.so|libshim_base.so \
    /vendor/bin/hw/vendor.mediatek.hardware.pq@2.2-service|libshim_base.so \
    /vendor/lib/hw/android.hardware.bluetooth@1.0-impl-mediatek.so|libshim_bt.so \
    /vendor/lib64/hw/android.hardware.bluetooth@1.0-impl-mediatek.so|libshim_bt.so

# Build broken flags
BUILD_BROKEN_PREBUILT_ELF_FILES := true
BUILD_BROKEN_INCORRECT_PARTITION_IMAGES := true
BUILD_BROKEN_ELF_PREBUILT_PRODUCT_COPY_FILES := true
BUILD_BROKEN_DUP_RULES := true

# libwifi-hal-wrapper struct compatibility for pre-Android 14 QPR2 / pre-A16 QPR0 vendor blobs
# The Cubot P50 vendor blob is A11 (RP1A.200720.011), predating both struct additions.
SOONG_CONFIG_NAMESPACES += mediatek_wifi_hal
SOONG_CONFIG_mediatek_wifi_hal += use_pre_u_qpr2_struct use_pre_baklava_qpr0_struct
SOONG_CONFIG_mediatek_wifi_hal_use_pre_u_qpr2_struct := true
SOONG_CONFIG_mediatek_wifi_hal_use_pre_baklava_qpr0_struct := true

include vendor/lineage/config/BoardConfigLineage.mk
-include vendor/lineage/config/BoardConfigReservedSize.mk

