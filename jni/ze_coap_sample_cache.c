/*
 * ZeSense Streaming Manager
 * -- last known sample cache
 *
 * Marco Zavatta
 * <marco.zavatta@telecom-bretagne.eu>
 * <marco.zavatta@mail.polimi.it>
 */

int get_sample_cache(ze_sample_cache_t *cache, int sensor, ASensorEvent *sample) {

	if (sensor<0 || sensor>=ZE_NUMSENSORS) {
		printf("sample cache get sample invalid parameters\n");
		return SM_ERROR;
	}

	sample = malloc(sizeof(ASensorEvent));
	if ( sample == NULL ) {
		printf("sample cache get sample error: malloc\n");
		return SM_ERROR;
	}

	pthread_mutex_lock(cache->mtx[sensor]);
		if (cache->events[sensor].empty == FALSE) { //not empty
			*sample = cache->events[sensor].event;
	pthread_mutex_unlock(cache->mtx[sensor]);
			return 0;
		}
	pthread_mutex_unlock(cache->mtx[sensor]);
	return SM_EMPTY;
}

int update_sample_cache(ze_sample_cache_t *cache, int sensor, ASensorEvent sample) {

	if (sensor<0 || sensor>=ZE_NUMSENSORS) {
		printf("sample cache update sample invalid parameters\n");
		return SM_ERROR;
	}

	pthread_mutex_lock(cache->mtx[sensor]);
		cache->events[sensor].event = sample;
		cache->events[sensor].empty = FALSE;
	pthread_mutex_unlock(cache->mtx[sensor]);
}

int init_cache(ze_sample_cache_t *cache) {

	//tutte le empty flags a TRUE!

	//inizializzare i mutex

}


