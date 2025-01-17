/*
   Copyright (c) 2021 Fraunhofer AISEC. See the COPYRIGHT
   file at the top-level directory of this distribution.

   Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
   http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
   <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
   option. This file may not be copied, modified, or distributed
   except according to those terms.
*/

#ifndef OSCORE_COSE_H
#define OSCORE_COSE_H

#include "byte_array.h"
#include "error.h"

/**
 * @brief Encode the input AAD to defined COSE structure
 * @param external_aad: input aad to form COSE structure
 * @param out: output encoded COSE byte string
 * @return OscoreError
 */
OscoreError create_enc_structure(struct byte_array* external_aad, struct byte_array* out);

/**
 * @brief Calculate the length of the COSE format for input AAD
 * @param external_aad: input aad to form COSE structure
 * @param out: output length
 * @return OscoreError
 */
OscoreError enc_structure_length(struct byte_array* external_aad, size_t* out);

/**
 * @brief Decrypt the ciphertext
 * @param in_ciphertext: input ciphertext to be decrypted
 * @param out_plaintext: output plaintext
 * @param nonce the nonce
 * @param aad the aad
 * @param recipient_key the recipient key
 * @return OscoreError
 */
OscoreError cose_decrypt(
    struct byte_array* in_ciphertext,
    struct byte_array* out_plaintext,
    struct byte_array* nonce,
    struct byte_array* aad,
    struct byte_array* recipient_key);

/**
 * @brief Encrypt the plaintext
 * @param in_plaintext: input plaintext to be encrypted
 * @param out_ciphertext: output ciphertext with authentication tag (8 bytes)
 * @param nonce the nonce
 * @param aad the aad
 * @param sender_key the sender key
 * @return OscoreError
 */
OscoreError cose_encrypt(
    struct byte_array* in_plaintext,
    uint8_t *out_ciphertext, uint32_t out_ciphertext_len,
    struct byte_array* nonce,
    struct byte_array* sender_aad, 
    struct byte_array* key) ;
#endif
