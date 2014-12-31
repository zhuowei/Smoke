LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= main.c
LOCAL_MODULE := smoke
LOCAL_CFLAGS += -std=c99 -fPIE -pie
LOCAL_LDLIBS :=
LOCAL_LDFLAGS += -fPIE -pie
include $(BUILD_EXECUTABLE)
