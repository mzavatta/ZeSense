/*
 * ZeSense Streaming Manager
 * -- fixed length FIFO buffer (non-circular)
 * 	  for incoming requests to Streaming Manager
 * 	  thread safe implementation
 *
 * Marco Zavatta
 * <marco.zavatta@telecom-bretagne.eu>
 * <marco.zavatta@mail.polimi.it>
 */


ze_request_t get_req_buf_item(ze_request_buf_t *buf) {

	/* Here I have the choice of creating a new item or just pass
	 * it back by value.
	 * The only difference is that the caller will need to call free_item
	 * but actually to free only the tkn!
	 *
	 * I would prefer allocating a new space for it but still..
	 */

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

// token is not big, we can copy it..
int put_req_buf_item(ze_request_buf_t *buf, int rtype, int sensor, coap_address_t dest,
		int freq, int tknlen, unsigned char *tkn) {

	pthread_mutex_lock(buf->mtx);
		if (buf->counter >= SM_RBUF_SIZE) { //full (greater shall not happen)
			pthread_cond_wait(buf->notfull, buf->mtx);
		}

		// Copy contents
		buf->rbuf[buf->puthere].rtype = rtype;
		buf->rbuf[buf->puthere].sensor = sensor;
		buf->rbuf[buf->puthere].dest = dest;
		buf->rbuf[buf->puthere].freq = freq;
		buf->rbuf[buf->puthere].tknlen = tknlen;

		// Pay attention to the pointer issue
		buf->rbuf[buf->puthere].tkn = malloc(item->tknlen);
		if (buf->rbuf[buf->puthere].tkn == NULL) {
			LOGW("malloc error");
	pthread_mutex_unlock(buf->mtx);
			return SM_ERROR;
		}
		*buf->rbuf[buf->puthere].tkn = *(item->tkn);

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

ze_sm_request_t* create_item(int rtype, int sensor, coap_address_t *dest,
		int freq, int tknlen, unsigned char *tkn) {

	ze_sm_request_t* temp = malloc(sizeof(ze_sm_request_t));
	it (temp == NULL) {
		LOGW("malloc failed");
		return NULL;
	}
	temp->tkn = malloc(tknlen);
	if (temp->tnk == NULL) {
		LOGW("malloc failed");
		return NULL;
	}

	temp->rtype = rtype;
	temp->sensor = sensor;
	temp->dest = &dest;
	temp->freq = freq;
	temp->tknlen = tknlen;
	*(temp->tkn) = *tkn;

	return temp;
}

//Attention it only frees the tkn!
void free_item(ze_sm_request_t *item) {
	free(item->tkn);
}

typedef struct ze_sm_request_t {
	/* Request type */
	int rtype;

	//TODO: could use a union

	/* Request parameters, NULL when they do not apply */
	int sensor;
	coap_address_t dest;
	int freq;
	int tknlen;
	unsigned char *tkn; //remember to allocate a new one!
};
