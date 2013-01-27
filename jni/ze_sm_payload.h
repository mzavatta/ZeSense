/*
 * ZeSense Streaming Manager
 * -- payload structures
 *
 * Marco Zavatta
 * <marco.zavatta@telecom-bretagne.eu>
 * <marco.zavatta@mail.polimi.it>
 */


typedef struct ze_payload_t {
	int64_t wts;
	unsigned int rtpts;
	/* Doing a dynamic size data field so that
	 * we can carry whatever we want in there
	 */
	int length;
	unsigned char *data;
};
