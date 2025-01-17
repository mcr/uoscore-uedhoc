/*
   Copyright (c) 2021 Fraunhofer AISEC. See the COPYRIGHT
   file at the top-level directory of this distribution.

   Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
   http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
   <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
   option. This file may not be copied, modified, or distributed
   except according to those terms.
*/

#ifndef OSCORE_H
#define OSCORE_H

#include <stdbool.h>
#include <stdint.h>

#include "inc/byte_array.h"
#include "inc/error.h"
#include "inc/print_util.h"
#include "inc/security_context.h"
#include "inc/supported_algorithm.h"

#define MAX_PLAINTEXT_LEN 1024

/**
 * Each endpoint derives the parameters in the security context from a
 * small set of input parameters.
 */
struct oscore_init_params {
    enum dev_type dev_type;
    /*master_secret must be provided. Currently 16 byte secrets are supported*/
    const struct byte_array master_secret;
    /*sender_id must be provided*/
    const struct byte_array sender_id;
    /*recipient_id must be provided*/
    const struct byte_array recipient_id;

    /*The specification doesn't describe how the ID Context is created */
    /*When the user wants to use ID Context it has to provide it in the initialization of the client. The servers ID Context is transported in the oscore option*/
    struct byte_array id_context;
    /*master_salt is optional (default empty byte string)*/
    const struct byte_array master_salt;
    /*aead_alg is optional (default AES-CCM-16-64-128)*/
    const enum AEAD_algorithm aead_alg;
    /*kdf is optional (default HKDF-SHA-256)*/
    const enum hkdf hkdf;
};

/**
 * @brief Initialize security context of OSCORE, including common context, 
 * recipient context and sender context.
 * 
 * @param 	params a struct containing the initialization parameters
 * @param	context a struct containing the contexts
 * @return  OscoreError
 */
OscoreError oscore_context_init(
    struct oscore_init_params* params,
    struct context* c);

/**
 * @brief  	Checks if the packet in buf_in is a OSCORE packet.
 * 			If so it converts it to a CoAP packet and sets the oscore_pkg to
 * 			true in order to indicate the caller function that a
 * 			OSCORE packet was received.
 * 
 * @param 	buf_in a buffer containing an incoming packet which can be OSCORE or
 * 			CoAP packet.
 * @param 	buf_in_len length of the data in the buf_in
 * @param 	buf_out when a OSCORE packet is found and decrypted the resulting
 * 			CoAP is saved in buf_out
 * @param 	buf_out_len length of the CoAP packet
 * @param   oscore_pkg_flag true if the received packet was OSOCRE, if the 
 *          packet was CoAP false
 * @param 	c pointer to a security context
 * @param 	oscore_pkg indicates if an incoming packet is OSCORE
 * @return 	OscoreError
 */
OscoreError oscore2coap(
    uint8_t* buf_in, uint16_t buf_in_len,
    uint8_t* buf_out, uint16_t* buf_out_len,
    bool* oscore_pkg_flag,
    struct context* c);

/**
 *@brief 	Converts a CoAP packet to OSCORE packet
 *
 *@param	buf_o_coap a buffer containing a CoAP packet
 *@param	buf_o_coap_len length of the CoAP buffer
 *@param	buf_oscore a buffer where the OSCORE packet will be written
 *@param	buf_oscore_len length of the OSCORE packet
 *@param	c a struct containing the OSCORE context
 *@return 	OscoreError
 */
OscoreError coap2oscore(
    uint8_t* buf_o_coap, uint16_t buf_o_coap_len,
    uint8_t* buf_oscore, uint16_t* buf_oscore_len,
    struct context* c);

#endif
