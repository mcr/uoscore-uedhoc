/*
   Copyright (c) 2021 Fraunhofer AISEC. See the COPYRIGHT
   file at the top-level directory of this distribution.

   Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
   http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
   <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
   option. This file may not be copied, modified, or distributed
   except according to those terms.
*/

#include <stdint.h>
#include <stdio.h>

void print_array(uint8_t* in_data, uint16_t in_len) {
    //fflush(stdout);
    printf(" (size %u):", in_len);
    for (uint16_t i = 0; i < in_len; i++) {
        if (i % 16 == 0)
            printf("\n\t%02X ", in_data[i]);
        else
            printf("%02X ", in_data[i]);
    }
    printf("\n");
    //fflush(stdout);
}
