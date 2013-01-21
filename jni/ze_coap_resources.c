

void ze_coap_init_resources(coap_context_t *context) {


  coap_resource_t *r;

  /*
  r = coap_resource_init(NULL, 0, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"General Info\"", 14, 0);
  coap_add_resource(ctx, r);

  /* store clock base to use in /time */

  r = ze_coap_init_accel();
  coap_add_resource(context, r);


  my_clock_base = clock_offset;

  r = coap_resource_init((unsigned char *)"time", 4, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_time);
  coap_register_handler(r, COAP_REQUEST_PUT, hnd_put_time);
  coap_register_handler(r, COAP_REQUEST_DELETE, hnd_delete_time);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"Internal Clock\"", 16, 0);
  coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"Ticks\"", 7, 0);
  r->observable = 1;
  coap_add_attr(r, (unsigned char *)"if", 2, (unsigned char *)"\"clock\"", 7, 0);

  coap_add_resource(ctx, r);

#ifndef WITHOUT_ASYNC
  r = coap_resource_init((unsigned char *)"async", 5, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_async);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_resource(ctx, r);
#endif /* WITHOUT_ASYNC */
}

//coap_add_observer(resource, peer, token);
//coap_add_option(response, COAP_OPTION_SUBSCRIPTION, 0, NULL);



coap_resource_t * ze_coap_init_accel() {
	coap_resource_t *r;
	r = coap_resource_init(NULL, 0, 0);
	coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);

	coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
	coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"General Info\"", 14, 0);

	/*
	 *   r = coap_resource_init((unsigned char *)"time", 4, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_time);
  coap_register_handler(r, COAP_REQUEST_PUT, hnd_put_time);
  coap_register_handler(r, COAP_REQUEST_DELETE, hnd_delete_time);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"Internal Clock\"", 16, 0);
  coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"Ticks\"", 7, 0);
  r->observable = 1;
  coap_add_attr(r, (unsigned char *)"if", 2, (unsigned char *)"\"clock\"", 7, 0);
	 */

	return r;
}


void accel_GET_handler (coap_context_t  *context, struct coap_resource_t *resource,
	      coap_address_t *peer, coap_pdu_t *request, str *token,
	      coap_pdu_t *response) {

	/*
	 * remember to null the coap_resource_t copy of the pointer inside the state table
	 * in the streaming manager, otherwise it will continue to use it
	 * even if the resource is gone
	 */

	coap_opt_iterator_t opt_iter;
	int sub_success = -1;
	coap_opt_t *obs_opt;
	unsigned char *obs_opt_time;

	obs_opt = coap_check_option(request, COAP_OPTION_SUBSCRIPTION, &opt_iter);
	if (request != NULL && obs_opt) { 	// If there is an observe option

		if (resource->observable == 1) {
			// Call on_subscribe
			resource->on_register(context, resource, peer, request, token);

			if (request->hdr->type == COAP_MESSAGE_CON) {
				// Prepare response as simple ACK, without observe option
				// A default one has been prepared by the caller of this GET handler
				// and will be sent unless we NULL it
			}
			else if (request->hdr->type == COAP_MESSAGE_NON) {
				// Should not send anything, prepare invalid response
				// Done by NULLing it (my modification to libcoap)
				response = NULL;
			}
		}
		else {
			// As from draft-coap-observe par4.1 suggestion "unable or unwilling"
			accel_GET_oneshot_handler(context, resource, peer, request, token, response);
		}
	}

	else { //There isn't an observe option

		//Call on_unregister, the client might have been registered
		//and as draft-coap-observe par4.1 mandates, the registration must be canceled.
		//On unregister will perform the necessary checks.
		resource->on_unregister(context, resource, peer, request, token);

		//Send one-shot resource representation
		accel_GET_oneshot_handler(context, resource, peer, request, token, response);
	}

	return;
}

void accel_GET_oneshot_handler(coap_context_t  *context, struct coap_resource_t *resource,
	      coap_address_t *peer, coap_pdu_t *request, str *token,
	      coap_pdu_t *response) {

	// Ask the sensor framework give us the payload of one sample
	int payload_length;
	unsigned char *payload;
	payload_length = get_single_sample(SENSOR_TYPE_ACCELEROMETER, payload);

	// The library is going to automatically send a response, with a piggybacked ACK maybe

	//Add options
	coap_add_option(response, COAP_OPTION_TOKEN, token->length, token->s);
	//if (max_age!=NULL) coap_add_option(response, COAP_OPTION_MAXAGE, max_age_length, max_age);
	//if (reg->content_format!=NULL) coap_add_option(response, COAP_OPTION_CONTENT_FORMAT, 2, reg->content_format);

	//Add data
	coap_add_data(response, payload_length, payload);

	//Fire
	if (coap_send(context, peer, response) == COAP_INVALID_TID) {
			debug("cannot send one-shot response %d\n");
	}
}

void accel_on_register (coap_context_t  *context, struct coap_resource_t *resource,
	      coap_address_t *peer, coap_pdu_t *request, str *token) {

	int success1;
	coap_subscription_t success2;

	int frequency = 20;
	int deadline = 0;

	success1 = add_stream(SENSOR_TYPE_ACCELEROMETER, peer, frequency, deadline);
	if (success1 == 0) {
		success2 = coap_add_observer(resource, peer, token);
		if (success2 == NULL) LOGE("cannot add registration in the list");
	}
	else {
		// As from draft-coap-observe par4.1 suggestion "unable or unwilling"
		accel_GET_oneshot_handler(context, resource, peer, request, token, response);
	}


}

void accel_on_unregister (coap_context_t  *context, struct coap_resource_t *resource,
	      coap_address_t *peer, coap_pdu_t *request, str *token,
	      coap_pdu_t *response) {

	// Halt the stream in stream manager
	stop_stream(SENSOR_TYPE_ACCELEROMETER, peer);

	// Delete the observer from the resource register
	coap_delete_observer(resource, peer, token);
}

