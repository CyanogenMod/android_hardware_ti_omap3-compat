LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        VidDecTest.c \
        MPEG4DecFunctions.c \
        MPEG2DecFunctions.c \
        H264DecFunctions.c \
        WMV9DecFunctions.c

LOCAL_C_INCLUDES := \
        $(TI_OMX_VIDEO)/video_decode/inc \
        hardware/ti/omap3-compat/liboverlay \
        $(TI_OMX_COMP_C_INCLUDES)

LOCAL_SHARED_LIBRARIES := $(TI_OMX_COMP_SHARED_LIBRARIES) \
        libOMX_Core

LOCAL_CFLAGS := $(TI_OMX_CFLAGS)

LOCAL_MODULE:= VidDecTest_common
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
