/*
 * ZeSense CoAP server
 * -- fixed length FIFO buffer (non-circular)
 * 	  for incoming requests to the CoAP server
 * 	  from the Sensor Manager
 *
 * Marco Zavatta
 * <marco.zavatta@telecom-bretagne.eu>
 * <marco.zavatta@mail.polimi.it>
 */

/* Buffer size */
#define COAP_RBUF_SIZE		20

/* Request codes */
#define COAP_SEND_NOT		50
#define COAP_SEND_ASYNCH	60

typedef struct ze_coap_request_buf_t {

	ze_coap_request_t rbuf[COAP_RBUF_SIZE];

	/* Indexes, wrap around according to %SM_RBUF_SIZE*/
	int gethere, puthere;

	/* Item counter, does not wrap around */
	int counter;

	/* Thread synch (no need of the empty condition) */
	pthread_mutex_t mtx;
	pthread_cond_t notfull;
	//pthread_cond_t notempty;
};

typedef struct ze_coap_request_t {
	/* Request type */
	int rtype;

	//TODO: could use a union

	/* Request parameters, NULL when they do not apply */
	coap_address_t dest;
	int tknlen;
	unsigned char *tkn; //remember to allocate a new one!

	ze_payload_container_t *pyl;
	//actually for this we do not need to allocate a new one
	//as long as we remember to free it!

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
ze_coap_request_t get_req_buf_item(ze_coap_request_buf_t *buf);

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
 * the buffer is full. It DOES NOT make a copy of the parameters
 * passed by pointer;
 *
 * @param The buffer instance
 * @param The item to be inserted, passed by value
 *
 * @return Zero on success
 */
int put_req_buf_item(ze_coap_request_buf_t *buf, int rtype, int sensor_id, coap_address_t dest,
		int tknlen, unsigned char *tkn, ze_payload_t *pyl);

void init_req_buf(ze_coap_request_buf_t *buf);
