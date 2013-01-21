/*

Thread

reads from socket and
calls dispatch
(this loop's load is longer than it may seem, dispatch performs a lot of work,
including the response handler and eventual registration handlers)

*/

void ze_coap_receiver_thread(coap_context_t *context) {

	int result;

	// File descriptors for select() to read
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET( context->sockfd, &readfds );

	while(1) {

		// Blocks indefinitely. That's good, this thread only performs this work
		result = select( FD_SETSIZE, &readfds, 0, 0, NULL );

		if ( result < 0 ) {		/* error */
			if (errno != EINTR)
				perror("select");
		} else if ( result > 0 ) {	/* read from socket */
			/* if our file descriptor is among those who changed */
			if ( FD_ISSET( context->sockfd, &readfds ) ) {
				coap_read( context );	/* read received data */
				coap_dispatch( context );	/* dispatch PDUs from receivequeue */
			}
		} else {	/* result<0 meaning timeout */
			/* coap_check_resource_list( context ); */
		}

	}
}

