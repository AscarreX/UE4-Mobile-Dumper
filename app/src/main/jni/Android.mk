LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := Dumper

LOCAL_SRC_FILES := Main.cpp \
ElfFixer/fix.cpp

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_CFLAGS += -Os -ffunction-sections -fdata-sections -fvisibility=hidden -fvisibility-inlines-hidden
LOCAL_CPPFLAGS += -Os -ffunction-sections -fdata-sections -fvisibility=hidden -fvisibility-inlines-hidden
LOCAL_LDFLAGS += -Wl,--gc-sections

LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)