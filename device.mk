#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# Enable updating of APEXes
$(call inherit-product, $(SRC_TARGET_DIR)/product/updatable_apex.mk)

# API levels
PRODUCT_SHIPPING_API_LEVEL := 30

# Page size compatibility (4KB native MT6765 kernel)
PRODUCT_MAX_PAGE_SIZE_SUPPORTED := 4096

# fastbootd
PRODUCT_PACKAGES += \
    android.hardware.fastboot-service.lineage \
    fastbootd

# Health (AIDL)
PRODUCT_PACKAGES += \
    android.hardware.health-service.lineage \
    android.hardware.health-service.lineage_recovery

# Lights (AIDL)
PRODUCT_PACKAGES += \
    android.hardware.lights-service.lineage

# Power (AIDL)
PRODUCT_PACKAGES += \
    android.hardware.power-service.example

# Overlays
PRODUCT_ENFORCE_RRO_TARGETS := *

# Partitions
PRODUCT_USE_DYNAMIC_PARTITIONS := true

# Product characteristics
PRODUCT_CHARACTERISTICS := default

# Rootdir
PRODUCT_PACKAGES += \
    install-recovery.sh \

PRODUCT_PACKAGES += \
    fstab.mt6765 \
    fstab.mt6762 \
    fstab.enableswap \
    factory_init.connectivity.rc \
    factory_init.project.rc \
    factory_init.rc \
    init.aee.rc \
    init.ago.rc \
    init.connectivity.rc \
    init.modem.rc \
    init.mt6762.rc \
    init.mt6765.rc \
    init.mt6765.usb.rc \
    init.project.rc \
    init.sensor_1_0.rc \
    init.stnfc.rc \
    meta_init.connectivity.rc \
    meta_init.modem.rc \
    meta_init.project.rc \
    meta_init.rc \
    multi_init.rc \
    init.recovery.mt6762.rc \
    init.recovery.mt6765.rc \

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/etc/fstab.mt6765:$(TARGET_COPY_OUT_RAMDISK)/fstab.mt6765 \
    $(LOCAL_PATH)/rootdir/etc/fstab.mt6765:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.mt6765 \
    $(LOCAL_PATH)/rootdir/etc/fstab.mt6762:$(TARGET_COPY_OUT_RAMDISK)/fstab.mt6762 \
    $(LOCAL_PATH)/rootdir/etc/fstab.mt6762:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.mt6762 \
    $(LOCAL_PATH)/rootdir/etc/fstab.enableswap:$(TARGET_COPY_OUT_RAMDISK)/fstab.enableswap

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH)

# ---------------------------------------------------------------------------
# Ported-by: kelexine (https://github.com/kelexine)
# See PR/patch notes for the full per-feature portability analysis.
# ---------------------------------------------------------------------------

# Static + RRO overlays
DEVICE_PACKAGE_OVERLAYS += \
    $(LOCAL_PATH)/overlay \
    $(LOCAL_PATH)/overlay-lineage

PRODUCT_PACKAGES += \
    RoundedCornerFW \
    RoundedCornerSysUI \
    BatteryHealthOverlay \
    FpsInfoOverlay \
    WifiOverlay \
    TetheringOverlay \
    ScreenRecordOverlay

# DeviceSettings companion app -- currently just Double-tap-to-wake, self-hides
# its toggle if the ILITEK control node isn't present on a given build/kernel.
PRODUCT_PACKAGES += \
    DeviceSettings

# Keylayouts (MTK ACCDET jack-detect + keypad driver)
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/ACCDET.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/ACCDET.kl \
    $(LOCAL_PATH)/keylayout/mtk-kpd.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/mtk-kpd.kl

# libshim_vtservice is intentionally NOT added here -- see libshims/Android.bp.
# Only add `libshim_vtservice` to PRODUCT_PACKAGES if boot testing shows
# missing-symbol dlopen failures for vtservice_hidl.

# ---------------------------------------------------------------------------
# OPT-IN / DISABLED BY DEFAULT: MTK IMS framework glue (ImsInit + RRO config).
# vendor/cubot/marlon-vendor.mk ships the native volte_* IMS blobs and radio
# HAL IMS slots but no framework-side IMS management package as of this
# writing -- flip this to true only after confirming that's still the case
# and after validating VoLTE/IMS registration behavior on real hardware.
# See ImsInit/src/.../PhoneStateService.java for the full caveat.
MARLON_ENABLE_MTK_IMS_FRAMEWORK ?= false

ifeq ($(MARLON_ENABLE_MTK_IMS_FRAMEWORK),true)
PRODUCT_PACKAGES += \
    ImsInit \
    mtk-ims-telephony

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/permissions/privapp-permissions-imsinit.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/privapp-permissions-imsinit.xml
endif

# Inherit the proprietary files
$(call inherit-product, vendor/cubot/marlon/marlon-vendor.mk)
