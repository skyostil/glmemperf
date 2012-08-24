LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := glmemperf
LOCAL_CXXFLAGS := -DSUPPORT_ANDROID -DPACKAGE_VERSION="\"1.0\"" -DPREFIX="\".\""
LOCAL_LDLIBS := -landroid -llog -lGLESv2 -lEGL
LOCAL_STATIC_LIBRARIES := android_native_app_glue

LOCAL_CPP_FEATURES += exceptions

LOCAL_SRC_FILES := \
        ../blittest.cpp \
        ../cleartest.cpp \
        ../cpuinterleavingtest.cpp \
        ../fboblittest.cpp \
        ../runner_android.cpp \
        ../shaderblittest.cpp \
        ../test.cpp \
        ../util.cpp

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
