LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
	src/OMX_VideoEnc_Thread.c \
	src/OMX_VideoEnc_Utils.c \
	src/OMX_VideoEncoder.c

LOCAL_C_INCLUDES := $(TI_OMX_COMP_C_INCLUDES) \
	$(TI_OMX_VIDEO)/video_encode/inc \

LOCAL_SHARED_LIBRARIES := $(TI_OMX_COMP_SHARED_LIBRARIES)

LOCAL_CFLAGS := $(TI_OMX_CFLAGS) -DOMAP_2430

ifeq ($(TARGET_BOOTLOADER_BOARD_NAME),jordan)
LOCAL_CFLAGS += -DINCREASE_WVGA_BUFSIZE
endif

LOCAL_MODULE:= libOMX.TI.Video.encoder
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
