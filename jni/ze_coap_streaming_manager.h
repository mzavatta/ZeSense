
// resource to sensor code map
// NO NEED, CAN HARDCODE IT IN THE RESOURCE SPECIFIC HANDLER!

// XXX: INITIAL SUPPORT ONLY FOR ONE OBSERVER FOR EACH SENSOR

void add_stream(coap_resource_t *resource, coap_address_t *dest, int sensor, str query);
void stop_stream(coap_address_t *dest, int sensor);
int is_streaming(coap_address_t *dest, int sensor); //if destination NULL, tell instead if the sensor is streaming at all

//
void send_single_sample(coap_context_t  *context, struct coap_resource_t *resource,
	      int sensor, coap_address_t *peer, coap_pdu_t *request, str *token);


typedef struct ze_sensor_t {
	int sensor;
	//ze_single_stream_t *streams = NULL;
	ze_single_stream_t stream;
	int freq; //Frequency of the Android sampling, maximum of those in streams
	int last_wts;
	int last_rtpts;


	/* this copy is very dangerous
	 * since we will use it to send messages
	 * we must remember to NULL this pointer when a resource gets deleted
	 * and to check before making a notification that this pointer is not NULL
	 */
	coap_resource_t *resource;

};

typedef struct ze_single_stream_t {

	ze_single_stream_t *next;

	//In some way this is the key
	//No two elements with the same dest will be present in the list
	//As mandated by draft-coap-observe
	coap_address_t dest;

	int freq;		//Frequency requested by client
	int freq_div;	//Frequency divider (might have a sample stream different from the requested freq)

	int last_wts;	//Last wallclock timestamp
	int last_rtpts;	//Last RTP timestamp

	int deadline;
};


ze_sensor_t ze_streaming_state[13]; //index is the Android's sensor number


typedef struct ze_sensor_coap_payload {

	int64_t wts;
	int rtpts;
	int debugpayload;

};

