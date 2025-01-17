/*
   Copyright (c) 2021 Fraunhofer AISEC. See the COPYRIGHT
   file at the top-level directory of this distribution.

   Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
   http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
   <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
   option. This file may not be copied, modified, or distributed
   except according to those terms.
*/
#include "oscore.h"

/**
 * Test 1:
 * - Client Key derivation with master salt see RFC8613 Appendix C.1.1
 * - Generating OSCORE request with key form C.1.1 see RFC8613 Appendix C.4 
 */

/*Test vector C1.1: Key derivation with Master Salt*/
uint8_t T1__MASTER_SECRET[16] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
uint8_t T1__MASTER_SECRET_LEN = sizeof(T1__MASTER_SECRET);

uint8_t* T1__SENDER_ID = NULL;
uint8_t T1__SENDER_ID_LEN = 0;

uint8_t T1__RECIPIENT_ID[1] = {0x01};
uint8_t T1__RECIPIENT_ID_LEN = sizeof(T1__RECIPIENT_ID);

uint8_t T1__MASTER_SALT[8] = {
    0x9e, 0x7c, 0xa9, 0x22, 0x23, 0x78, 0x63, 0x40};
uint8_t T1__MASTER_SALT_LEN = sizeof(T1__MASTER_SALT);

uint8_t* T1__ID_CONTEXT = NULL;
uint8_t T1__ID_CONTEXT_LEN = 0;

/*Test vector C4: Generating a OSCORE Packet with key material form test vector C.1 */
uint8_t T1__COAP_REQ[] = {
    0x44, 0x01, 0x5d, 0x1f, 0x00, 0x00, 0x39, 0x74,
    0x39, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f,
    0x73, 0x74, 0x83, 0x74, 0x76, 0x31};
uint16_t T1__COAP_REQ_LEN = sizeof(T1__COAP_REQ);

/*Expected result*/
uint8_t T1__SENDER_KEY[] = {
    0xf0, 0x91, 0x0e, 0xd7, 0x29, 0x5e, 0x6a, 0xd4,
    0xb5, 0x4f, 0xc7, 0x93, 0x15, 0x43, 0x02, 0xff};
uint8_t T1__SENDER_KEY_LEN = sizeof(T1__SENDER_KEY);

uint8_t T1__RECIPIENT_KEY[] = {
    0xff, 0xb1, 0x4e, 0x09, 0x3c, 0x94, 0xc9, 0xca,
    0xc9, 0x47, 0x16, 0x48, 0xb4, 0xf9, 0x87, 0x10};
uint8_t T1__RECIPIENT_KEY_LEN = sizeof(T1__RECIPIENT_KEY);

uint8_t T1__COMMON_IV[] = {
    0x46, 0x22, 0xd4, 0xdd, 0x6d, 0x94, 0x41, 0x68,
    0xee, 0xfb, 0x54, 0x98, 0x7c};
uint8_t T1__COMMON_IV_LEN = sizeof(T1__COMMON_IV);

uint8_t T1__OSCORE_REQ[] = {
    0x44, 0x02, 0x5d, 0x1f, 0x00, 0x00, 0x39, 0x74,
    0x39, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f,
    0x73, 0x74, 0x62, 0x09, 0x14, 0xff, 0x61, 0x2f,
    0x10, 0x92, 0xf1, 0x77, 0x6f, 0x1c, 0x16, 0x68,
    0xb3, 0x82, 0x5e};
uint8_t T1__OSCORE_REQ_LEN = sizeof(T1__OSCORE_REQ);

/**
 * Test 2:
 * - Server Key derivation with master salt see RFC8613 Appendix C.1.2
 * - Generating OSCORE response with key form C.1.2 see RFC8613 Appendix C.7 
 */
uint8_t T2__MASTER_SECRET[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
uint8_t T2__MASTER_SECRET_LEN = sizeof(T2__MASTER_SECRET);

uint8_t T2__SENDER_ID[] = {0x01};
uint8_t T2__SENDER_ID_LEN = sizeof(T2__SENDER_ID);

uint8_t* T2__RECIPIENT_ID = NULL;
uint8_t T2__RECIPIENT_ID_LEN = 0;

uint8_t T2__MASTER_SALT[8] = {0x9e, 0x7c, 0xa9, 0x22, 0x23, 0x78, 0x63, 0x40};
uint8_t T2__MASTER_SALT_LEN = sizeof(T2__MASTER_SALT);

uint8_t* T2__ID_CONTEXT = NULL;
uint8_t T2__ID_CONTEXT_LEN = 0;

/*The OSCORE message created in C4 (35 Byte). Constructed from a CoAP request of length 22. This request contains no payload. The request contains only Uri-host (locahost) and Uri-path option (tv1). In the OSCORE packet Uri-host option is transferred as plain normal option, The Uri-path is contained in the ciphertext. */
uint8_t T2__OSCORE_REQ[] = {0x44, 0x02, 0x5d, 0x1f, 0x00, 0x00, 0x39, 0x74, 0x39, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0x62, 0x09, 0x14, 0xff, 0x61, 0x2f, 0x10, 0x92, 0xf1, 0x77, 0x6f, 0x1c, 0x16, 0x68, 0xb3, 0x82, 0x5e};
uint8_t T2__OSCORE_REQ_LEN = sizeof(T2__OSCORE_REQ);

/*Unprotected CoAP response (21 bytes)*/
/*Contains the payload "Hello World!"*/
uint8_t T2__COAP_RESPONSE[] = {0x64, 0x45, 0x5d, 0x1f, 0x00, 0x00, 0x39, 0x74, 0xff, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21};
uint8_t T2__COAP_RESPONSE_LEN = sizeof(T2__COAP_RESPONSE);

/*Expected result*/
/*the  reconstructed coap request see Appendix C4*/
uint8_t T2__COAP_REQ[] = {0x44, 0x01, 0x5d, 0x1f, 0x00, 0x00, 0x39, 0x74, 0x39, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0x83, 0x74, 0x76, 0x31};
uint8_t T2__COAP_REQ_LEN = sizeof(T2__COAP_REQ);

uint8_t T2__OSCORE_RESP[] = {0x64, 0x44, 0x5D, 0x1F, 0x00, 0x00, 0x39, 0x74, 0x90, 0xFF, 0xDB, 0xAA, 0xD1, 0xE9, 0xA7, 0xE7, 0xB2, 0xA8, 0x13, 0xD3, 0xC3, 0x15, 0x24, 0x37, 0x83, 0x03, 0xCD, 0xAF, 0xAE, 0x11, 0x91, 0x06};
uint8_t T2__OSCORE_RESP_LEN = sizeof(T2__OSCORE_RESP);

/**
 * Test 3:
 * - Client Key derivation without master salt see RFC8613 Appendix C.2.1
 * - Generating OSCORE request with key form C.2.1 see RFC8613 Appendix C.5 
 */
/*Test vector C2.1: Key derivation without Master Salt*/
uint8_t T3__MASTER_SECRET[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
uint8_t T3__MASTER_SECRET_LEN = sizeof(T3__MASTER_SECRET);

uint8_t T3__SENDER_ID[1] = {0x00};
uint8_t T3__SENDER_ID_LEN = sizeof(T3__SENDER_ID);

uint8_t T3__RECIPIENT_ID[1] = {0x01};
uint8_t T3__RECIPIENT_ID_LEN = sizeof(T3__RECIPIENT_ID);

uint8_t* T3__MASTER_SALT = NULL;
uint8_t T3__MASTER_SALT_LEN = 0;

uint8_t* T3__ID_CONTEXT = NULL;
uint8_t T3__ID_CONTEXT_LEN = 0;

/*Test vector C5: Generating a OSCORE Packet with key material form test vector C.2.1 */
uint8_t T3__COAP_REQ[] = {0x44, 0x01, 0x71, 0xc3, 0x00, 0x00, 0xb9, 0x32, 0x39,
                          0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0x83, 0x74, 0x76,
                          0x31};
uint16_t T3__COAP_REQ_LEN = sizeof(T3__COAP_REQ);

/*expected result*/
uint8_t T3__OSCORE_REQ[] = {0x44, 0x02, 0x71, 0xc3, 0x00, 0x00, 0xb9, 0x32, 0x39, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0x63, 0x09, 0x14, 0x00, 0xff, 0x4e, 0xd3, 0x39, 0xa5, 0xa3, 0x79, 0xb0, 0xb8, 0xbc, 0x73, 0x1f, 0xff, 0xb0};
uint8_t T3__OSCORE_REQ_LEN = sizeof(T3__OSCORE_REQ);

/**
 * Test 4:
 * - Server Key derivation without master salt see RFC8613 Appendix C.2.2
 */
/*Test vector C2.2: Key derivation without Master Salt*/
uint8_t T4__MASTER_SECRET[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
uint8_t T4__MASTER_SECRET_LEN = sizeof(T4__MASTER_SECRET);

uint8_t T4__SENDER_ID[1] = {0x01};
uint8_t T4__SENDER_ID_LEN = sizeof(T4__SENDER_ID);

uint8_t T4__RECIPIENT_ID[1] = {0x00};
uint8_t T4__RECIPIENT_ID_LEN = sizeof(T4__RECIPIENT_ID);

uint8_t* T4__MASTER_SALT = NULL;
uint8_t T4__MASTER_SALT_LEN = 0;

uint8_t* T4__ID_CONTEXT = NULL;
uint8_t T4__ID_CONTEXT_LEN = 0;

/*expected result*/
uint8_t T4__SENDER_KEY[] = {0xe5, 0x7b, 0x56, 0x35, 0x81, 0x51, 0x77, 0xcd, 0x67, 0x9a, 0xb4, 0xbc, 0xec, 0x9d, 0x7d, 0xda};
uint8_t T4__SENDER_KEY_LEN = sizeof(T4__SENDER_KEY);

uint8_t T4__RECIPIENT_KEY[] = {0x32, 0x1b, 0x26, 0x94, 0x32, 0x53, 0xc7, 0xff, 0xb6, 0x00, 0x3b, 0x0b, 0x64, 0xd7, 0x40, 0x41};
uint8_t T4__RECIPIENT_KEY_LEN = sizeof(T4__RECIPIENT_KEY);

uint8_t T4__COMMON_IV[] = {0xbe, 0x35, 0xae, 0x29, 0x7d, 0x2d, 0xac, 0xe9, 0x10, 0xc5, 0x2e, 0x99, 0xf9};
uint8_t T4__COMMON_IV_LEN = sizeof(T4__COMMON_IV);

/**
 * Test 5 :
 * - Client Key derivation with ID Context see Appendix 3.1
 * - OSCORE request generation see Appendix C6
 */
/*Test vector C3.1: Key derivation with ID Context*/
uint8_t T5__MASTER_SECRET[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
uint8_t T5__MASTER_SECRET_LEN = sizeof(T5__MASTER_SECRET);

uint8_t* T5__SENDER_ID = NULL;
uint8_t T5__SENDER_ID_LEN = 0;

uint8_t T5__RECIPIENT_ID[1] = {0x01};
uint8_t T5__RECIPIENT_ID_LEN = sizeof(T5__RECIPIENT_ID);

uint8_t T5__MASTER_SALT[8] = {0x9e, 0x7c, 0xa9, 0x22, 0x23, 0x78, 0x63, 0x40};
uint8_t T5__MASTER_SALT_LEN = sizeof(T5__MASTER_SALT);

uint8_t T5__ID_CONTEXT[8] = {0x37, 0xcb, 0xf3, 0x21, 0x00, 0x17, 0xa2, 0xd3};
uint8_t T5__ID_CONTEXT_LEN = sizeof(T5__ID_CONTEXT);

/*Test vector C6: Generating a OSCORE Packet with key material form test vector C.2.1 */
uint8_t T5__COAP_REQ[] = {0x44, 0x01, 0x2f, 0x8e, 0xef, 0x9b, 0xbf, 0x7a, 0x39,
                          0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0x83, 0x74, 0x76,
                          0x31};
uint16_t T5__COAP_REQ_LEN = sizeof(T5__COAP_REQ);

/*Expected result*/
uint8_t T5__OSCORE_REQ[] = {0x44, 0x02, 0x2f, 0x8e, 0xef, 0x9b, 0xbf, 0x7a, 0x39, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0x6b, 0x19, 0x14, 0x08, 0x37, 0xcb, 0xf3, 0x21, 0x00, 0x17, 0xa2, 0xd3, 0xff, 0x72, 0xcd, 0x72, 0x73, 0xfd, 0x33, 0x1a, 0xc4, 0x5c, 0xff, 0xbe, 0x55, 0xc3};
uint8_t T5__OSCORE_REQ_LEN = sizeof(T5__OSCORE_REQ);

/**
 * Test 6:
 * - Server Key derivation with ID context see RFC8613 Appendix C.3.2
 */
uint8_t T6__MASTER_SECRET[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
uint8_t T6__MASTER_SECRET_LEN = sizeof(T6__MASTER_SECRET);

uint8_t T6__SENDER_ID[1] = {0x01};
uint8_t T6__SENDER_ID_LEN = sizeof(T6__SENDER_ID);

uint8_t* T6__RECIPIENT_ID = NULL;
uint8_t T6__RECIPIENT_ID_LEN = 0;

uint8_t T6__MASTER_SALT[8] = {0x9e, 0x7c, 0xa9, 0x22, 0x23, 0x78, 0x63, 0x40};
uint8_t T6__MASTER_SALT_LEN = sizeof(T5__MASTER_SALT);

uint8_t T6__ID_CONTEXT[8] = {0x37, 0xcb, 0xf3, 0x21, 0x00, 0x17, 0xa2, 0xd3};
uint8_t T6__ID_CONTEXT_LEN = sizeof(T5__ID_CONTEXT);

/*expected result*/
uint8_t T6__SENDER_KEY[] = {0xe3, 0x9a, 0x0c, 0x7c, 0x77, 0xb4, 0x3f, 0x03, 0xb4, 0xb3, 0x9a, 0xb9, 0xa2, 0x68, 0x69, 0x9f};
uint8_t T6__SENDER_KEY_LEN = sizeof(T6__SENDER_KEY);

uint8_t T6__RECIPIENT_KEY[] = {0xaf, 0x2a, 0x13, 0x00, 0xa5, 0xe9, 0x57, 0x88, 0xb3, 0x56, 0x33, 0x6e, 0xee, 0xcd, 0x2b, 0x92};
uint8_t T6__RECIPIENT_KEY_LEN = sizeof(T6__RECIPIENT_KEY);

uint8_t T6__COMMON_IV[] = {0x2c, 0xa5, 0x8f, 0xb8, 0x5f, 0xf1, 0xb8, 0x1c, 0x0b, 0x71, 0x81, 0xb8, 0x5e};
uint8_t T6__COMMON_IV_LEN = sizeof(T6__COMMON_IV);

/**
 * Test 7:
 * - Server with partial IV see Appendix C8
 * - currently not supported
 */
uint8_t T7__MASTER_SECRET[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
uint8_t T7__MASTER_SECRET_LEN = sizeof(T7__MASTER_SECRET);

uint8_t T7__SENDER_ID[] = {0x01};
uint8_t T7__SENDER_ID_LEN = sizeof(T7__SENDER_ID);

uint8_t* T7__RECIPIENT_ID = NULL;
uint8_t T7__RECIPIENT_ID_LEN = 0;

uint8_t T7__MASTER_SALT[8] = {0x9e, 0x7c, 0xa9, 0x22, 0x23, 0x78, 0x63, 0x40};
uint8_t T7__MASTER_SALT_LEN = sizeof(T7__MASTER_SALT);

uint8_t* T7__ID_CONTEXT = NULL;
uint8_t T7__ID_CONTEXT_LEN = 0;

/*Test vector C4: Generating a OSCORE Packet with key material form test vector C.1 */
/*the OSCORE message created in C4*/
uint8_t T7__OSCORE_REQ[] = {0x44, 0x02, 0x5d, 0x1f, 0x00, 0x00, 0x39, 0x74, 0x39, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0x62, 0x09, 0x14, 0xff, 0x61, 0x2f, 0x10, 0x92, 0xf1, 0x77, 0x6f, 0x1c, 0x16, 0x68, 0xb3, 0x82, 0x5e};
uint8_t T7__OSCORE_REQ_LEN = sizeof(T7__OSCORE_REQ);

/*unprotected CoAP response*/
uint8_t T7__COAP_RESPONSE[] = {0x64, 0x45, 0x5d, 0x1f, 0x00, 0x00, 0x39, 0x74, 0xff, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21};
uint8_t T7__COAP_RESPONSE_LEN = sizeof(T7__COAP_RESPONSE);

/*expected result*/
uint8_t T7__OSCORE_RES[] = {0x64, 0x44, 0x5d, 0x1f, 0x00, 0x00, 0x39, 0x74, 0x92, 0x01, 0x00, 0xff, 0x4d, 0x4c, 0x13, 0x66, 0x93, 0x84, 0xb6, 0x73, 0x54, 0xb2, 0xb6, 0x17, 0x5f, 0xf4, 0xb8, 0x65, 0x8c, 0x66, 0x6a, 0x6c, 0xf8, 0x8e};
uint8_t T7__OSCORE_RES_LEN = sizeof(T7__OSCORE_RES);
