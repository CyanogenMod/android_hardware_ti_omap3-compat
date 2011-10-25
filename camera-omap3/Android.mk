ifdef BOARD_USES_TI_CAMERA_HAL

################################################

LOCAL_PATH := $(call my-dir)

TI_OMX_TOP    ?= $(ANDROID_BUILD_TOP)/hardware/ti/omap3-compat/omx
TI_OMX_IMAGE  ?= $(TI_OMX_TOP)/image/src/openmax_il
TI_OMX_SYSTEM ?= $(TI_OMX_TOP)/system/src/openmax_il

TI_BRIDGE_TOP := $(ANDROID_BUILD_TOP)/hardware/ti/omap3-compat/dspbridge
TI_BRIDGE_INCLUDES := $(TI_BRIDGE_TOP)/libbridge/inc

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    CameraHal.cpp \
    CameraHal_Utils.cpp \
    MessageQueue.cpp \
    
LOCAL_SHARED_LIBRARIES:= \
    libdl \
    libui \
    libbinder \
    libutils \
    libcutils \
    libcamera_client \
    libsurfaceflinger_client

LOCAL_C_INCLUDES += \
    $(ANDROID_BUILD_TOP)/frameworks/base/include/camera \
    $(ANDROID_BUILD_TOP)/frameworks/base/include/binder \
    $(ANDROID_BUILD_TOP)/hardware/ti/omap3-compat/liboverlay

LOCAL_CFLAGS += -fno-short-enums

ifdef HARDWARE_OMX

LOCAL_SRC_FILES += \
    scale.c \
    JpegEncoder.cpp \
    JpegEncoderEXIF.cpp \

LOCAL_C_INCLUDES += \
    $(TI_BRIDGE_INCLUDES) \
    $(TI_OMX_SYSTEM)/lcml/inc \
    $(TI_OMX_SYSTEM)/omx_core/inc \
    $(TI_OMX_SYSTEM)/common/inc \
    $(TI_OMX_IMAGE)/jpeg_enc/inc \
    $(ANDROID_BUILD_TOP)/external/libexif

LOCAL_CFLAGS += -O0 -g3 -fpic -fstrict-aliasing -DIPP_LINUX -D___ANDROID___ -DHARDWARE_OMX

# Required for Motorola Defy, Cliq2 & DroidX
# kernel/arch/arm/plat-omap/include/dspbridge/wcdioctl.h
ifeq ($(TARGET_USE_OMX_RECOVERY),true)
LOCAL_CFLAGS += -DMOTO_FORCE_RECOVERY
endif

LOCAL_SHARED_LIBRARIES += \
    libbridge \
    libLCML \
    libOMX_Core

LOCAL_STATIC_LIBRARIES := \
    libexifgnu

endif

################################################

ifdef FW3A

LOCAL_C_INCLUDES += \
    $(ANDROID_BUILD_TOP)/hardware/ti/omap3-compat/fw3A/include \
    $(ANDROID_BUILD_TOP)/hardware/ti/omap3-compat/fw3A/include/fw/api/linux

LOCAL_SHARED_LIBRARIES += \
    libdl \
    libicamera \
    libicapture \

LOCAL_CFLAGS += -O0 -g3 -DIPP_LINUX -D___ANDROID___ -DFW3A -DICAP

endif

ifdef IMAGE_PROCESSING_PIPELINE

LOCAL_C_INCLUDES += \
    $(ANDROID_BUILD_TOP)/hardware/ti/omap3-compat/mm_isp/ipp/inc \
    $(ANDROID_BUILD_TOP)/hardware/ti/omap3-compat/mm_isp/capl/inc \

LOCAL_SHARED_LIBRARIES += \
    libcapl \
    libImagePipeline

LOCAL_CFLAGS += -DIMAGE_PROCESSING_PIPELINE

endif

LOCAL_MODULE:= libcamera
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

################################################

ifdef HARDWARE_OMX

include $(CLEAR_VARS)

LOCAL_SRC_FILES := JpegEncoderTest.cpp

LOCAL_C_INCLUDES := \
    $(TI_OMX_SYSTEM)/omx_core/inc \
    $(TI_OMX_IMAGE)/jpeg_enc/inc \
    $(ANDROID_BUILD_TOP)/external/libexif \

LOCAL_SHARED_LIBRARIES := libcamera

LOCAL_MODULE := JpegEncoderTest
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

endif

################################################

endif

