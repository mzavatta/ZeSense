

void ze_coap_sender_thread(coap_context_t *context) {

	//TODO: mutexes

	coap_queue_t *nextpdu;

	coap_tick_t now;

	while (1) {

		//Check sendqueue for retransmissions
		nextpdu = coap_peek_next( context );
		coap_ticks(&now);
		while ( nextpdu && nextpdu->t <= now ) {
			coap_retransmit( context, coap_pop_next( context ) );
			nextpdu = coap_peek_next( context );
		}

		// sleep(sometime);

	}

}

	/*
	thread:

	see sendqueue for retransmissions
	è qui o nella libreria che gestiamo : ?
	retransmission timer
	quali subscriptions fermare ecc..

	see real sendqueue for real transmissions

	pick from scheduler
	 */



}

