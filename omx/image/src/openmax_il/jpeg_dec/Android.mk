ifeq ($(BUILD_JPEG_DECODER),1)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
	src/OMX_JpegDec_Thread.c \
	src/OMX_JpegDec_Utils.c \
	src/OMX_JpegDecoder.c \

TOP ?= $(ANDROID_BUILD_TOP)
TI_OMX_TOP   ?= $(TOP)/hardware/ti/omx
TI_OMX_IMAGE ?= $(TI_OMX_TOP)/image/src/openmax_il
TI_OMX_SYSTEM?= $(TI_OMX_TOP)/system/src/openmax_il

TI_BRIDGE_TOP      ?= $(TOP)/hardware/ti/omap3-compat/dspbridge
TI_BRIDGE_INCLUDES ?= $(TI_BRIDGE_TOP)/libbridge/inc


TI_OMX_COMP_C_INCLUDES ?= \
	$(TI_OMX_SYSTEM)/lcml/inc \
	$(TI_OMX_SYSTEM)/common/inc \
	$(TI_BRIDGE_INCLUDES) \
	$(TOP)/frameworks/base/include/media/stagefright/openmax \

LOCAL_C_INCLUDES := \
	$(TI_OMX_COMP_C_INCLUDES) \
	$(TI_OMX_IMAGE)/jpeg_dec/inc \

TI_OMX_COMP_SHARED_LIBRARIES ?= libc libdl liblog

LOCAL_SHARED_LIBRARIES := $(TI_OMX_COMP_SHARED_LIBRARIES)

LOCAL_CFLAGS += $(TI_OMX_CFLAGS) -DOMAP_2430 -DOMAP_3430
LOCAL_CFLAGS += -DOMX_DEBUG=1

LOCAL_MODULE := libOMX.TI.JPEG.decoder
LOCAL_MODULE_TAGS := eng

include $(BUILD_SHARED_LIBRARY)
endif #BUILD_JPEG_DECODER

#########################################################

ifeq ($(BUILD_JPEG_DEC_TEST),1)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= tests/JPEGTest.c

TI_OMX_COMP_C_INCLUDES ?= \
	$(TI_OMX_SYSTEM)/lcml/inc \
	$(TI_OMX_SYSTEM)/common/inc \
	$(TI_BRIDGE_INCLUDES) \
	$(TOP)/frameworks/base/include/media/stagefright/openmax \

LOCAL_C_INCLUDES := \
	$(TI_OMX_COMP_C_INCLUDES) \
	$(TI_OMX_IMAGE)/jpeg_dec/inc \

TI_OMX_COMP_SHARED_LIBRARIES ?= libc libdl liblog
LOCAL_SHARED_LIBRARIES := $(TI_OMX_COMP_SHARED_LIBRARIES) libOMX.TI.JPEG.decoder

LOCAL_CFLAGS := -Wall -fpic -pipe -O0
LOCAL_CFLAGS += -DOMX_DEBUG=1

LOCAL_MODULE:= JPEGTest
LOCAL_MODULE_TAGS := eng

include $(BUILD_EXECUTABLE)
endif
