LOCAL_PATH:=$(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:=backmodule
LOCAL_SRC_FILES:=second.c
LOCAL_LDLIBS := -llog
#LOCAL_LDLIB := -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY)

