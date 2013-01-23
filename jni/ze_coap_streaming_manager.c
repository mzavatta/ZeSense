

int sm_bind_source(stream_context_t *mngr, int sensor_id, str uri) {

	CHECK_OUT_RANGE(sensor_id);

	mngr->sensor[sensor_id].uri = uri;
	return 0;
}

int sm_start_stream(stream_context_t *mngr, int sensor_id, coap_address_t dest, int freq) {

	CHECK_OUT_RANGE(sensor_id);

	//FIXME: for the moment we support only one stream for
	//it saves some time on the linked list management

	ze_sensor_t *sensor = &(mngr->sensors[sensor_id]);
	ze_stream_t *streams = sensor->streams;
	ze_stream_t *sub, *newstream;

	//Prepare stream item
	newstream = (ze_stream_t *) malloc(sizeof(ze_stream_t));
	if (newstream == NULL) {
		LOGW("new stream malloc failed");
		return SM_ERROR;
	}
	memset(newstream, 0, sizeof(ze_stream_t));
	ns->next = NULL;
	ns->freq = freq;
	ns->dest = dest;
	ns->last_rtpts = SM_RTPTS_START;
	//TODO: last_wts
	//TODO: randomize rtpts since it's its first assignment
	//TODO: frequency divider to be considered based on the current sampling frequency

	if (streams == NULL) { //First stream of this sensor

		//Activate sampling
		android_sensor_activate(mngr, sensor_id, sensor->freq);

		//Insert stream in the list
		//LL_APPEND(streams);
		streams = newstream;

		return 0;
	}
	else {

		//For the moment, we have only one, replace anyhow
		streams = newstream;
		sensor->freq = freq;
		android_sensor_changef(mngr, sensor_id, sensor->freq);
		return SM_STREAM_REPLACED;

		/*
		//Try to find the
		//Reconsider the sensor maximum frequency
		if (freq > (sensor->freq)) {
			android_sensor_changef(sensor, sensor_str->freq);
			sensor->freq = freq;
		}
		//Append or "replace" previous stream if same destination
		LL_SEARCH(streams, sub, dest, stream_equals_dest);
		if (sub) { //Stream with same destination present, delete the previous
			LL_DELETE(streams, sub);
		}
		LL_APPEND(streams, ns);
		*/
	}

	return SM_ERROR;
}


int sm_stop_stream(stream_context_t *mngr, int sensor_id, coap_address_t dest) {

	CHECK_OUT_RANGE(sensor_id);

	ze_sensor_t *sensor = &(mngr->sensors[sensor_id]);
	ze_stream_t *streams = sensor->streams;
	ze_stream_t *del;

	//If the sensor has some streams
	if (streams != NULL) {

		//For the moment, there can be only one, delete and shut down
		free(streams);
		streams = NULL;
		android_sensor_turnoff(mngr, sensor_id);

		/*
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
		*/

		return 0;
	}
	LOGW("something went wrong, asked to stop stream but no streams are active");
	return SM_ERROR;
}

/**
 * Checks if a stream from @p sensor_id to destination @p dest
 * is currently running.
 *
 * @param mngr		The Streaming Manager context
 * @param sensor_id	The sensor source of data
 * @param dest		The IP/port coordinates of the destination
 *
 * @return Zero if positive answer, @c SM_NEGATIVE otherwise
 * @c SM_OUT_RANGE if @p sensor_id is out of bound
 */
int sm_is_streaming(stream_context_t *mngr, int sensor_id, coap_address_t dest) {

	CHECK_OUT_RANGE(sensor_id);

	//For the moment, we have only one to check
	if ( coap_address_equals( &(mngr->sensors[sensor_id].streams.dest), &dest ) )
		return 0;
	else
		return SM_NEGATIVE;

	//In the future need to scroll streams list and if I find an element with
	//same destination, return 0, else SM_NEGATIVE
}


int stream_equals_dest(ze_single_stream_t *elem, coap_address_t *dest) {
	if (coap_address_equals(element->dest, dest)  == 1) return 0;
	else return -1;
}


int android_sensor_activate(stream_context_t *mngr, int sensor, int freq) {

	CHECK_OUT_RANGE(sensor);

	//Grab reference from Android
	mngr->sensors[sensor].android_sensor_handle =
			ASensorManager_getDefaultSensor(mngr->sensorManager, sensor);

	//Enable it
	if (mngr->sensors[sensor].android_sensor_handle != NULL) {
		ASensorEventQueue_enableSensor(mngr->sensorEventQueue,
				mngr->sensors[sensor].android_sensor_handle);
		ASensorEventQueue_setEventRate(mngr->sensorEventQueue,
				mngr->sensors[sensor].android_sensor_handle, freq);
	}
	else {
		LOGW("cannot get sensor %d", sensor);
		return SM_ERROR;
	}
	return 0;
}

int android_sensor_changef(stream_context_t *mngr, int sensor, int freq) {

	CHECK_OUT_RANGE(sensor);
	if (mngr->sensors[sensor].android_sensor_handle == NULL) {
		LOGW("sensor not initialized in Android");
		return SM_ERROR;
	}
	ASensorEventQueue_setEventRate(mngr->sensorEventQueue,
			mngr->sensors[sensor].android_sensor_handle, freq);
	return 0;
}


int android_sensor_turnoff(stream_context_t *mngr, int sensor) {

	CHECK_OUT_RANGE(sensor);
	if (mngr->sensors[sensor].android_sensor_handle == NULL) {
		LOGW("sensor not initialized in Android");
		return SM_ERROR;
	}
	ASensorEventQueue_disableSensor(mngr->sensorEventQueue,
			mngr->sensors[sensor].android_sensor_handle);
	return 0;
}



void ze_coap_streaming_thread(stream_context_t *mngr, ze_request_buf_t *smreqbuf,
		ze_sample_cache_t *cache, notbuf) {

	// Hello and current time and date
	LOGI("Hello from Streaming Manager Thread");
	time_t lt;
	lt = time(NULL);

    // Prepare looper
    mngr->looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    LOGI("looper prepared");

    // Take the SensorManager (C++ singleton class)
    mngr->sensorManager = ASensorManager_getInstance();
    LOGI("got sensorManager");

    // Create event queue associated with that looper //XXX !!! wtf is 45 ??
    mngr->sensorEventQueue =
    		ASensorManager_createEventQueue(sensorManager, looper, 45, NULL, NULL);
    LOGI("got sensorEventQueue");


	ASensorEvent event;
	ze_payload_container_t pyl;
	pyl.pyl->data = &event;

	coap_address_t dst;
	str uri;
	int rto, rtc, max_age;

	ze_request_t request;

	while(1) {


		request = get_req_buf_item(smreqbuf);

		if (request.rtype == SM_REQ_START) {
			sm_start_stream(mngr, request.sensor, request.dest, request.freq);
		}
		else if (request.rtype == SM_REQ_STOP) {
			sm_start_stream(mngr, request.sensor, request.dest);
		}
		//HOW TO IMPLEMENT THE IS STREAMING? IT'S GOTTA GO IN SHARED MEMoRY THAT, TOO..
		//WE'LL SLAP A BIG LOCK ON IT FOR THE MOMENT.. BTW, DO WE REALLY NEED IT?

		//DO A TWO THREAD IMPLEMENTATION
		//WHERE THE ROLE OF THE RECEIVER IS VERY LIMITED (DISPATCH, AND IF IT'S A SENSOR REQUEST
		//OF ANY KIND observe OR not.
		//IF IT'S A ONE-SHOT THEN REGISTER AN ASYNCH TRANSACTION, PASS IT TO THE STREAMING MANAGER
		//WHO RELAYS IT TO THE SENDER, WHO CLEARS THE ASYNCH TRANSACTION
		//IMPLEMENT A GENERIC INTERFACE AT THE SENDER TO SEND NOT ONLY NOTIFICATIONS BUT ALSO
		//ONE SHOT REQUESTS THAT ASK FOR SENSORS. TRANSFER THE CACHE BACK TO THE MAIN STATUS ARRAY
		//OF THE STREAMING MANAGER

		//is this blocking?
		if (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0) {

			if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
            	LOGI("accel: x=%f y=%f z=%f",
						event.acceleration.x, event.acceleration.y,
						event.acceleration.z);

            	//Update cache
            	//If the streams are disabled,

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
					dst = ze_streaming_state[ASENSOR_TYPE_ACCELEROMETER].stream.dest;
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


/* Thread
while 1
- SI scorre la lista per vedere se attivare dei sensori e a che frequenza
NO, e invece IS! questo lo facciamo fare al server core! tanto la rate di richieste non può essere altissima
anche se il thread che riceve le richieste è un pò più caricato non fa niente!

- pick some from the request queue
(using get_req_buf_item), do the actions requested

- even if we don't have a stream for that sample, update the last known value in the sample cache

- ad ogni sample per ogni stream su quel sensore guarda se la f è adatta e se si lo invia usando notify()
(in notify non need to care whether a minimum number of CON are being sent, if we want in here we notify
all NONC and the notify will add the CON for us)
Reliability policy.. per ora tutti NON, poi magari facciamo tutti CON, e poi magari la parametrizziamo...
 */
