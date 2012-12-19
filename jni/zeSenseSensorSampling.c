/*
 * ZeSense
 * Author: Marco Zavatta
 * <marco.zavatta@telecom-bretagne.eu>
 */

#include <jni.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include <android/log.h>
#include <android/sensor.h>

// Logging macros
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "zeSenseSensorSampling", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "zeSenseSensorSampling", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "zeSenseSensorSampling", __VA_ARGS__))

/*
 * <source>/hardware/libhardware/include/hardware/sensors.h
#define SENSOR_TYPE_ACCELEROMETER       1
#define SENSOR_TYPE_MAGNETIC_FIELD      2
#define SENSOR_TYPE_ORIENTATION         3
#define SENSOR_TYPE_GYROSCOPE           4
#define SENSOR_TYPE_LIGHT               5
#define SENSOR_TYPE_PRESSURE            6
#define SENSOR_TYPE_TEMPERATURE         7   // deprecated
#define SENSOR_TYPE_PROXIMITY           8
#define SENSOR_TYPE_GRAVITY             9
#define SENSOR_TYPE_LINEAR_ACCELERATION 10
#define SENSOR_TYPE_ROTATION_VECTOR     11
#define SENSOR_TYPE_RELATIVE_HUMIDITY   12
#define SENSOR_TYPE_AMBIENT_TEMPERATURE 13

 * NDK mirrors in synch with above
enum {
    ASENSOR_TYPE_ACCELEROMETER      = 1,
    ASENSOR_TYPE_MAGNETIC_FIELD     = 2,
    ASENSOR_TYPE_GYROSCOPE          = 4,
    ASENSOR_TYPE_LIGHT              = 5,
    ASENSOR_TYPE_PROXIMITY          = 8
};
 */

// Definitions for missing NDK sensor types
#define ZESENSE_SENSOR_TYPE_ORIENTATION		3
#define ZESENSE_SENSOR_TYPE_PRESSURE		6

// Global experiment settings
#define EXPERIMENT_DURATION 5 //Seconds
//#define EXPERIMENT_MULTIPLE 1

// Accelerometer settings ASENSOR_TYPE_ACCELEROMETER
#define ACCEL_ON 1
#define ACCEL_HZ 20 //Hz
#define NUM_ACCEL_SAMPLES (ACCEL_HZ*EXPERIMENT_DURATION)

// Gyro settings ASENSOR_TYPE_GYROSCOPE
//#define GYRO_ON 1
#define GYRO_HZ 50 //Hz
#define NUM_GYRO_SAMPLES (GYRO_HZ*EXPERIMENT_DURATION)

// Light settings ASENSOR_TYPE_LIGHT
//#define LIGHT_ON 1
#define LIGHT_HZ 20 //Hz
#define NUM_LIGHT_SAMPLES (LIGHT_HZ*EXPERIMENT_DURATION)

// Magnetic sensor ASENSOR_TYPE_MAGNETIC_FIELD
//#define MAG_ON 1
#define MAG_HZ 20 //Hz
#define NUM_MAG_SAMPLES (MAG_HZ*EXPERIMENT_DURATION)

// Proximity sensor ASENSOR_TYPE_PROXIMITY
//#define PROX_ON 1
#define PROX_HZ 20 //Hz		//TODO: value broadcasted only when it changes!!
#define NUM_PROX_SAMPLES (PROX_HZ*EXPERIMENT_DURATION)

// Orientation sensor ZESENSE_SENSOR_TYPE_ORIENTATION
//#define ORIENT_ON 1
#define ORIENT_HZ 20 //Hz
#define NUM_ORIENT_SAMPLES (ORIENT_HZ*EXPERIMENT_DURATION)

// Pressure sensor ZESENSE_SENSOR_TYPE_PRESSURE
//#define PRES_ON 1
#define PRES_HZ 20 //Hz
#define NUM_PRES_SAMPLES (PRES_HZ*EXPERIMENT_DURATION)


struct zeSense_ASensorEvent {
	ASensorEvent event; //as it is fed into the event interface
	struct timespec collectionTimestamp; //when we collect it
};

struct zeSense_experiment {
	struct zeSense_ASensorEvent accelEvents[ACCEL_HZ*EXPERIMENT_DURATION];
	int64_t collectionDifferences[(ACCEL_HZ*EXPERIMENT_DURATION)-1];
	int64_t generationDifferences[(ACCEL_HZ*EXPERIMENT_DURATION)-1];
	int64_t collGenDifference[ACCEL_HZ*EXPERIMENT_DURATION];
};

int experimenting = 1; //

void Java_eu_tb_zesense_ZeSenseSensorService_zeSense_1SamplingNative(JNIEnv* env, jobject thiz) {

	LOGI("Hello from zs_SamplingNative");

	struct zeSense_experiment experiment;

	// Set up sensor sampling infrastructure:
	// the looper monitors a set of fds and feeds events to a sensor event queue
	// we only deal with the sensor event queue, it then handles the looper by itself
	// the sensor manager knows which fd corresponds to a given sensor
    ASensorManager* sensorManager;
    ASensorEventQueue* sensorEventQueue;
    ALooper* looper;

    // Prepare looper
    looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    LOGI("looper prepared");

    // Take the SensorManager (C++ singleton class)
    sensorManager = ASensorManager_getInstance();
    LOGI("got sensorManager");

    // Create event queue associated with that looper
    sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, 45, NULL, NULL);
    LOGI("got sensorEventQueue");

    //TODO: possibility of assigning a different looper-queue to each sensor in different threads!?
    //in this way we're assigning the same looper to many fds. It will probably poll them in a predefined order
    //instead of leaving this decision to the scheduler

#ifdef ACCEL_ON
    // Grab the sensor description
    const ASensor* accelerometerSensor;
    accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);

    // Start monitoring the sensor
    if (accelerometerSensor != NULL) {
    	LOGI("got accelerometer sensor");
        ASensorEventQueue_enableSensor(sensorEventQueue, accelerometerSensor);
        ASensorEventQueue_setEventRate(sensorEventQueue, accelerometerSensor, (1000L/ACCEL_HZ)*1000);
    }
    else {
    	LOGW("accelerometer sensor NULL");
    	exit(1);
    }
#endif

#ifdef GYRO_ON
    // Grab the sensor description
    const ASensor* gyroSensor;
    gyroSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_GYROSCOPE);

    // Start monitoring the sensor
    if (gyroSensor != NULL) {
    	LOGI("got gyro sensor");
        ASensorEventQueue_enableSensor(sensorEventQueue, gyroSensor);
        ASensorEventQueue_setEventRate(sensorEventQueue, gyroSensor, (1000L/GYRO_HZ)*1000);
    }
    else {
    	LOGW("gyro sensor NULL");
    	exit(1);
    }
#endif

#ifdef LIGHT_ON
    // Grab the sensor description
    const ASensor* lightSensor;
    lightSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_LIGHT);

    // Start monitoring the sensor
    if (lightSensor != NULL) {
    	LOGI("got light sensor");
        ASensorEventQueue_enableSensor(sensorEventQueue, lightSensor);
        ASensorEventQueue_setEventRate(sensorEventQueue, lightSensor, (1000L/LIGHT_HZ)*1000);
    }
    else {
    	LOGW("light sensor NULL");
    	exit(1);
    }
#endif

#ifdef MAG_ON
    // Grab the sensor description
    const ASensor* magSensor;
    magSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_MAGNETIC_FIELD);

    // Start monitoring the sensor
    if (magSensor != NULL) {
    	LOGI("got magnetic sensor");
        ASensorEventQueue_enableSensor(sensorEventQueue, magSensor);
        ASensorEventQueue_setEventRate(sensorEventQueue, magSensor, (1000L/MAG_HZ)*1000);
    }
    else {
    	LOGW("magnetic sensor NULL");
    	exit(1);
    }
#endif

#ifdef PROX_ON //TODO: value broadcasted only when it changes!!
    // Grab the sensor description
    const ASensor* proxSensor;
    proxSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_PROXIMITY);

    // Start monitoring the sensor
    if (proxSensor != NULL) {
    	LOGI("got proximity sensor");
        ASensorEventQueue_enableSensor(sensorEventQueue, proxSensor);
        ASensorEventQueue_setEventRate(sensorEventQueue, proxSensor, (1000L/PROX_HZ)*1000);
    }
    else {
    	LOGW("proximity sensor NULL");
    	exit(1);
    }
#endif

#ifdef PRES_ON
    // Grab the sensor description
    const ASensor* presSensor;
    presSensor = ASensorManager_getDefaultSensor(sensorManager, ZESENSE_SENSOR_TYPE_PRESSURE);

    // Start monitoring the sensor
    if (presSensor != NULL) {
    	LOGI("got pressure sensor");
        ASensorEventQueue_enableSensor(sensorEventQueue, presSensor);
        ASensorEventQueue_setEventRate(sensorEventQueue, presSensor, (1000L/PRES_HZ)*1000);
    }
    else {
    	LOGW("pressure sensor NULL");
    	exit(1);
    }
#endif

#ifndef EXPERIMENT_MULTIPLE
    int event_counter = 0;
    ASensorEvent event;
    struct timeval start_time, stop_time, diff;
    int64_t duration_time;
    double average_frequency;

    // Experiment start time
	if(gettimeofday(&start_time, NULL)!=0) LOGW("Failed start_time gettimeofday()");
	//LOGI("start %lu", start_time.tv_sec);
	//LOGI("rest %lu", start_time.tv_usec);

    while (event_counter < NUM_ACCEL_SAMPLES) {
    	if (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0) {
			if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
				LOGI("accelerometer: x=%f y=%f z=%f",
						event.acceleration.x, event.acceleration.y,
						event.acceleration.z);
			}
        	event_counter++;
    	}
    }

    LOGI("event_counter %d", event_counter);

    // Experiment end time
	if(gettimeofday(&stop_time, NULL)!=0) LOGW("Failed stop_time gettimeofday()");
	//LOGI("stop %lu", stop_time.tv_sec);
	//LOGI("rest %lu", stop_time.tv_usec);

    // Calculate duration and average frequency
	timeval_subtract(&diff, &stop_time, &start_time);
	//LOGI("duration %lu", diff.tv_sec);
	//LOGI("rest %lu", diff.tv_usec);

	duration_time = (diff.tv_sec*1000000LL)+diff.tv_usec; //microsecs
	average_frequency = (event_counter)/((double) duration_time/1000000LL);

	LOGI("Experiment lasted %lld, avg freq %g", duration_time, average_frequency);

#else // Use a timeout as long as EXPERIMENT_DURATION

    ASensorEvent event;
    while(experimenting) {
		if (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0) {

			LOGI("sensor event from sensor%d", event.sensor);

			if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
				LOGI("accelerometer: x=%f y=%f z=%f",
						event.acceleration.x, event.acceleration.y,
						event.acceleration.z);
			}

			if (event.type == ASENSOR_TYPE_GYROSCOPE) {
				LOGI("gyro: x=%f y=%f z=%f",
						event.vector.x, event.vector.y,
						event.vector.z);
			}

			if (event.type == ASENSOR_TYPE_LIGHT) {
				LOGI("light: l=%f", event.light);
			}

			if (event.type == ASENSOR_TYPE_MAGNETIC_FIELD) {
				LOGI("mag: x=%f y=%f z=%f",
						event.magnetic.x, event.magnetic.y,
						event.magnetic.z);
			}

			if (event.type == ASENSOR_TYPE_PROXIMITY) {
				LOGI("prox: p=%f", event.distance);
				//ASensorEventQueue_enableSensor(sensorEventQueue, proxSensor); //shouldn't be necessary
			}

			if (event.type == ZESENSE_SENSOR_TYPE_PRESSURE) {
				LOGI("pressure: p=%f", event.pressure);
			}
		}
    }
#endif
    //return (*env)->NewStringUTF(env, "Hello from JNI !");
}

#ifdef EXPERIMENT_MUTIPLE
void experiment_multi_timeout(int signum) {
	if (experimenting == 0)
		experimenting = 1;
	else {
		LOGW("timeout experiment multi fired, but not experimenting");
		exit(1);
	}
#endif

int timeval_subtract (result, x, y)	struct timeval *result, *x, *y; {
	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
	    y->tv_usec -= 1000000 * nsec;
	    y->tv_sec += nsec;
	}
	if (x->tv_usec - y->tv_usec > 1000000) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000;
	    y->tv_usec += 1000000 * nsec;
	    y->tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
	     tv_usec is certainly positive. */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}
