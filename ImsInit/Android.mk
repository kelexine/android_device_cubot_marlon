# Ported-by: kelexine (https://github.com/kelexine)
#
# OPT-IN / DISABLED BY DEFAULT: only built when
# MARLON_ENABLE_MTK_IMS_FRAMEWORK := true (see device.mk). See
# PhoneStateService.java for why -- this is an unverified workaround ported
# from a different vendor's IMS blob set and has not been tested on marlon.

LOCAL_PATH := $(call my-dir)

ifeq ($(MARLON_ENABLE_MTK_IMS_FRAMEWORK),true)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME := ImsInit
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true
LOCAL_VENDOR_MODULE := false

LOCAL_USE_AAPT2 := true

LOCAL_JAVA_LIBRARIES := \
    ims-common

include $(BUILD_PACKAGE)
endif
