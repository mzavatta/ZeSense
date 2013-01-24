ze_coap_server_core_thread(coap_context_t *cctx, ze_coap_request_buf_t *notbuf) {



	//Take request and dispatch





	ze_coap_request_t req;
	coap_pdu_t *pdu = NULL;
	coap_async_state_t *asy = NULL, *asyt = NULL;
	coap_resource_t *res;
	coap_subscription_t *sub = NULL, *subt = NULL;

	//Pick from the queue and perform the necessary
	req = get_req_buf_item(notbuf);

	if (req.rtype == COAP_SEND_ASYNCH) {

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
				      asy->message_id, req.pyl->length);
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
		}
	}
	else if (req.rtyp == COAP_SEND_NOT) {

		//lookup the resource with that uri, actually there may be none
		//because somebody might have erased it with a DELETE
		//while the Sampling Manager was still sending samples
		coap_key_t key;
		coap_hash(req.uri.s, req.uri.length, key);
		res = coap_get_resource_from_key(context, key);
		if (res != NULL) {
			//lookup the subscription within res
			sub = coap_find_observer_notoken(res, req.dest);
			if (sub != NULL) {

				//build pdu
				pdu = coap_pdu_init(req.conf, COAP_RESPONSE_205,
					      asy->message_id, req.pyl->length);
				coap_add_option(pdu, COAP_OPTION_SUBSCRIPTION, sizeof(short), cctx->observe);
				coap_add_option(pdu, COAP_OPTION_TOKEN, sub->token_length, sub->token);
				coap_add_data(pdu,
						((req.pyl->length)+sizeof(int64_t)+sizeof(int)+sizeof(int)),
						req.pyl);

				if (sub->non_cnt >= COAP_OBS_MAX_NON || req.conf == COAP_MESSAGE_CON) {
					//send a confirmable
					coap_send_confirmed(cctx, req.dest, pdu);
					sub->non_cnt = 0;

				}
				else {
					//send a non-confirmable
					coap_send_confirmed(cctx, req.dest, pdu);
					sub->non_cnt++;
				}

				coap_pdu_clear(pdu, );
			}
			else{
				LOGW("subscription not found within the resource!");
			}
		}
		else {
			LOGW("resource not found!");
		}
	}

	free(req.tkn);
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


coap_subscription_t *
coap_find_observer_notoken(coap_resource_t *resource, const coap_address_t *peer) {
  coap_subscription_t *s;
  assert(resource);
  assert(peer);
  LL_FOREACH(resource->subscribers, s) {
    if (coap_address_equals(&s->subscriber, peer))
      return s;
  }
  return NULL;
}

/* TOKEN COMPARISON, MIGHT BE USEFUL!
&& (!token || (token->length == s->token_length
	       && memcmp(token->s, s->token, token->length) == 0))
	       */
