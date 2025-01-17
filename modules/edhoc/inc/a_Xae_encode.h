/*
   Copyright (c) 2021 Fraunhofer AISEC. See the COPYRIGHT
   file at the top-level directory of this distribution.

   Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
   http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
   <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
   option. This file may not be copied, modified, or distributed
   except according to those terms.
*/
#ifndef A_3AE_ENCODE_H
#define A_3AE_ENCODE_H

#include <stdint.h>

#include "error.h"

/**
 * @brief   Ecodes associated data for message 3. (COSE "Encrypt0") data 
 *          structure is used 
 * @param   th can be th2 or th3
 * @param   th_len the length of th
 * @param   A_Xae pointer to hold the encoded data
 * @param   A_Xae_len length of the encoded data
 */
EdhocError a_Xae_encode(
    uint8_t* th, const uint16_t th_len,
    uint8_t* A_Xae, uint32_t* A_Xae_len);

#endif