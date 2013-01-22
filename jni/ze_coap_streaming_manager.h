/*
 * ZeSense Streaming Manager
 * -- core streaming module
 *
 * Marco Zavatta
 * <marco.zavatta@telecom-bretagne.eu>
 * <marco.zavatta@mail.polimi.it>
 */

/**
 * Binds a sensor source @p sensor to a specific @p URI.
 * Only one URI can be associated to a sensor source;
 * If an association is already in place, it will be overwritten.
 *
 * @param mngr		The Streaming Manager context
 * @param sensor	One of the available sensor sources
 * @param uri		String variable to associate to @p sensor
 *
 * @return Zero on success and first association,
 * @c SM_URI_REPLACED if successful and association overwritten
 * @c SM_ERROR on failure
 */
int bind_source(stream_context_t *mngr, int sensor, str uri);

/*
 * Binds the given @p server to Streaming Manager @p mngr
 * in order to relay notifications to him.
 */
int bind_server(stream_context_t *mngr, coap_context_t *server);

/**
 * Starts a stream of notifications of samples
 * collected by @p sensor to destination @p dest.
 * The client will be notified at the given frequency @p freq.
 * Only one specific destination is allowed for a sensor source.
 * If already existing, it will be replaced.
 * The requested @p freq will be rounded to multiples of 10Hz
 * and capped to the maximum running frequency
 * of the sensor source.
 *
 * TODO: many other parameters may be added in the future
 * for example reliability policy, the deadlines or
 * the timestamps policy (for now they are fixed and hard-coded)
 * maybe even in the form of some query language
 *
 * @param mngr		The Streaming Manager context
 * @param sensor	The sensor source of data
 * @param dest		The IP/port coordinates of the destination
 * @param freq		The frequency of notifications
 *
 * @return Zero on success, @c SM_STREAM_REPLACED if the new stream
 * replaced an existing one, @c SM_ERROR on failure
 */
int start_stream(stream_context_t *mngr, int sensor, coap_address_t *dest, int freq);

/**
 * Stops the stream of notifications from @p sensor
 * to destination @p dest.
 *
 * @param mngr		The Streaming Manager context
 * @param sensor	The sensor source of data
 * @param dest		The IP/port coordinates of the destination
 *
 * @return Zero on success, @c SM_ERROR on failure
 * (e.g. the stream does not exist)
 */
int stop_stream(stream_context_t *mngr, int sensor, coap_address_t *dest);

/**
 * Checks if a stream from @p sensor to destination @p dest
 * is currently running.
 *
 * @param mngr		The Streaming Manager context
 * @param sensor	The sensor source of data
 * @param dest		The IP/port coordinates of the destination
 *
 * @return Zero if positive answer, @c SM_NEGATIVE otherwise
 */
int is_streaming(stream_context_t *mngr, int sensor, 	coap_address_t *dest);

/**
 * Returns a single sample of @p sensor
 * Is is put in the container @p data of length @p length
 * The memory allocated for @p data will not be freed.
 *
 * @param mngr		The Streaming Manager context
 * @param sensor	The sensor source of data
 * @param data		Data bytes
 * @param length	Length of the @p data field
 *
 * @return Zero on success, @c SM_ERROR on failure
 */
int get_single_sample(stream_context_t *mngr, int sensor,
		unsigned char *data, int length);


typedef struct stream_context_t {
	/* Sensors sources available for streaming */
	ze_sensor_t sensor[13];

	/* The server we're sending the streams to */
	coap_context_t server = NULL;
};


typedef struct ze_sensor_t {
	/* Association sensor-resource */
	int sensor;
	str uri;

	/* Quick access to last known sensor value */
	ASensorEvent last_known_event;

	/* List of streams registered on this sensor */
	//ze_single_stream_t *streams = NULL;
	ze_single_stream_t stream = NULL;

	/* Local status variables */
	int freq;
	int last_wts;
	int last_rtpts;
};

typedef struct ze_stream_t {
	ze_single_stream_t *next = NULL;

	/* In some way this is the lookup key,
	 * no two elements with the same dest will be present in the list
	 * as mandated by draft-coap-observe-7
	 */
	coap_address_t dest;

	/* Client specified */
	int freq;

	/* Streaming Manager local status variables */
	int last_wts;	//Last wallclock timestamp
	int last_rtpts;	//Last RTP timestamp
	int freq_div;	//Frequency divider
};

typedef struct ze_payload_container_t {
	ze_payload_t pyl;
	int length;
};

typedef struct ze_payload_t {
	int64_t wts;
	int rtpts;
	unsigned char *pyl;
};





