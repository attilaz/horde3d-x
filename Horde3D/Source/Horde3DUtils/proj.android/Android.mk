LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := horde3d_utils_static

LOCAL_MODULE_FILENAME := libhorde3d_utils

LOCAL_SRC_FILES := \
../main.cpp
 
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../ \
                    $(LOCAL_PATH)/../../../Bindings/C++ \
                    $(LOCAL_PATH)/../../Horde3DEngine \
                    $(LOCAL_PATH)/../../Shared

LOCAL_LDLIBS := -lGLESv2 \
                -llog \
                -lz

LOCAL_EXPORT_LDLIBS := -lGLESv2 \
                       -llog \
                       -lz

# define the macro to compile through support/zip_support/ioapi.c
LOCAL_CFLAGS := -Wno-psabi -DUSE_FILE32API
LOCAL_EXPORT_CFLAGS := -Wno-psabi -DUSE_FILE32API

include $(BUILD_STATIC_LIBRARY)
