/*
 * ZeSense
 * Author: Marco Zavatta
 * <marco.zavatta@telecom-bretagne.eu>
 */

#include <jni.h>
#include "ze_coap_server_root.h"

#define COAP_SERVER 1

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

	return ze_server_root(env, thiz, actx);

}

/*
jint
Java_eu_tb_zesense_ZeSMThread_ze_1sm_1server_1entry__Landroid_content_Context_2
	(JNIEnv* env, jobject thiz, jobject actx) {

	return ze_sm_server_root(env, thiz, actx);

}*/
/*
jint
Java_eu_tb_zesense_ZeRTPThread_get_1command__Leu_tb_zesense_ZeRTPCommand_2
	(JNIEnv* env, jobject thiz, jobject command) {

	return get_rtp_buf_item(env, thiz, command);
}*/

