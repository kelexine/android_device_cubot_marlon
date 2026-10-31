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
    android.hardware.health-service.marlon

# Lights (AIDL)
PRODUCT_PACKAGES += \
    android.hardware.lights-service.lineage

# Power (AIDL)
PRODUCT_PACKAGES += \
    android.hardware.power-service.lineage-libperfmgr \
    libmtkperf_client

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/powerhint.json:$(TARGET_COPY_OUT_VENDOR)/etc/powerhint.json

# Graphics Composer 2.1 HIDL compat
PRODUCT_PACKAGES += \
    android.hardware.graphics.composer@2.1-impl \
    android.hardware.graphics.composer@2.1-service

# Memtrack 1.0 HIDL
PRODUCT_PACKAGES += \
    android.hardware.memtrack@1.0-impl \
    android.hardware.memtrack@1.0-service

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
    PowerOffAlarm \
    boot_kmsg \
    libshim_base \
    libshim_audio \
    libshim_bt \
    libshim_c2 \
    android.hardware.bluetooth.audio-impl-mediatek \
    libshim_binder \
    libshim_camera \
    libshim_crypto \
    libshim_cxx \
    libshim_graphics \
    libshim_lights \
    libshim_ril \
    libshim_unwindstack \
    libshim_vtservice \
    init.recovery.mt6762.rc \
    init.recovery.mt6765.rc \

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/etc/fstab.mt6765:$(TARGET_COPY_OUT_RAMDISK)/fstab.mt6765 \
    $(LOCAL_PATH)/rootdir/etc/fstab.mt6762:$(TARGET_COPY_OUT_RAMDISK)/fstab.mt6762 \
    $(LOCAL_PATH)/rootdir/etc/fstab.enableswap:$(TARGET_COPY_OUT_RAMDISK)/fstab.enableswap \
    $(LOCAL_PATH)/rootdir/etc/init.mt6765.usb.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.mt6765.usb.rc

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    $(LOCAL_PATH) \
    hardware/google/interfaces \
    hardware/google/pixel \
    hardware/lineage/interfaces/power-libperfmgr \
    hardware/mediatek \
    hardware/mediatek/libmtkperf_client

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
    $(LOCAL_PATH)/keylayout/ACCDET.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/mt63xx-accdet_Headset.kl \
    $(LOCAL_PATH)/keylayout/mtk-kpd.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/mtk-kpd.kl \
    $(LOCAL_PATH)/keylayout/mtk-tpd.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/mtk-tpd.kl

# AxionOS Kernel Manager
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/ax_kernel_manager.xml:$(TARGET_COPY_OUT_VENDOR)/etc/ax_kernel_manager.xml \
    $(LOCAL_PATH)/configs/ax_kernel_manager.xml:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/ax_kernel_manager.xml

# Wi-Fi & P2P Supplicant Overlays (Enable 5GHz P2P / Quick Share)
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/wifi/wpa_supplicant.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant.conf \
    $(LOCAL_PATH)/configs/wifi/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf \
    $(LOCAL_PATH)/configs/wifi/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf
# MediaTek BesLoudness Audio Enhancement
PRODUCT_PACKAGES += \
    BesLoudness

# MediaTek IMS / VoLTE Service (Android 16 standalone)
PRODUCT_PACKAGES += \
    ImsService \
    mtk-ims-telephony

PRODUCT_PRODUCT_PROPERTIES += \
    persist.dbg.volte_avail_ovr=1 \
    persist.dbg.vt_avail_ovr=1 \
    persist.dbg.wfc_avail_ovr=1 \
    persist.vendor.vilte_support=0 \
    persist.vendor.ims_support=1 \
    persist.vendor.volte_support=1

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/permissions/privapp-permissions-com.mediatek.ims.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/permissions/privapp-permissions-com.mediatek.ims.xml


# VINTF & OTA
PRODUCT_OTA_ENFORCE_VINTF_KERNEL_REQUIREMENTS := false
PRODUCT_ENABLE_UFFD_GC := false

# Non-A/B Update
AB_OTA_UPDATER := false
PRODUCT_SOONG_NAMESPACES += bootable/deprecated-ota

# Inherit the proprietary files
$(call inherit-product, vendor/cubot/marlon/marlon-vendor.mk)

# MediaTek Hardware & Power HAL integration
PRODUCT_PACKAGES := $(filter-out vendor.mediatek.hardware.mtkpower@1.0-service, $(PRODUCT_PACKAGES))
PRODUCT_PACKAGES += \
    vendor.mediatek.hardware.mtkpower@1.2-service.stub \
    libmtkperf_client_vendor \
    libpowerhalwrap_vendor \
    libaedv

PRODUCT_COPY_FILES := $(filter-out %vendor/etc/init/vendor.mediatek.hardware.mtkpower@1.0-service.rc, $(PRODUCT_COPY_FILES))


# Legacy MTK audio HAL (android.hardware.audio@6.0-impl-mediatek.so) links against the stock
# Android 11 libaudiofoundation.so and android.hardware.audio.common@6.0-util.so (which provides
# the 2-argument audioConfigFromHal symbol required by vendor blobs).
# Install stock blobs directly onto the vendor partition to prevent linker aborts.
PRODUCT_COPY_FILES += \
    vendor/cubot/marlon/proprietary/vendor/lib/libaudiofoundation.so:$(TARGET_COPY_OUT_VENDOR)/lib/libaudiofoundation.so \
    vendor/cubot/marlon/proprietary/vendor/lib64/libaudiofoundation.so:$(TARGET_COPY_OUT_VENDOR)/lib64/libaudiofoundation.so

# Wi-Fi Stack (AIDL / Modern Source Stack via libwifi-hal-wrapper)
PRODUCT_PACKAGES := $(filter-out android.hardware.wifi@1.0-service-lazy hostapd wpa_supplicant, $(PRODUCT_PACKAGES))
PRODUCT_PACKAGES += \
    wpa_supplicant \
    hostapd \
    wpa_cli \
    hostapd_cli \
    libwifi-hal-wrapper \
    android.hardware.wifi-service

PRODUCT_COPY_FILES := $(filter-out %vendor/etc/init/android.hardware.wifi@1.0-service-lazy.rc %vendor/etc/init/android.hardware.wifi.supplicant-service.rc, $(PRODUCT_COPY_FILES))

PRODUCT_PACKAGES := $(filter-out android.hardware.lights-service.mediatek lights-mtk-default.xml, $(PRODUCT_PACKAGES))
PRODUCT_COPY_FILES := $(filter-out %vendor/etc/init/lights-mtk-default.rc, $(PRODUCT_COPY_FILES))
PRODUCT_PACKAGES := $(filter-out vibrator-mtk-default.xml, $(PRODUCT_PACKAGES))
PRODUCT_COPY_FILES := $(filter-out %vendor/etc/init/vibrator-mtk-default.rc, $(PRODUCT_COPY_FILES))

# USB Stack (AOSP basic HAL, filter out legacy MTK HIDL USB daemon)
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.adb.nonblocking_ffs=0 \
    ro.adb.nonblocking_ffs=0

PRODUCT_PACKAGES := $(filter-out android.hardware.usb@1.1-service-mediatekv2, $(PRODUCT_PACKAGES))
PRODUCT_PACKAGES += \
    android.hardware.usb@1.3-service.basic \
    android.hardware.usb@1.0.vendor:64

# Bluetooth Audio (in-process MediaTek Audio HAL provider, drop AOSP standalone daemon)
PRODUCT_PACKAGES := $(filter-out android.hardware.bluetooth.audio@2.0-service, $(PRODUCT_PACKAGES))
PRODUCT_COPY_FILES := $(filter-out %vendor/etc/init/android.hardware.bluetooth.audio@2.0-service.rc, $(PRODUCT_COPY_FILES))

# Audio Policy Configuration
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/audio/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml

# Media & Codec Configurations (Optimized MT6765 Codec2/OMX pipeline)
PRODUCT_PACKAGES += \
    libavservices_minijail.vendor

PRODUCT_COPY_FILES := $(filter-out \
    %vendor/etc/media_codecs.xml \
    %vendor/etc/media_codecs_c2.xml \
    %vendor/etc/media_codecs_google_audio.xml \
    %vendor/etc/media_codecs_google_video_le.xml \
    %vendor/etc/media_codecs_mediatek_audio.xml \
    %vendor/etc/media_codecs_mediatek_video.xml \
    %vendor/etc/media_codecs_performance.xml \
    %vendor/etc/media_profiles_V1_0.xml, \
    $(PRODUCT_COPY_FILES))

PRODUCT_COPY_FILES += \
    $(call find-copy-subdir-files,*,$(LOCAL_PATH)/configs/media,$(TARGET_COPY_OUT_VENDOR)/etc)

# Seccomp
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/seccomp/android.hardware.media.c2@1.2-extended-seccomp-policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/android.hardware.media.c2@1.2-extended-seccomp-policy \
    $(LOCAL_PATH)/configs/seccomp/android.hardware.media.c2@1.2-mediatek-seccomp-policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/android.hardware.media.c2@1.2-mediatek-seccomp-policy





