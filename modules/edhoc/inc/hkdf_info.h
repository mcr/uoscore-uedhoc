/*
   Copyright (c) 2021 Fraunhofer AISEC. See the COPYRIGHT
   file at the top-level directory of this distribution.

   Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
   http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
   <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
   option. This file may not be copied, modified, or distributed
   except according to those terms.
*/

#ifndef HKDF_INFO_H
#define HKDF_INFO_H

#include "byte_array.h"
#include "error.h"
#include "suites.h"


/**
 * @brief   Encodes the HKDF Info as into the out-array
 * @param   aead_alg AEAD Algorithm
 * @param   th transcripthash
 * @param   th_len length of th
 * @param   label human readable label
 * @param   okm_len length of output keying material
 * @param   out out-array
 * @param   out_len length of out
 * @return  EdhocError
 */
EdhocError create_hkdf_info(
    enum aead_alg aead_alg,
    const uint8_t *th,
    uint8_t th_len,
    const char *label,
    uint64_t okm_len,
    uint8_t *out,
    uint8_t *out_len);

#endif
