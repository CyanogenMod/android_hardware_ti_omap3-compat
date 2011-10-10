ifeq ($(HARDWARE_OMX),true)

TOP ?= $(ANDROID_BUILD_TOP)
TI_OMX_TOP    ?= $(TOP)/hardware/ti/omap3-compat/omx
TI_OMX_IMAGE  ?= $(TI_OMX_TOP)/image/src/openmax_il
TI_OMX_SYSTEM ?= $(TI_OMX_TOP)/system/src/openmax_il

TI_BRIDGE_TOP := $(TOP)/hardware/ti/omap3-compat/dspbridge
TI_BRIDGE_INCLUDES := $(TI_BRIDGE_TOP)/libbridge/inc

TI_OMX_COMP_C_INCLUDES ?= \
	$(TI_OMX_TOP)/lcml/inc \
	$(TI_OMX_TOP)/common/inc \
	$(TI_OMX_SYSTEM)/omx_core/inc \
	$(TI_BRIDGE_INCLUDES) \
	$(TOP)/frameworks/base/include/media/stagefright/openmax \

OMX_VENDOR_INCLUDES ?= $(TI_OMX_COMP_C_INCLUDES)

TI_OMX_COMP_SHARED_LIBRARIES ?= libc libdl liblog libOMX_Core

LOCAL_PATH:= $(call my-dir)

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
	$(TOP)/external/skia/include/core \
	$(TOP)/external/skia/include/images \
	$(OMX_VENDOR_INCLUDES)

LOCAL_CFLAGS += -fpic -fstrict-aliasing
LOCAL_CFLAGS += -DDEBUG_LOG
LOCAL_CFLAGS += -DOPTIMIZE

LOCAL_SRC_FILES+= \
	SkImageUtility.cpp \
	SkImageDecoder_libtijpeg.cpp \
	SkImageDecoder_libtijpeg_entry.cpp \
	SkImageEncoder_libtijpeg.cpp \
	SkImageEncoder_libtijpeg_entry.cpp

LOCAL_MODULE:= libskiahw
LOCAL_MODULE_TAGS:= optional

include $(BUILD_SHARED_LIBRARY)

endif #HARDWARE_OMX
