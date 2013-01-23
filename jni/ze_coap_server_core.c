/*
 * ZeSense CoAP server
 * -- core module
 *
 * Marco Zavatta
 * <marco.zavatta@telecom-bretagne.eu>
 * <marco.zavatta@mail.polimi.it>
 *
 * Built using libcoap by
 * Olaf Bergmann <bergmann@tzi.org>
 * http://libcoap.sourceforge.net/
 */

#include 'ze_coap_server_core.h'
#include 'ze_coap_resources.h'


int Java_eu_tb_zesense_ZeJNIHub_ze_1coap_1server_1core() {

	LOGI("ZeSense new CoAP server hello!");

	/* Spawn *all* the threads from here, this thread has no other function
	 * except for instantiating the CoAP, SM contexts and the two buffers
	 * and passing their references to the threads.
	 *
	 * This thread is in some way the lifecycle manager of the system.
	 */

	/* Contexts and buffers; NEVER to reallocate, move or delete because
	 * they are shared among threads! unless we use
	 * reference counting */
	coap_context_t  *cctx;
	cctx = get_context(SERVER_IP, SERVER_PORT);
	if (!context)
		return -1;

	stream_context_t *smctx;
	//Init

	ze_request_buf_t smreqbuf;
	init_req_buf(&smreqbuf);

	//Instance of notifications buf
	//Init

    // Open log file
	char *logpath = LOGPATH;
	logfd = fopen(logpath,"ab");
	if(logfd == NULL) {
		LOGW("unable to open %s", logpath);
		exit(1);
	}
	else LOGI("success opening %s", logpath);


	// Log experiment start time
	if (fputs(ctime(&lt), logfd)<0) LOGW("write failed");

	/* Fire threads! (as the last thing) */
	/*
	 * In a three-thread scenario:
	 * - receiver needs cctx, smctx, smreqbuf, notbuf, cache
	 * - sm needs smctx, smreqbuf, notbuf, cache
	 * - sender needs cctx, smreqbuf, notbuf
	 *
	 * (receiver needs smctx because of the tiny bastard pick-up of
	 * a one-shot sample, which for the moment we leave in the receiver
	 * thread)
	 * (for the moment we leave one-shot requests to the receiver
	 * supposing that the request rate is not so overwhelming..
	 * though this means either sharing the outwards socket or
	 * create a response buffer for the sender to pick up from)
	 * (a possible solution is to modify the notbuf & coap_notify() to include
	 * not only notifications but any kind of response, with a parameter fed to
	 * coap_notify() do drive the coap_notify() behaviour)
	 *
	 * (sender needs smreqbuf because it might
	 * decide to stop a stream if the acks do not arrive)
	 */

	fd_set readfds;
	struct timeval tv, *timeout;
	int result;
	coap_tick_t now;
	coap_queue_t *nextpdu;
	//char addr_str[NI_MAXHOST] = "192.168.43.1";
	//char addr_str[NI_MAXHOST] = "10.0.2.15";
	//char port_str[NI_MAXSERV] = "5683";
	int opt;
	coap_log_t log_level = LOG_WARN;

	coap_set_log_level(log_level);



	ze_coap_init_resources(context);

	//signal(SIGINT, handle_sigint);


	// Start children threads



	while ( !quit ) {
		FD_ZERO(&readfds);
		FD_SET( ctx->sockfd, &readfds );

		nextpdu = coap_peek_next( ctx );

		coap_ticks(&now);
		while ( nextpdu && nextpdu->t <= now ) {
			coap_retransmit( ctx, coap_pop_next( ctx ) );
			nextpdu = coap_peek_next( ctx );
		}

		if ( nextpdu && nextpdu->t <= now + COAP_RESOURCE_CHECK_TIME ) {
			/* set timeout if there is a pdu to send before our automatic timeout occurs */
			tv.tv_usec = ((nextpdu->t - now) % COAP_TICKS_PER_SECOND) * 1000000 / COAP_TICKS_PER_SECOND;
			tv.tv_sec = (nextpdu->t - now) / COAP_TICKS_PER_SECOND;
			timeout = &tv;
		} else {
			tv.tv_usec = 0;
			tv.tv_sec = COAP_RESOURCE_CHECK_TIME;
			timeout = &tv;
		}
		result = select( FD_SETSIZE, &readfds, 0, 0, timeout );

		if ( result < 0 ) {		/* error */
			if (errno != EINTR)
				perror("select");
		} else if ( result > 0 ) {	/* read from socket */
			if ( FD_ISSET( ctx->sockfd, &readfds ) ) {
				coap_read( ctx );	/* read received data */
				coap_dispatch( ctx );	/* and dispatch PDUs from receivequeue */
			}
		} else {			/* timeout */
			/* coap_check_resource_list( ctx ); */
		}

#ifndef WITHOUT_ASYNC
	/* check if we have to send asynchronous responses */
	check_async(ctx, now);
#endif /* WITHOUT_ASYNC */

#ifndef WITHOUT_OBSERVE
	/* check if we have to send observe notifications */
	check_observe(ctx);
#endif /* WITHOUT_OBSERVE */
  }

	coap_free_context( cctx );
	sm_free_context( smctx );

	return 0;
}
