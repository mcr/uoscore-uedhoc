/*
   Copyright (c) 2021 Fraunhofer AISEC. See the COPYRIGHT
   file at the top-level directory of this distribution.

   Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
   http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
   <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
   option. This file may not be copied, modified, or distributed
   except according to those terms.
*/

#include "../inc/byte_array.h"

struct byte_array EMPTY_ARRAY = {
    .len = 0,
    .ptr = (uint8_t*)"",
};

struct byte_array NULL_ARRAY = {
    .len = 0,
    .ptr = NULL,
};

bool array_equals(const struct byte_array* left,const struct byte_array* right) {
    if (left->len != right->len) {
        return false;
    }
    for (int i = 0; i < left->len; i++) {
        if (left->ptr[i] != right->ptr[i]) {
            return false;
        }
    }
    return true;
}
