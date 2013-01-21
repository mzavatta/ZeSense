ASensorManager* sensorManager;
ASensorEventQueue* sensorEventQueue;
ALooper* looper;
const ASensor* accelerometerSensor;

#define RTP_FREQ 200


int add_stream(int sensor, coap_address_t *dest, int freq, int deadline) {

	ze_sensor_t *sensor_str;
	ze_single_stream_t *streams, *sub, ns;

	sensor_str = &ze_streaming_state[sensor];
	streams = ze_streaming_state[sensor].streams;

	/*
	 * interpret the query and create
	 * frequency
	 * deadline
	 * ..
	 */

	//Prepare stream item
	ns = (ze_single_stream *) malloc(sizeof(ze_single_stream_t));
	if (ns) {
		memset(ns, 0, sizeof(ze_single_stream_t));
	}
	ns->freq = frequency;
	ns->deadline = deadline;
	ns->dest = *dest;
	ns->last_rtpts = 100;
	//TODO: last_wts
	//TODO: randomize rtpts since it's its first assignment
	//TODO: frequency divider to be considered based on the current sampling frequency

	//If greater, reconsider the sensor maximum frequency
	if (frequency > sensor_str->freq) freq=frequency;

	if (streams == NULL) { //First stream of this sensor

		//Activate sampling
		android_sensor_activate(sensor, sensor_str->freq);

		//Insert stream in the list
		LL_APPEND(streams);
	}
	else {

		//Change sampling frequency
		android_sensor_changef(sensor, sensor_str->freq);

		//Append or "replace" previous stream if same destination
		LL_SEARCH(streams, sub, dest, stream_equals_dest);
		if (sub) { //Stream with same destination present, delete the previous
			LL_DELETE(streams, sub);
		}
		LL_APPEND(streams, ns);
	}

	return 0;
}


void stop_stream(int sensor, 	coap_address_t *dest) {

	ze_sensor_t *sensor_str;
	ze_single_stream_t *streams, *del;

	sensor_str = &ze_streaming_state[sensor];
	streams = ze_streaming_state[sensor].streams;

	//If the sensor has some streams
	if (streams != NULL) {

		// Find the one with the given destination
		LL_SEARCH(streams, sub, dest, stream_equals_dest);
		if (sub) { //Stream with same destination present, delete it
			LL_DELETE(streams, sub);
		}

		// If it was the last one
		if (streams == NULL) {
			android_sensor_turnoff(sensor);
		}
		else { // There are others streams for that sensor, need to reconsider the frequency
			find max
			sensor_str->freq = newmaxf;
			android_sensor_changef(ASENSOR_TYPE_ACCELEROMETER, newmaxf);
		}
	}
	else {
		LOGW("something went wrong, asked to stop stream but no streams are active");
	}
}

int stream_equals_dest(ze_single_stream_t *elem, coap_address_t *dest) {
	if (coap_address_equals(element->dest, dest)  == 1) return 0;
	else return -1;
}


int android_sensor_changef(int sensor, int freq) {

	if (sensor == ASENSOR_TYPE_ACCELEROMETER) {
		if (accelerometerSensor != NULL)
			ASensorEventQueue_setEventRate(sensorEventQueue, accelerometerSensor, freq);
		else {
			LOGW("accelerometer sensor NULL");
			exit(1);
		}
	}

}

int android_sensor_activate(int sensor, int freq) {

	if (sensor == ASENSOR_TYPE_ACCELEROMETER) {
		// Grab the sensor description
		accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);

		// Start monitoring the sensor
		if (accelerometerSensor != NULL) {
			LOGI("got accelerometer sensor");
			ASensorEventQueue_enableSensor(sensorEventQueue, accelerometerSensor);
			ASensorEventQueue_setEventRate(sensorEventQueue, accelerometerSensor, freq);
		}
		else {
			LOGW("accelerometer sensor NULL");
			exit(1);
		}
	}

}

int android_sensor_turnoff(int sensor) {

	if (sensor == ASENSOR_TYPE_ACCELEROMETER) {
		// Grab the sensor description
		accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);

		// Start monitoring the sensor
		if (accelerometerSensor != NULL) {
			LOGI("got accelerometer sensor");
			ASensorEventQueue_enableSensor(sensorEventQueue, accelerometerSensor);
			ASensorEventQueue_setEventRate(sensorEventQueue, accelerometerSensor, freq);
		}
		else {
			LOGW("accelerometer sensor NULL");
			exit(1);
		}
	}

}



void ze_coap_streaming_thread(coap_context_t *context) {

	// Hello and current time and date
	LOGI("Hello from zs_SamplingNative");
	time_t lt;
	lt = time(NULL);

	// File handle for logging
	FILE *logfd;

    // Prepare looper
    looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    LOGI("looper prepared");

    // Take the SensorManager (C++ singleton class)
    sensorManager = ASensorManager_getInstance();
    LOGI("got sensorManager");

    // Create event queue associated with that looper
    sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, 45, NULL, NULL);
    LOGI("got sensorEventQueue");

    // Open log file
	char *logpath = LOGPATH;
	logfd = fopen(logpath,"ab");
	if(logfd == NULL) {
		LOGW("unable to open %s", logpath);
		exit(1);
	}
	else LOGI("success opening %s", logpath);

	// Log experiment start time
	if (fputs(ctime(&lt), logfd)<0) LOGW("write failed");

	ASensorEvent event;
	ze_sensor_coap_payload *payload;
	int payload_length;
	coap_address_t dst;
	str uri;
	int rto, rtc, max_age;

	payload = malloc()

	while(1) {
		//is this blocking?
		if (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0) {

			if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
            	LOGI("accel: x=%f y=%f z=%f",
						event.acceleration.x, event.acceleration.y,
						event.acceleration.z);

            	//Check if we have streams for that sensor
				if (ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].stream != NULL) {
					//Fot the moment only one observer possible for each sensor

					//Prepare payload
					payload->wts = event.timestamp;
					payload->rtpts = ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].stream.last_rtpts
							+ (ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].stream.freq/RTP_FREQ);
					payload->debugpayload = 123;
					payload_length = sizeof(ze_sensor_coap_payload);

					//Fire
					uri = ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].uri;
					dst = ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].uri.stream.dest;
					rto = ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].stream.deadline;

					//Attention that this goes directly to the socket for the moment..
					coap_notify(context, uri, dst, &payload, payload_length, COAP_MESSAGE_NON, rto, NULL, NULL, NULL);
				}
				else {
					LOGW("got accelerometer sample but no streams present for it");
				}
			}
    	}
	}
}

int get_single_sample(int sensor, unsigned char *data) {

	data = (ze_sensor_coap_payload*) malloc(sizeof(ze_sensor_coap_payload));

	event = ze_streaming_state[sensor].event;
	data->wts = event.timestamp;
	data->rtpts = 100;
	data->debugpayload = 123;

	return sizeof(ze_sensor_coap_payload);
}


/* Thread
while 1
- scorre la lista per vedere se attivare dei sensori e a che frequenza
NO! questo lo facciamo fare al server core! tanto la rate di richieste non può essere altissima
anche se il thread che riceve le richieste è un pò più caricato non fa niente!

- ad ogni sample per ogni stream su quel sensore guarda se la f è adatta e se si lo invia usando notify()
(in notify non need to care whether a minimum number of CON are being sent, if we want in here we notify
all NONC and the notify will add the CON for us)
Reliability policy.. per ora tutti NON, poi magari facciamo tutti CON, e poi magari la parametrizziamo...
 */
