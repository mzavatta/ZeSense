# ZeSense
# Author: Marco Zavatta
# <marco.zavatta@telecom-bretagne.eu>

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := ZeSenseNativeSensorSampling
LOCAL_SRC_FILES := zeSenseSensorSampling.c
LOCAL_LDLIBS := -llog -landroid

include $(BUILD_SHARED_LIBRARY)
