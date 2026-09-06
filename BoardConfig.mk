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
BOARD_KERNEL_CMDLINE := bootopt=64S3,32N2,64N2 buildvariant=user
BOARD_KERNEL_PAGESIZE := 2048
BOARD_MKBOOTIMG_ARGS += --header_version $(BOARD_BOOT_HEADER_VERSION)
BOARD_KERNEL_IMAGE_NAME := Image
BOARD_INCLUDE_DTB_IN_BOOTIMG := true
BOARD_KERNEL_SEPARATED_DTBO := true
TARGET_KERNEL_CONFIG := marlon_defconfig
TARGET_KERNEL_SOURCE := kernel/cubot/marlon

# Kernel - prebuilt
TARGET_FORCE_PREBUILT_KERNEL := true
ifeq ($(TARGET_FORCE_PREBUILT_KERNEL),true)
TARGET_PREBUILT_KERNEL := $(DEVICE_PATH)/prebuilts/kernel
TARGET_PREBUILT_DTB := $(DEVICE_PATH)/prebuilts/dtb.img
BOARD_MKBOOTIMG_ARGS += --dtb $(TARGET_PREBUILT_DTB)
BOARD_INCLUDE_DTB_IN_BOOTIMG := 
BOARD_PREBUILT_DTBOIMAGE := $(DEVICE_PATH)/prebuilts/dtbo.img
BOARD_KERNEL_SEPARATED_DTBO := 
endif

# Partitions
BOARD_FLASH_BLOCK_SIZE := 131072 # (BOARD_KERNEL_PAGESIZE * 64)
BOARD_BOOTIMAGE_PARTITION_SIZE := 33554432
BOARD_DTBOIMG_PARTITION_SIZE := 8388608
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 33554432
BOARD_SUPER_PARTITION_SIZE := 4294967296
BOARD_SUPER_PARTITION_GROUPS := cubot_dynamic_partitions
BOARD_CUBOT_DYNAMIC_PARTITIONS_PARTITION_LIST := system vendor product
BOARD_CUBOT_DYNAMIC_PARTITIONS_SIZE := 4290772992

# Platform
TARGET_BOARD_PLATFORM := mt6765
BOARD_USES_MTK_HARDWARE := true

# SELinux
BOARD_VENDOR_SEPOLICY_DIRS += $(DEVICE_PATH)/sepolicy/vendor

# Precompiled vendor sepolicy (from device)
BOARD_VENDOR_SEPOLICY_CIL_FILE := $(DEVICE_PATH)/sepolicy/vendor/vendor_sepolicy.cil
BOARD_VENDOR_SEPOLICY_VERS := $(shell cat $(DEVICE_PATH)/sepolicy/vendor/plat_sepolicy_vers.txt)
BOARD_SEPOLICY_M4DEFS += vendor_sepolicy_vers=$(BOARD_VENDOR_SEPOLICY_VERS)

# Vendor file contexts, property contexts, hwservice contexts
BOARD_VENDOR_FILE_CONTEXTS := $(DEVICE_PATH)/sepolicy/vendor/vendor_file_contexts
BOARD_VENDOR_PROPERTY_CONTEXTS := $(DEVICE_PATH)/sepolicy/vendor/vendor_property_contexts
BOARD_VENDOR_HWSERVICE_CONTEXTS := $(DEVICE_PATH)/sepolicy/vendor/vendor_hwservice_contexts
BOARD_VENDOR_VNDSERVICE_CONTEXTS := $(DEVICE_PATH)/sepolicy/vendor/vndservice_contexts
BOARD_VENDOR_SEAPP_CONTEXTS := $(DEVICE_PATH)/sepolicy/vendor/vendor_seapp_contexts
BOARD_VENDOR_MAC_PERMISSIONS := $(DEVICE_PATH)/sepolicy/vendor/vendor_mac_permissions.xml

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
BOARD_AVB_RECOVERY_KEY_PATH := external/avb/test/data/testkey_rsa4096.pem
BOARD_AVB_RECOVERY_ALGORITHM := SHA256_RSA4096
BOARD_AVB_RECOVERY_ROLLBACK_INDEX := 1
BOARD_AVB_RECOVERY_ROLLBACK_INDEX_LOCATION := 1

# VINTF
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/manifest.xml

# Inherit the proprietary files
include vendor/cubot/marlon/BoardConfigVendor.mk
