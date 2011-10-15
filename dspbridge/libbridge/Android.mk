LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= \
	DSPManager.c \
	DSPProcessor.c \
	DSPProcessor_OEM.c \
	DSPNode.c \
	DSPStrm.c \
	perfutils.c \
	dsptrap.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/inc	

LOCAL_CFLAGS += -pipe -fomit-frame-pointer -Wall  -Wno-trigraphs -Werror-implicit-function-declaration  -fno-strict-aliasing -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -msoft-float -Uarm -DMODULE -D__LINUX_ARM_ARCH__=7  -fno-common -DLINUX -DOMAP_3430 -fpic

LOCAL_MODULE:= libbridge
LOCAL_MODULE_TAGS := optional

# Required for Motorola Defy, Cliq2 & DroidX Codecs
# kernel/arch/arm/plat-omap/include/dspbridge/wcdioctl.h
ifeq ($(TARGET_USE_OMX_RECOVERY),true)
TI_OMX_CFLAGS += -DMOTO_FORCE_RECOVERY
endif

include $(BUILD_SHARED_LIBRARY)

