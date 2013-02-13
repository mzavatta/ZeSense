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

	/*_Leu_tb_zesense_ZeCoAPService_ZeCoAPThread_2*/
	/*
	LOGW("entry");

	//jobject jj = (*env)->NewWeakGlobalRef(env, thiz);
	//if (jj==NULL) LOGI("jj null");
	jclass servclass = (*env)->FindClass(env, "java/lang/Thread");
	//jclass servclassg = (*env)->NewGlobalRef(env, servclass);
	//if (servclassg == NULL) LOGI("servclassg null");
	jmethodID inted = (*env)->GetMethodID(env, servclass, "isInterrupted", "()Z");
	//jmethodID intedg = (*env)->NewGlobalRef(env, inted);
	int work = 1;
	while (work) {
		sleep(1);
		LOGI("Checking");
		jboolean status = (*env)->CallBooleanMethod(env, thiz, inted);
		if (status == JNI_TRUE) {
			LOGW("Interrupted");
			work = 0;
		}
	}
	return 1;
*/
	return ze_coap_server_root(env, thiz, actx);
}

