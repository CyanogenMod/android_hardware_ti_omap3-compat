LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    stagefright_overlay_output.cpp \
    TIHardwareRenderer.cpp \
    TIOMXPlugin.cpp

LOCAL_CFLAGS := $(PV_CFLAGS_MINUS_VISIBILITY)

TOP ?= $(ANDROID_BUILD_TOP)

LOCAL_C_INCLUDES:= \
    $(TOP)/frameworks/base/include/media/stagefright/openmax \
    $(TOP)/hardware/ti/omap3-compat/liboverlay

LOCAL_SHARED_LIBRARIES := \
    libbinder \
    libutils \
    libcutils \
    libui \
    libdl \
    libsurfaceflinger_client

LOCAL_MODULE := libstagefrighthw

include $(BUILD_SHARED_LIBRARY)

