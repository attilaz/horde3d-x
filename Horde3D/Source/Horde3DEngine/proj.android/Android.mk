LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := horde3d_static

LOCAL_MODULE_FILENAME := libhorde3d

LOCAL_SRC_FILES := \
../egAnimatables.cpp \
../egAnimation.cpp \
../egCamera.cpp \
../egCom.cpp \
../egExtensions.cpp \
../egGeometry.cpp \
../egLight.cpp \
../egMain.cpp \
../egMaterial.cpp \
../egModel.cpp \
../egModules.cpp \
../egParticle.cpp \
../egPipeline.cpp \
../egPrimitives.cpp \
../egRenderer.cpp \
../egRendererBase.cpp \
../egResource.cpp \
../egScene.cpp \
../egSceneGraphRes.cpp \
../egShader.cpp \
../egTexture.cpp \
../utImage.cpp \
../utTexture.cpp \
../GLES2/egRendererBaseGLES2.cpp \
../GLES2/utOpenGLES2.cpp


LOCAL_EXPORT_C_INCLUDES :=  $(LOCAL_PATH)/../../../Bindings/C++

LOCAL_C_INCLUDES :=  $(LOCAL_PATH)/../../../Bindings/C++	\
					$(LOCAL_PATH)/../	\
					$(LOCAL_PATH)/../GLES2	\
					$(LOCAL_PATH)/../../Shared	\
					$(LOCAL_PATH)/../../../../Extensions

LOCAL_LDLIBS := -lGLESv2 \
                -llog \
                -lz

LOCAL_EXPORT_LDLIBS := -lGLESv2 \
                       -llog \
                       -lz

LOCAL_CFLAGS := -Wno-psabi
LOCAL_EXPORT_CFLAGS := -Wno-psabi

include $(BUILD_STATIC_LIBRARY)
