/*
 * ZeSense Streaming Manager
 * -- core streaming module
 *
 * Marco Zavatta
 * <marco.zavatta@telecom-bretagne.eu>
 * <marco.zavatta@mail.polimi.it>
 */

int sm_bind_source(stream_context_t *mngr, int sensor_id, str uri) {

	CHECK_OUT_RANGE(sensor_id);

	mngr->sensor[sensor_id].uri = uri;
	return 0;
}

ze_stream_t *sm_start_stream(stream_context_t *mngr, int sensor_id, coap_ticket_t reg, int freq) {

	CHECK_OUT_RANGE(sensor_id);

	ze_stream_t *sub, *newstream;

	newstream = sm_new_stream(reg, freq);

	//TODO: last_wts
	//TODO: randomize rtpts since it's its first assignment
	//TODO: frequency divider to be considered based on the current sampling frequency


	if ( mngr->sensors[sensor_id].android_handle == NULL ) {
		/* Sensor is not active, activate in any case
		 * Put a check anyway, if a sensor is not active
		 * the streams should be empty
		 */
		if (mngr->sensors[sensor_id].sensors != NULL)
			LOGW("streaming manager inconsistent state");

		android_sensor_activate(mngr, sensor_id, freq);
	}

	/* Sensor is already active.
	 * Re-evaluate its frequency based on the new request
	 */
	if (freq > mngr->sensors[sensor_id].freq) {
		mngr->sensors[sensor_id].freq = freq;
		android_sensor_changef(mngr, sensor_id, freq);
	}

	/* Replace if there is the same stream already
	 * if not just insert
	 */
	sub = sm_find_stream(mngr, sensor_id, reg);
	if (sub != NULL) {
		LL_DELETE(mngr->sensors[sensor_id].streams, sub);
	LL_APPEND(mngr->sensors[sensor_id].streams, newstream);

	return newstream;
}

/* best would be to call the stuff with as parameter coap_asynch_checkout(res) */
int sm_stop_stream(stream_context_t *mngr, int sensor_id, coap_ticket_t reg) {

	CHECK_OUT_RANGE(sensor_id);

	ze_stream_t *del;
	ze_stream_t *temp;

	del = sm_find_stream(mngr, sensor_id, reg);

	if (del == NULL) {
		/* stream not found.. its very strange but anyway */
		LOGW("inconsistent state in streaming manager, asked to stop"
				"stream but no streams are active");
		return SM_ERROR;
	}

	/* delete it from this list */
	LL_DELETE(mngr->sensors[sensor_id].streams, del);

	/* turn off the sensor if it was the last one,
	 * otherwise reconsider the output frequency of the sensor
	 * if we've taken out the highest one */
	if (mngr->sensors[sensor_id].streams == NULL) {
		android_sensor_turnoff(mngr, sensor_id);
		mngr->sensors[sensor_id].freq = 0;
	}
	else {
		/* reconsider the maximum frequency, maybe we just stopped
		 * the stream with the highest one.
		 * just take the maximum of those left
		 */
		temp = mngr->sensors[sensor_id].streams;
		mngr->sensors[sensor_id].freq = temp->freq;
		while (temp != NULL) {
			if (temp->freq > mngr->sensors[sensor_id].freq)
				mngr->sensors[sensor_id].freq = temp->freq;
			temp = temp->next;
		}
		android_sensor_changef(mngr, sensor_id, mngr->sensors[sensor_id].freq);
	}

	return 0;
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

ze_stream_t *sm_find_stream(stream_context_t *mngr, int sensor_id, coap_ticket_t reg) {

	ze_stream_t *temp = mngr->sensors[sensor_id].streams;
	while (temp != NULL) {
		if (temp->reg == reg) break;
		temp = temp->next;
	}
	return temp;
}

ze_stream_t *sm_new_stream(coap_ticket_t reg, int freq) {

	ze_stream_t new;
	new = (ze_stream_t *) malloc(sizeof(ze_stream_t));
	if (new == NULL) {
		LOGW("new stream malloc failed");
		return NULL;
	}
	memset(new, 0, sizeof(ze_stream_t));

	new->next = NULL;
	new->reg = reg;
	new->freq = freq;
	new->last_rtpts = SM_RTPTS_START;

	return new;
}



int stream_equals_dest(ze_single_stream_t *elem, coap_address_t *dest) {
	if (coap_address_equals(element->dest, dest)  == 1) return 0;
	else return -1;
}


int android_sensor_activate(stream_context_t *mngr, int sensor, int freq) {

	CHECK_OUT_RANGE(sensor);

	//Grab reference from Android
	mngr->sensors[sensor].android_handle =
			ASensorManager_getDefaultSensor(mngr->sensorManager, sensor);

	//Enable it
	if (mngr->sensors[sensor].android_handle != NULL) {
		ASensorEventQueue_enableSensor(mngr->sensorEventQueue,
				mngr->sensors[sensor].android_handle);
		ASensorEventQueue_setEventRate(mngr->sensorEventQueue,
				mngr->sensors[sensor].android_handle, freq);
	}
	else {
		LOGW("cannot get sensor %d", sensor);
		return SM_ERROR;
	}
	return 0;
}

int android_sensor_changef(stream_context_t *mngr, int sensor, int freq) {

	CHECK_OUT_RANGE(sensor);
	if (mngr->sensors[sensor].android_handle == NULL) {
		LOGW("sensor not initialized in Android");
		return SM_ERROR;
	}
	ASensorEventQueue_setEventRate(mngr->sensorEventQueue,
			mngr->sensors[sensor].android_handle, freq);
	return 0;
}


int android_sensor_turnoff(stream_context_t *mngr, int sensor) {

	CHECK_OUT_RANGE(sensor);
	if (mngr->sensors[sensor].android_handle == NULL) {
		LOGW("sensor not initialized in Android");
		return SM_ERROR;
	}
	ASensorEventQueue_disableSensor(mngr->sensorEventQueue,
			mngr->sensors[sensor].android_handle);
	mngr->sensors[sensor].android_handle = NULL;
	return 0;
}


int sm_new_oneshot(stream_context_t *mngr, int sensor_id, coap_address_t dest,
		int tknlen, unsigned char *tkn) {

	CHECK_OUT_RANGE(sensor_id);

	ze_oneshot_t *temp = malloc(sizeof(ze_oneshot_t)+tknlen);
	if (temp == NULL) {
		LOGW("malloc failed");
		return SM_ERROR;
	}
	temp->dest = dest;
	temp->tknlen = tknlen;
	temp->tkn = tkn;

	LL_APPEND(mngr.sensors[sensor_id]->oneshots, temp);

	return 0;
}

int sm_del_oneshot(stream_context_t *mngr, int sensor_id, coap_address_t dest,
		int tknlen, unsigned char *tkn) {



}


stream_context_t *get_streaming_manager(coap_context_t  *cctx) {

	stream_context_t *temp;

	temp = malloc(sizeof(stream_context_t));
	if (temp == NULL) {
		LOGW("cannot allocate streaming manager");
		return NULL;
	}

	memset(temp, 0, sizeof(stream_context_t));

	temp->server = cctx;

	temp->sensorManager = NULL;
	temp->sensorEventQueue = NULL;
	temp->looper = NULL;

	return temp;
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

	ze_payload_t *pyl;

	int rto, rtc, max_age;

	ze_sm_request_t sm_req;
	ze_coap_request_t server_req;

	/* To control the time spent on streaming
	 * wrt the time spent on serving requests
	 */
	int queuecount = 0;

	while(1) {

		// See if there is a request
		sm_req.rtype = SM_REQ_INVALID;
		sm_req = get_req_buf_item(smreqbuf);

		if (sm_req.rtype == SM_REQ_START) {
			sm_start_stream(mngr, sm_req.sensor, sm_req.reg, sm_req.freq);
		}
		else if (sm_req.rtype == SM_REQ_STOP) {
			sm_stop_stream(mngr, sm_req.sensor, sm_req.reg);


		}
		else if (sm_req.rtype == SM_REQ_ONESHOT) {

			if (mngr.sensors[sm_req.sensor].android_handle != NULL) {

				//Sensor is active, suppose cache is fresh
				event = mngr.sensors[sm_req.sensor].event_cache;

				//Take sample from cache and form payload
				pyl = malloc(sizeof(ze_payload_t));
				if (pyl == NULL)
					LOGW("malloc failed!");
				pyl->length = sizeof(ASensorEvent);
				pyl->data = malloc(pyl.length);
				if (pyl->data == NULL)
					LOGW("malloc failed!");
				*(pyl->data) = event;
				pyl->wts = event.timestamp;
				pyl->rtpts = 0;

				//Mirror the received request in the sender's interface
				//attaching the payload
				put_req_buf_item(notbuf, COAP_SEND_ASYNCH,
						mngr->sensors[sm_req.sensor].uri, sm_req.dest, COAP_MESSAGE_NON,
						sm_req.tknlen, sm_req.tkn, pyl);

				//do not free the pyl because it is referenced by the notqueue now!
			}
			else {
				//Register oneshot request
				sm_new_oneshot(mngr, sm_req.sensor, sm_req.dest, sm_req.tknlen, sm_req.tkn);
				//Do not free *tkn since now it's referenced by the
			}
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

		//I COULD USE ASYNCH REQUESTS INSTEAD OF THE REGISTRATIONS REGISTER

		while (queuecount < QUEUE_REQ_RATIO) {

		//is this blocking?
		if (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0) {

			if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
            	LOGI("accel: x=%f y=%f z=%f",
						event.acceleration.x, event.acceleration.y,
						event.acceleration.z);

            	//Update cache
            	mngr->sensors[ASENSOR_TYPE_ACCELEROMETER].event_cache = event;

            	/*
            	 * if we have any oneshot for this sensor, clear each of them and send a packet
            	 */
				if (mngr->sensors[ASENSOR_TYPE_ACCELEROMETER].oneshots != NULL) {

					//Take sample from cache and form payload
					pyl = malloc(sizeof(ze_payload_t));
					if (pyl == NULL)
						LOGW("malloc failed!");
					pyl->length = sizeof(ASensorEvent);
					pyl->data = malloc(pyl.length);
					if (pyl->data == NULL)
						LOGW("malloc failed!");
					*(pyl->data) = event;
					pyl->wts = event.timestamp;
					pyl->rtpts = 0;


					//make in this way so that we don't allocate a payload if we don't
					//have anybody to send to

					//for each oneshot
					while (mngr->sensors[ASENSOR_TYPE_ACCELEROMETER].oneshots != NULL) {

						ze_oneshot_t *tempy = mngr->sensors[ASENSOR_TYPE_ACCELEROMETER].oneshots;

						put_req_buf_item(notbuf, COAP_SEND_ASYNCH,
								mngr->sensors[ASENSOR_TYPE_ACCELEROMETER].uri,
								tempy->dest, COAP_MESSAGE_NON,
								tempy->tknlen, tempy->tkn, pyl);

						mngr->sensors[ASENSOR_TYPE_ACCELEROMETER].oneshots = tempy->next;
						free(tempy); //it frees the outside but not the token, very good.
					}
				}


				if (mngr->sensors[ASENSOR_TYPE_ACCELEROMETER].streams != NULL) {
					//Fot the moment only one observer possible for each sensor

					ze_oneshot_t *tempy = mngr->sensors[ASENSOR_TYPE_ACCELEROMETER].streams;

					//Take sample from cache and form payload
					pyl = malloc(sizeof(ze_payload_t));
					if (pyl == NULL)
						LOGW("malloc failed!");
					pyl->length = sizeof(ASensorEvent);
					pyl->data = malloc(pyl.length);
					if (pyl->data == NULL)
						LOGW("malloc failed!");
					*(pyl->data) = event;
					pyl->wts = event.timestamp;
					pyl->rtpts = 4567; //assign the timestamp

					put_req_buf_item(notbuf, COAP_SEND_NOTIF,
							mngr->sensors[ASENSOR_TYPE_ACCELEROMETER].uri,
							tempy->dest, COAP_MESSAGE_NON,
							tempy->tknlen, tempy->tkn, pyl);

					//do not need to clear anything..

					//Update sensor timestamp I guess, based on the frequency..

				}
				else {
					//we have cleared all the oneshots and there is no stream
					//for that sensor
					android_sensor_turnoff(mngr, ASENSOR_TYPE_ACCELEROMETER);
				}
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
