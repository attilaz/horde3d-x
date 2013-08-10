LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := horde3d_terrain_static

LOCAL_MODULE_FILENAME := libhorde3d_terrain

LOCAL_SRC_FILES := \
../terrain.cpp \
../extension.cpp

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/..

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../ \
                    $(LOCAL_PATH)/../../../../Horde3D/Source/Horde3DEngine \
                    $(LOCAL_PATH)/../../../../Horde3D/Source/Shared

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
