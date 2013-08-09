LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := knight

LOCAL_MODULE_FILENAME := libknight

LOCAL_SRC_FILES := knight/main.cpp \
                   ../../AppDelegate.cpp \
                   ../../Scene.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../

LOCAL_WHOLE_STATIC_LIBRARIES += cocos2dx_static

include $(BUILD_SHARED_LIBRARY)

$(call import-module,proj.android)
