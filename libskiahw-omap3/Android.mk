ifeq ($(HARDWARE_OMX),true)

TI_OMAP3_TOP  := $(ANDROID_BUILD_TOP)/hardware/ti/omap3-compat
TI_OMX_TOP    := $(TI_OMAP3_TOP)/omx
TI_OMX_IMAGE  := $(TI_OMX_TOP)/image/src/openmax_il
TI_OMX_SYSTEM := $(TI_OMX_TOP)/system/src/openmax_il

TI_BRIDGE_TOP := $(ANDROID_BUILD_TOP)/hardware/ti/omap3-compat/dspbridge
TI_BRIDGE_INCLUDES := $(TI_BRIDGE_TOP)/libbridge/inc

TI_OMX_COMP_C_INCLUDES ?= \
	$(TI_OMX_TOP)/lcml/inc \
	$(TI_OMX_TOP)/common/inc \
	$(TI_OMX_SYSTEM)/omx_core/inc \
	$(TI_BRIDGE_INCLUDES) \
	$(ANDROID_BUILD_TOP)/frameworks/base/include/media/stagefright/openmax \

OMX_VENDOR_INCLUDES ?= $(TI_OMX_COMP_C_INCLUDES)

TI_OMX_COMP_SHARED_LIBRARIES ?= libc libdl liblog libOMX_Core

LOCAL_PATH:= $(call my-dir)

################################################

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

# do not prelink
LOCAL_PRELINK_MODULE := false

LOCAL_REQUIRED_MODULES := libskia

LOCAL_SHARED_LIBRARIES := $(TI_OMX_COMP_SHARED_LIBRARIES) \
	libskia \
	libutils \
	libcutils

LOCAL_C_INCLUDES += \
	$(ANDROID_BUILD_TOP)/external/skia/include/core \
	$(ANDROID_BUILD_TOP)/external/skia/include/images \
	$(OMX_VENDOR_INCLUDES)

LOCAL_CFLAGS += -fpic -fstrict-aliasing
LOCAL_CFLAGS += -DDEBUG_LOG
LOCAL_CFLAGS += -DOPTIMIZE

ifeq ($(TARGET_USE_OMX_RECOVERY),true)
LOCAL_CFLAGS += -DMOTO_FORCE_RECOVERY
endif

LOCAL_SRC_FILES+= \
	SkImageUtility.cpp \
	SkImageDecoder_libtijpeg.cpp \
	SkImageDecoder_libtijpeg_entry.cpp \
	SkImageEncoder_libtijpeg.cpp \
	SkImageEncoder_libtijpeg_entry.cpp

LOCAL_MODULE:= libskiahw
LOCAL_MODULE_TAGS:= optional

include $(BUILD_SHARED_LIBRARY)

################################################

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := libskia

LOCAL_WHOLE_STATIC_LIBRARIES := libc_common

LOCAL_SRC_FILES := SkLibTiJpeg_Test.cpp

LOCAL_MODULE := SkLibTiJpeg_Test
LOCAL_MODULE_TAGS:= optional

LOCAL_C_INCLUDES += \
    $(ANDROID_BUILD_TOP)/external/skia/include/images \
    $(ANDROID_BUILD_TOP)/external/skia/include/core \
    $(ANDROID_BUILD_TOP)/bionic/libc/bionic

LOCAL_C_INCLUDES += \
    $(TI_OMAP3_TOP)/libskiahw-omap3 \
    $(TI_OMX_SYSTEM)/omx_core/inc

LOCAL_SHARED_LIBRARIES := \
    libskia \
    libskiahw \
    libcutils

include $(BUILD_EXECUTABLE)

endif #HARDWARE_OMX
