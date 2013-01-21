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



int Java_eu_tb_zesense_ZeJNIHub_ze_1coap_1server_1example_1main() {

	LOGI("ZeSense new CoAP server hello!");

	// Server context, never to reallocate or move or delete,
	// its pointer is passed to many threads
	coap_context_t  *context;

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

	//Get an instance of the global context
	context = get_context(SERVER_IP, SERVER_PORT);
	if (!context)
		return -1;

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

	coap_free_context( ctx );

	return 0;
}
