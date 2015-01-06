LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := model

LOCAL_C_INCLUDES := include/
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/source/*.cpp))

# force interpretation of code as c++:
LOCAL_CFLAGS += -x c++
# treat these files as c++:
LOCAL_CPP_EXTENSION := .mm .cpp .cc

#build static library
include $(BUILD_STATIC_LIBRARY)
