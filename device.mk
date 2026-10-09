#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# Enable updating of APEXes
$(call inherit-product, $(SRC_TARGET_DIR)/product/updatable_apex.mk)

# Setup Dalvik VM configs for 6GB RAM
$(call inherit-product, frameworks/native/build/phone-xhdpi-6144-dalvik-heap.mk)

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

# Graphics Composer 2.1 HIDL compat
PRODUCT_PACKAGES += \
    android.hardware.graphics.composer@2.1-impl \
    android.hardware.graphics.composer@2.1-service

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
    init.target.rc \
    meta_init.connectivity.rc \
    meta_init.modem.rc \
    meta_init.project.rc \
    meta_init.rc \
    multi_init.rc \
    pwroff_alarm \
    boot_kmsg \
    init.recovery.mt6762.rc \
    init.recovery.mt6765.rc \

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/etc/fstab.mt6765:$(TARGET_COPY_OUT_RAMDISK)/fstab.mt6765 \
    $(LOCAL_PATH)/rootdir/etc/fstab.mt6762:$(TARGET_COPY_OUT_RAMDISK)/fstab.mt6762 \
    $(LOCAL_PATH)/rootdir/etc/fstab.enableswap:$(TARGET_COPY_OUT_RAMDISK)/fstab.enableswap

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH)

# Overlays
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

# DeviceSettings
PRODUCT_PACKAGES += \
    DeviceSettings

# Encore Hardware Daemon & AIDL Interface
PRODUCT_PACKAGES += \
    encored \
    vendor.cubot.hardware.encore

# Keylayouts
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/ACCDET.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/ACCDET.kl \
    $(LOCAL_PATH)/keylayout/mtk-kpd.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/mtk-kpd.kl

# AxionOS Kernel Manager
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/ax_kernel_manager.xml:$(TARGET_COPY_OUT_VENDOR)/etc/ax_kernel_manager.xml \
    $(LOCAL_PATH)/configs/ax_kernel_manager.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/ax_kernel_manager.xml

# Wi-Fi & P2P Supplicant Overlays (Enable 5GHz P2P / Quick Share)
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/wifi/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf \
    $(LOCAL_PATH)/configs/wifi/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf


# MTK IMS Framework (opt-in)
MARLON_ENABLE_MTK_IMS_FRAMEWORK ?= false

ifeq ($(MARLON_ENABLE_MTK_IMS_FRAMEWORK),true)
PRODUCT_PACKAGES += \
    ImsInit \
    mtk-ims-telephony

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/permissions/privapp-permissions-imsinit.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/privapp-permissions-imsinit.xml
endif

# VINTF & OTA
PRODUCT_OTA_ENFORCE_VINTF_KERNEL_REQUIREMENTS := false
PRODUCT_ENABLE_UFFD_GC := false

# Non-A/B Update
AB_OTA_UPDATER := false
PRODUCT_SOONG_NAMESPACES += bootable/deprecated-ota

# Inherit the proprietary files
$(call inherit-product, vendor/cubot/marlon/marlon-vendor.mk)
