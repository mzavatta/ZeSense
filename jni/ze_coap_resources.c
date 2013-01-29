

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

	coap_opt_iterator_t opt_iter;
	coap_opt_t *obopt;
	coap_registration_t *reg;

	/* TODO
	 * Instead of setting 20Hz by default
	 * interpret parameters in the request query
	 * string
	 */
	int freq = 20;

	obopt = coap_check_option(request, COAP_OPTION_SUBSCRIPTION, &opt_iter);
	if (obopt != NULL) { //There is an observe option

		if (resource->observable == 1) {

			/* The returned pointer is either a new pointer or an
			 * existing one. The reference counter is not incremented
			 * by coap_add_registration(), not even for the reference
			 * that is held by the sibling in the registration list.
			 * The destructor takes care of unplugging the registration
			 * from the list correctly.
			 */
			reg = coap_add_registration(resource, peer, token);

			/* The ticket gets created by this call. */
			put_req_buf_item(buf, SM_REQ_START, ASENSOR_TYPE_ACCELEROMETER,
					coap_registration_ceckout(reg), freq);


			if (request->hdr->type == COAP_MESSAGE_CON) {
				/* Prepare response as simple ACK, without observe option
				 * A default ACK has been prepared by the caller of this GET handler
				 * so just don't touch it
				 */
			}
			else if (request->hdr->type == COAP_MESSAGE_NON) {
				/* Should send nothing in reply at the messaging layer
				 * (we'll send the response later)
				 * NULL the proposed response container, so that
				 * the response that is passed back to the caller
				 * is discarded. (my modification to libcoap)
				 */
				response = NULL;
			}

			/* Releasing because we've copied the ticket in the message queue.
			 * Not releasing because we haven't checked it out in this
			 * routine.*/
			//coap_registration_release(reg);
		}
		else {
			/* As from draft-coap-observe par4.1 suggestion "unable or unwilling",
			 * ask one-shot representation to SM. The ticket field is NULL
			 * at the moment.
			 */
			put_req_buf_item(buf, SM_REQ_ONESHOT, ASENSOR_TYPE_ACCELEROMETER,
					NULL, NULL);

			//FIXME
			coap_register_async(context, peer, request,
					COAP_ASYNC_SEPARATE, NULL);
		}
	}

	else { //There isn't an observe option

		/* Ask a regular oneshot representation. */
		put_req_buf_item(buf, SM_REQ_ONESHOT, ASENSOR_TYPE_ACCELEROMETER,
				NULL, NULL);

		//FIXME
		coap_register_async(context, peer, request,
				COAP_ASYNC_SEPARATE, NULL);

		/* As per CoAP observer draft, clear this registration.
		 * This must be done through the streaming manager
		 * SM_REQ_STOP, passing the ticket of the registration.
		 * It's the resource-specific on_unregister()'s duty.
		 */
		reg = coap_find_registration(resource, peer);
		if (reg != NULL)
			resource->on_unregister(buf, reg);
	}

	return;
}

void on_unregister(ze_request_buf_t *buf, coap_registration_t *reg) {

	/*
	 * Unregistration must be done through the streaming manager
	 * SM_REQ_STOP, passing the ticket of the registration.
	 * The streaming manager will confirm the
	 * cancellation, then the reference count will be decremented
	 * by the server and if zero the registration destroyed.
	 * If the streaming manager does not have any stream
	 * with that ticket (should not happen), it confirms the
	 * cancellation anyways.
	 */

	put_req_buf_item(buf, SM_REQ_STOP, ASENSOR_TYPE_ACCELEROMETER,
			reg, NULL);

}
