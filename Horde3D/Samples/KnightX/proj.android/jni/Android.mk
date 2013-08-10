LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := knight

LOCAL_MODULE_FILENAME := libknight

LOCAL_SRC_FILES := knight/main.cpp \
                   ../../AppDelegate.cpp \
                   ../../Scene.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../

LOCAL_WHOLE_STATIC_LIBRARIES += cocos2dx_static
LOCAL_WHOLE_STATIC_LIBRARIES += horde3d_static
LOCAL_WHOLE_STATIC_LIBRARIES += horde3d_terrain_static
LOCAL_WHOLE_STATIC_LIBRARIES += horde3d_utils_static

include $(BUILD_SHARED_LIBRARY)

$(call import-module,Horde3D/Samples/cocos2d-x-mini/proj.android)
$(call import-module,Horde3D/Source/Horde3DEngine/proj.android)
$(call import-module,Extensions/Terrain/Source/proj.android)
$(call import-module,Horde3D/Source/Horde3DUtils/proj.android)
