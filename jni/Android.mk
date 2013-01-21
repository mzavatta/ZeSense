# ZeSense
# Author: Marco Zavatta
# <marco.zavatta@telecom-bretagne.eu>

# Before launching ndk-build
# set from the shell the NDK_MODULE_PATH variable to the needed locations e.g.
# $ export NDK_MODULE_PATH+=/home/telecombretagne/

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := ZeSense
LOCAL_SRC_FILES := ze_sensorsampling.c ze_coap_server_example.c
LOCAL_LDLIBS := -llog -landroid
LOCAL_STATIC_LIBRARIES := libcoap-3.0.0-android

include $(BUILD_SHARED_LIBRARY)

$(call import-module,libcoap-3.0.0-android)
