// the four request handler methods
// the two observation handler methods

/*
 * init method for registration in the CoAP server
 */



void ze_coap_init_resources(coap_context_t *context);


// Accelerometer
coap_resource_t * ze_coap_init_accel();
void accel_GET_handler (coap_context_t  *context, struct coap_resource_t *resource,
	      coap_address_t *peer, coap_pdu_t *request, str *token,
	      coap_pdu_t *response);
void accel_on_register (coap_context_t  *ctx, struct coap_resource_t *resource,
	      coap_address_t *peer, coap_pdu_t *request, str *token,
	      coap_pdu_t *response);
void accel_on_unregister (coap_context_t  *ctx, struct coap_resource_t *resource,
	      coap_address_t *peer, coap_pdu_t *request, str *token,
	      coap_pdu_t *response);
