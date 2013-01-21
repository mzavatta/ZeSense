ASensorManager* sensorManager;
ASensorEventQueue* sensorEventQueue;
ALooper* looper;
const ASensor* accelerometerSensor;

#define RTP_FREQ 200


void add_stream(coap_resource_t *resource, coap_address_t *dest, int sensor, str query) {

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
		LL_SEARCH(streams, sub, destination, stream_equals_dest);
		if (sub) { //Stream with same destination present, delete the previous
			LL_DELETE(streams, sub);
		}
		LL_APPEND(streams, ns);
	}
}

int find_stream() {
	ze_streaming_state
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


int stream_equals_dest(ze_single_stream_t *elem, coap_address_t *dest) {
	if (coap_address_equals(element->dest, dest)  == 1) return 0;
	else return -1;
}


void ze_coap_streaming_thread() {

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
	ze_sensor_coap_payload pay;
	coap_pdu_t *pdu;

	while(1) {
		//is this blocking?
		if (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0) {

			if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
            	LOGI("accel: x=%f y=%f z=%f",
						event.acceleration.x, event.acceleration.y,
						event.acceleration.z);

				if (ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].stream != NULL) {
					//Build a PDU
					// TODO: reconsider whether this job should be done by notify
					typedef struct {
					  size_t max_size;			/**< allocated storage for options and data */
					  coap_hdr_t *hdr;
					  unsigned short length;	/* PDU length (including header, options, data)  */
					  coap_list_t *options;		/* parsed options */
					  unsigned char *data;		/* payload */
					} coap_pdu_t;

					pdu = coap_pdu_init()

					coap_add_data(pdu, unsigned int len, const unsigned char *data);

					//Timestamp according to RTP
					pay.wts = event.timestamp;
					pay.rtpts = ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].stream.last_rtpts
							+ (ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].stream.freq/RTP_FREQ);

					//Fire
					int rto = ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].stream.deadline;
					coap_address_t dst = ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].stream.dest;

					/**	 coap_notify(coap_resource_t *resource, unsigned int len, const unsigned char *data,
					 * 	coap_address_t *dst, int conf, int rto, int rtc, int max_age); */

					/* takes care of automatic things that go along with observe */

					// if the resource associated with the sensor hasn't been DELETED
					if (ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER] != NULL) {
						//coap_notify(pdu, dst, COAP_MESSAGE_NON, rto, NULL, NULL);
					}
					else {
						// clean the streams associated with this sensor!
					}
				}
				else {
					LOGW("got accelerometer sample but no streams present for it");
				}

			}

    	}
	}
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
