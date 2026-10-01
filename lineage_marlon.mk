#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit from marlon device
$(call inherit-product, device/cubot/marlon/device.mk)

# Inherit some common Lineage stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Protobuf vendor compat for Widevine DRM and ClearKey CAS
PRODUCT_SOURCE_ROOT_DIRS := $(filter-out -prebuilts/misc/protobuf_vendorcompat,$(PRODUCT_SOURCE_ROOT_DIRS))
PRODUCT_PACKAGES += \
    libprotobuf-cpp-full-3.9.1-vendorcompat \
    libprotobuf-cpp-lite-3.9.1-vendorcompat

PRODUCT_DEVICE := marlon
PRODUCT_NAME := lineage_marlon
PRODUCT_BRAND := CUBOT
PRODUCT_MODEL := P50
PRODUCT_MANUFACTURER := cubot

PRODUCT_GMS_CLIENTID_BASE := android-cubot

PRODUCT_BUILD_PROP_OVERRIDES += \
    BuildDesc="full_v956-user 11 RP1A.200720.011 20220816 release-keys" \
    BuildFingerprint=CUBOT/P50_EEA/P50:11/RP1A.200720.011/20220816:user/release-keys
