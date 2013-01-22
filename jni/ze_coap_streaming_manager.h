/*
 * ZeSense Streaming Manager
 * -- core streaming module
 *
 * Marco Zavatta
 * <marco.zavatta@telecom-bretagne.eu>
 * <marco.zavatta@mail.polimi.it>
 */


#include <android/sensor.h>


/* Error conditions */
#define SM_ERROR			(-1)
#define SM_URI_REPLACED 	1
#define SM_STREAM_REPLACED	2
#define SM_NEGATIVE			3

/* Request codes
 * mirroring start_stream() and
 * stop_stream() calls */
#define SM_REQ_START		10
#define SM_REQ_STOP			20

/* Synchronization settings */
#define RTP_CLOCK_FREQ		200

/* Sensor settings */
#define ACCEL_MAX_FREQ		100
#define GYRO_MAX_FREQ		200
#define LIGHT_MAX_FREQ		200

/* Streaming Manager settings */
#define SM_RBUF_SIZE		20

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

/**
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

/**
 * Streaming Manager's global context
 * Array indexes mirror Android-defined sensor types
 */
typedef struct stream_context_t {
	/* Sensors sources available for streaming */
	ze_sensor_t sensor[13];

	/* The server we're sending the streams to */
	coap_context_t server = NULL;

	//Transferred elsewhere... it does not belong here..
	//it belongs where the instance of stream_context_t is!
	/* Request buffer available for a multithread implementation */
	//ze_request_t rbuf[SM_RBUF_SIZE];
};

/**
 * Fixed length FIFO buffer (non-circular)
 */
typedef struct ze_request_buf_t {

	ze_request_t rbuf[SM_RBUF_SIZE];

	/* Indexes, wrap around according to %SM_RBUF_SIZE*/
	int gethere, puthere;

	/* Item counter, does not wrap around */
	int counter;

	/* Thread synch (no need of the empty condition) */
	pthread_mutex_t mtx;
	pthread_cond_t notfull;
	//pthread_cond_t notempty;
};

/**
 * To fit our purposes:
 * - It MUST NOT block on the empty condition. The putter might not
 * feed any more data into it, but we must go on!
 * - It COULD block on the mutex. We're confident that the getter
 * will not starve us.
 *
 * Gets the oldest item in the buffer @p buf. It blocks if the buffer is
 * being used by another thread, it does not block if empty.
 *
 * @param The buffer instance
 *
 * @return The oldest item in the buffer, NULL if buffer empty
 */
ze_request_t get_req_buf_item(ze_request_buf_t *buf) {

	pthread_mutex_lock(buf->mtx);
		if (buf->counter <= 0) { //empty (shall never < 0 anyway)
			/*
			 * pthread_cond_wait(buf->notempty, buf->mtx);
			 * do nothing, we must not block!
			 */
			return NULL;
		}
		else {
			ze_request_t temp = buf->rbuf[buf->gethere];
			buf->gethere = ((buf->gethere)+1) % SM_RBUF_SIZE;
			counter--;
			pthread_cond_signal(buf->notfull); //surely no longer full
		}
	pthread_mutex_unlock(buf->mtx);

	return temp;
}

/*
 * To fit our purposes:
 * - It SHOULD block on the full condition. Were do we put the message from
 * the network once we've fetched it from the socket?  We're confident that
 * the getter will not starve us.
 * - It SHOULD block on the mutex. We're confident that the getter will not
 * starve us.
 *
 * Puts an item in the buffer @p buf. It blocks if the buffer is
 * being used by another thread, and it also blocks indefinitely is
 * the buffer is full.
 *
 * @param The buffer instance
 * @param The item to be inserted, passed by value
 *
 * @return Zero on success
 */
int put_req_buf_item(ze_request_buf_t *buf, ze_request_t item) {

	pthread_mutex_lock(buf->mtx);
		if (buf->counter >= SM_RBUF_SIZE) { //full (greater shall not happen)
			pthread_cond_wait(buf->notfull, buf->mtx);
		}
		buf->rbuf[buf->puthere] = item;
		buf->puthere = ((buf->puthere)+1) % SM_RBUF_SIZE;
		counter++;
		//pthread_cond_signal(buf->notempty); //surely no longer empty
	pthread_mutex_unlock(buf->mtx);

	return 0;
}

void init_req_buf(ze_request_buf_t *buf) {

	/* What happens if a thread tries to initialize a mutex or a cond var
	 * that has already been initialized? "POSIX explicitly
	 * states that the behavior is not defined, so avoid
	 * this situation in your programs"
	 */
	int error = pthread_mutex_init(buf->mtx, NULL);
	if (error)
		fprintf(stderr, "Failed to initialize mtx:%s\n", strerror(error));

	error = pthread_cond_init(buf->notfull, NULL);
	if (error)
		fprintf(stderr, "Failed to initialize full cond var:%s\n", strerror(error));

	/*
	 * error = pthread_cond_init(buf->notempty, NULL);
	 * if (error)
	 *	 fprintf(stderr, "Failed to initialize empty cond var:%s\n", strerror(error));
	 */

	/* Reset pointers */
	buf->gethere = 0;
	buf->puthere = 0;
	buf->counter = 0;
}



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

typedef struct ze_request_t {
	/* Request type */
	int rtype;

	//TODO: could use a union

	/* Request parameters, NULL when they do not apply */
	int sensor;
	coap_address_t *dest;
	int freq;
};


