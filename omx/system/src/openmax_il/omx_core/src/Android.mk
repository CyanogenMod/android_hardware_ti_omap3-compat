ifeq ($(HARDWARE_OMX),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
	OMX_Core.c

LOCAL_C_INCLUDES += \
	$(TI_OMX_INCLUDES) \

LOCAL_SHARED_LIBRARIES := \
	libdl \
	liblog
	
LOCAL_CFLAGS := $(TI_OMX_CFLAGS)

LOCAL_MODULE:= libOMX_Core
LOCAL_MODULE_TAGS := eng

include $(BUILD_SHARED_LIBRARY)

endif
