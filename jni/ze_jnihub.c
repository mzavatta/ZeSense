/*
 * ZeSense
 * Author: Marco Zavatta
 * <marco.zavatta@telecom-bretagne.eu>
 */

#include <jni.h>
#include "ze_coap_server_root.h"

//void Java_eu_tb_zesense_ZeJNIHub_zeSense_1SamplingNative(JNIEnv* env, jobject thiz) {

/* Package eu/tb/zesense declared in class ZeCOAPThread method name ze_coap_server_entry
 * parameter ZeGPSManager whose signature is Leu/tb/zesense/ZeGPSManager;
 */

/*
jint
Java_eu_tb_zesense_ZeCoAPThread_ze_1coap_1server_1entry__Leu_tb_zesense_ZeGPSManager_2
	(JNIEnv* env, jobject thiz, jobject gpsManager) {
	*/

jint
Java_eu_tb_zesense_ZeCoAPThread_ze_1coap_1server_1entry__Landroid_content_Context_2
	(JNIEnv* env, jobject thiz, jobject actx) {

	return ze_coap_server_root(env, thiz, actx);
}

