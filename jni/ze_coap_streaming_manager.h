
// XXX: INITIAL SUPPORT ONLY FOR ONE OBSERVER FOR EACH SENSOR
ze_sensor_t ze_streaming_state[13];

// With the resource uri and the destination address we can associate each sensor
// to a specific resource in the CoAP realm
// TODO: the reliability policy might be parametrized too
int add_stream(int sensor, coap_address_t *dest, int freq, int deadline);
void stop_stream(int sensor, 	coap_address_t *dest); //if destination NULL, return global sensor info
int is_streaming(int sensor, 	coap_address_t *dest); //if destination NULL, return global sensor info



/**
 * Returns a single sample of @p sensor,
 * in the container @p data
 * Returns the length of the @p data
 *
 * The reliability policy here doesn't concern us since the actual send is done
 * by somebody else, which will decide the policy on his own
 */
int get_single_sample(int sensor, unsigned char *data);

typedef struct ze_sensor_t {

	// Association sensor-resource
	int sensor;
	str uri;
	/* this copy is very dangerous
	 * since we will use it to send messages
	 * we must remember to NULL this pointer when a resource gets deleted
	 * and to check before making a notification that this pointer is not NULL
	coap_resource_t *resource;
	*/

	/*
	 * Utility to be quick when answering a one-shot sensor value request
	 */
	ASensorEvent last_event;

	//ze_single_stream_t *streams = NULL;
	ze_single_stream_t stream = NULL;

	// Streaming manager local status variables
	int freq;
	int last_wts;
	int last_rtpts;
};

typedef struct ze_single_stream_t {

	ze_single_stream_t *next;

	/* In some way this is the key,
	 * no two elements with the same dest will be present in the list
	 * ss mandated by draft-coap-observe
	 */
	coap_address_t dest;

	// User specified
	int freq;
	int deadline;

	// Streaming manager local status variables
	int last_wts;	//Last wallclock timestamp
	int last_rtpts;	//Last RTP timestamp
	int freq_div;	//Frequency divider (automatically assigned)
};

typedef struct ze_sensor_coap_payload {
	int64_t wts;
	int rtpts;
	int debugpayload;
};



