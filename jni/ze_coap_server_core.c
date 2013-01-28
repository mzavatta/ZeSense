ze_coap_server_core_thread(coap_context_t *cctx, ze_coap_request_buf_t *notbuf) {

	ze_coap_request_t req;
	coap_address_t dest;
	coap_pdu_t *pdu = NULL;
	coap_async_state_t *asy = NULL, *asyt = NULL;
	coap_resource_t *res;
	coap_subscription_t *sub = NULL, *subt = NULL;

	size_t pdusize = 0;

	unsigned char *pyl = NULL;
	int plylength = 0;


	/*
	 * start by fetching a request and dispatch it
	 */
	req = get_req_buf_item(notbuf);
	/* Recall that the getter does already the checkout
	 * on the reference counter.
	 * We must free
	 */

	if (req.rtpye == COAP_STREAM_STOPPED) {
		/* we're sure that no other notification will arrive
		 * with that ticket. release it on behalf of the streaming
		 * manager. if there is no ongoing transaction, it will be
		 * destroyed. after the last ongoing transaction finishes,
		 * it calls release and if it's the last one holding the
		 * registration pointer, it will free the memory.
		 */
		coap_registration_release(req.reg);
	}
	else if (req.rtype == COAP_SEND_ASYNCH) {

		/*
		//lookup request in the asynch register. It must be there.
		asy = cctx->async_state;
		asyt = cctx->async_state;
		while (asy != NULL) {
			if ( coap_address_equals(req.dest, asy->peer) == 1
					&&	(*(req.tkn) == *(asy->token)) ) break;
			asyt = asy;
			asy = asy->next;
		}
		if (asy == NULL) {
			LOGW("problem, asynch request not found");
		}
		else { //found it
			//Send response
			pdu = coap_pdu_init(req.conf, COAP_RESPONSE_205,
				      asy->message_id non credo vada bene, req.pyl->length);
			coap_add_option(pdu, COAP_OPTION_TOKEN, req.tknlen, req.tkn);
			coap_add_data(pdu,
					((req.pyl->length)+sizeof(int64_t)+sizeof(int)+sizeof(int)),
					req.pyl);

			if (req.conf == COAP_MESSAGE_CON) { //confirmable
				coap_send_confirmed(cctx, req.dest, pdu);
			}
			else { //non confirmable
				coap_send(cctx, req.dest, pdu);
			}

			//clear PDU for next loop
			coap_pdu_clear(pdu, max size??);

			//remove asynch transaction
			asyt->next = asy->next;
			coap_free_async(asy);
			*/

	}
	else if (req.rtype == COAP_SEND_NOTIF) {

		//dest = req.reg->subscriber;

		//lookup the resource with that uri, actually there may be none
		//because somebody might have erased it with a DELETE
		//while the Sampling Manager was still sending samples
		//coap_key_t key;
		//coap_hash(req.uri.s, req.uri.length, key);
		//res = coap_get_resource_from_key(context, key);
		//if (res != NULL) {
			//lookup the subscription within res
			//sub = coap_find_observer(res, req.dest);
			//if (sub != NULL) {

		/* Build pdu.
		 * the size parameter given to coap_pdu_init
		 * consist of a hint on the total message size
		 * (header + options + data)
		 * the actual length of the data sent will be computed as
		 * options and data are added and must turn out <= size
		 * given to coap_pdu_init. So let's be loose and give
		 * COAP_MAX_PDU_SIZE (1400 bytes)
		 */
		//pdusize += sizeof(coap_hdr_t); //header
		//pdusize += req.reg->token_length; //token
		//pdusize += sizeof(unsigned short); //observe
		//pdusize +=

		/* Transfer our payload structure into a series of bytes.
		 * Sending only the timestamp and the sensor event for the
		 * moment.
		 * TODO: optimize it so that we don't create another copy
		 * and it need not malloc every loop
		 * could be passed already in this way by the request manager..
		 */
		pyllength = sizeof(int64_t)+sizeof(int)+(req.pyl->length);
		pyl = malloc(pyllegth);
		if (pyl == NULL) {
			LOGW("cannot malloc for payload in server core thread");
			exit(1);
		}
		memcpy(pyl, &(reg.pyl->wts), sizeof(int64_t));
		memcpy(pyl+sizeof(int64_t), &(req.pyl->length), sizeof(int));
		memcpy(pyl+sizeof(int64_t)+sizeof(int), req.pyl->data, req.pyl->length);

		/* Need to add options in order... */
		pdu = coap_pdu_init(req.conf, COAP_RESPONSE_205,
				coap_new_message_id(cctx), COAP_MAX_PDU_SIZE);
		coap_add_option(pdu, COAP_OPTION_SUBSCRIPTION, sizeof(short), cctx->observe);
		coap_add_option(pdu, COAP_OPTION_TOKEN, req.reg->token_length, req.reg->token);
		coap_add_data(pdu, pyllength, pyl);

		if (req.reg->non_cnt >= COAP_OBS_MAX_NON || req.conf == COAP_MESSAGE_CON) {
			/* either the max NON have been reached or
			 * we explicitly requested a CON.
			 * Send a CON and clean the NON counter
			 */
			/* TODO: redo such a function, the registration reference must be
			 * registered in the transaction record. In other words, we need to pass
			 * coap_registration_checkout(coap_registration_t *r);
			 */
			coap_send_confirmed(cctx, req.reg->subscriber, pdu);
			req.reg->non_cnt = 0;
		}
		else {
			/* send a non-confirmable
			 * and increase the NON counter
			 * no need to keep the transaction state
			 */
			coap_send(cctx, req.reg->subscriber, pdu);
			req.reg->non_cnt++;
		}

		coap_pdu_clear(pdu, COAP_MAX_PDU_SIZE);
		/* Even if pyl is a pointer to char, it does not
		 * free only one byte. The heap manager stores
		 * when doing malloc() the number of bytes it allocated
		 * nearby the allocated block. So free will know
		 * how many bytes to deallocate
		 */
		free(pyl);

	}
	else {
		LOGW("Cannot interpret request type");
	}


	free(req.pyl->data);
	free(req.pyl);


	//Consider retransmissions







}
/*
asynch_equals(coap_async_state_t *one, coap_async_state_t *two) {
	coap_address_equals(one->peer, const coap_address_t *b) 1 if equal
	if (one->peer == two->peer
			&&
			one->token == two->token)
		return 0;
}*/




/* TOKEN COMPARISON, MIGHT BE USEFUL!
&& (!token || (token->length == s->token_length
	       && memcmp(token->s, s->token, token->length) == 0))
	       */
