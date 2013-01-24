

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

	//GET FREQUENCY FROM THE QUERY STRING IN THE REQUEST!
	int freq = 20;

	obs_opt = coap_check_option(request, COAP_OPTION_SUBSCRIPTION, &opt_iter);
	if (request != NULL && obs_opt) { 	// If there is an observe option

		if (resource->observable == 1) {

			put_req_buf_item(buf, SM_REQ_START, ASENSOR_TYPE_ACCELEROMETER, peer,
					freq, NULL, NULL);

			//Does it replaces it if already existing?
			coap_add_observer(resource, peer, token);


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
			//Ask oneshot representation
			put_req_buf_item(buf, SM_REQ_ONESHOT, ASENSOR_TYPE_ACCELEROMETER, peer,
					NULL, token->length, token->s);
		}
	}

	else { //There isn't an observe option

		//Call on_unregister, the client might have been registered
		//and as draft-coap-observe par4.1 mandates, the registration must be canceled.
		//On unregister will perform the necessary checks.

		//Ask oneshot representation
		put_req_buf_item(buf, SM_REQ_ONESHOT, ASENSOR_TYPE_ACCELEROMETER, peer,
				NULL, token->length, token->s);

		if (coap_find_observer(resource, peer,token) != NULL) {

			// Stop stream to that peer
			put_req_buf_item(buf, SM_REQ_STOP, ASENSOR_TYPE_ACCELEROMETER, peer,
					NULL, NULL, NULL);

			// Delete the observer from the resource register
			coap_delete_observer(resource, peer, token);
		}
	}

	return;
}
