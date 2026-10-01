#
# File: axion_marlon.mk
# Author: kelexine <https://github.com/kelexine>
# Date: 2026-09-08
# Purpose: AxionOS product configuration for Cubot P50 (marlon)
# SPDX-FileCopyrightText: 2026 kelexine
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit from marlon device
$(call inherit-product, device/cubot/marlon/device.mk)

# AxionOS performance and feature flags
TARGET_DISABLE_EPPE := true
TARGET_FACE_UNLOCK_SUPPORTED := true
TARGET_BOOT_ANIMATION_RES := 720

# AxionFx Audio Enhancement Suite
TARGET_INCLUDE_AXFX := true
ifeq ($(TARGET_INCLUDE_AXFX),true)
$(call inherit-product-if-exists, packages/apps/AxionFx/config.mk)
PRODUCT_COPY_FILES := $(filter-out %vendor/etc/audio_effects.xml,$(PRODUCT_COPY_FILES))
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/audio/audio_effects_axion.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.xml
endif

# AxionOS "About Phone" device properties
AXION_MAINTAINER := kelexine
AXION_PROCESSOR := MediaTek_Helio_P35_MT6765
AXION_CAMERA_REAR_INFO := 12,5
AXION_CAMERA_FRONT_INFO := 20

# CPUsets configuration (mt6765: homogeneous octa-core A53, no big.LITTLE)
AXION_CPU_BG := 0-7
AXION_CPU_FG := 0-7
AXION_CPU_LIMIT_BG := 0-3
AXION_CPU_UNLIMIT_UI := 0-7
AXION_CPU_LIMIT_UI := 0-7
AXION_CPU_DISPLAY := 0-7
AXION_CPU_AUDIO := 0-7

# Bypass charging: unsupported, no input_suspend node on this kernel
BYPASS_CHARGE_SUPPORTED := false

# Inherit common Lineage/Axion configuration
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Protobuf vendor compat for Widevine DRM and ClearKey CAS
PRODUCT_SOURCE_ROOT_DIRS := $(filter-out -prebuilts/misc/protobuf_vendorcompat,$(PRODUCT_SOURCE_ROOT_DIRS))
PRODUCT_PACKAGES += \
    libprotobuf-cpp-full-3.9.1-vendorcompat \
    libprotobuf-cpp-lite-3.9.1-vendorcompat

PRODUCT_DEVICE := marlon
PRODUCT_NAME := axion_marlon
PRODUCT_BRAND := CUBOT
PRODUCT_MODEL := P50
PRODUCT_MANUFACTURER := cubot

PRODUCT_GMS_CLIENTID_BASE := android-cubot

PRODUCT_BUILD_PROP_OVERRIDES += \
    BuildDesc="full_v956-user 11 RP1A.200720.011 20220816 release-keys" \
    BuildFingerprint=CUBOT/P50_EEA/P50:11/RP1A.200720.011/20220816:user/release-keys
