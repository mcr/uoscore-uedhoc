/*
   Copyright (c) 2021 Fraunhofer AISEC. See the COPYRIGHT
   file at the top-level directory of this distribution.

   Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
   http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
   <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
   option. This file may not be copied, modified, or distributed
   except according to those terms.
*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../inc/aad.h"
#include "../inc/byte_array.h"
#include "../inc/coap.h"
#include "../inc/error.h"
#include "../inc/nonce.h"
#include "../inc/option.h"
#include "../inc/oscore_cose.h"
#include "../inc/print_util.h"
#include "../inc/security_context.h"

/**
 * @brief Parse all received options to find the OSCORE_option. If it doesn't  * have OSCORE option, then this packet is a normal CoAP. If it does have, it's * an OSCORE packet, and then parse the compressed OSCORE_option value to get  * value of PIV, KID and KID context of the client.
 * @param in: input OSCORE packet
 * @param out: pointer output compressed OSCORE_option
 * @return error types or is or not OSCORE packet
 */
static inline OscoreError oscore_option_parser(
    struct o_coap_packet* in,
    struct compressed_oscore_option* out,
    bool* oscore_pkt) {
    uint8_t temp_option_count = in->options_cnt;
    struct o_coap_option* temp_options = in->options;
    uint8_t temp_option_num = 0;
    uint8_t* temp_current_option_value_ptr;
    uint8_t temp_kid_len = 0;

    *oscore_pkt = false;

    for (uint8_t i = 0; i < temp_option_count; i++) {
        temp_option_num = temp_options[i].option_number;
        temp_kid_len = temp_options[i].len;

        /* Check current option is OSCORE_option or not */
        if (temp_option_num == COAP_OPTION_OSCORE) {
            if (temp_options[i].len == 0) {
                /* No OSCORE option value*/
                out->h = 0;
                out->k = 0;
                out->n = 0;
                //				out->KID_len = 0;
                //				out->KIDC_len = 0;
                //				out->PIV = NULL;
                //				out->KID = NULL;
                //				out->KID_context = NULL;
            } else {
                /* Get address of current option value*/
                temp_current_option_value_ptr = temp_options[i].value;
                /* Parse first byte of OSCORE value*/
                out->h = ((*temp_current_option_value_ptr) & COMP_OSCORE_OPT_KIDC_H_MASK) >> COMP_OSCORE_OPT_KIDC_H_OFFSET;
                out->k = ((*temp_current_option_value_ptr) & COMP_OSCORE_OPT_KID_K_MASK) >> COMP_OSCORE_OPT_KID_K_OFFSET;
                out->n = ((*temp_current_option_value_ptr) & COMP_OSCORE_OPT_PIV_N_MASK) >> COMP_OSCORE_OPT_PIV_N_OFFSET;
                temp_current_option_value_ptr++;
                temp_kid_len--;

                /* Get PIV */
                switch (out->n) {
                    case 0:
                        /* NO PIV in COSE object*/
                        out->piv.ptr = NULL;
                        break;
                    case 6:
                    case 7:
                        /* ERROR: Byte length of PIV not right, max. 5 bytes */
                        return OscoreInPktInvalidPiv;
                        break;
                    default:
                        out->piv.ptr = temp_current_option_value_ptr;
                        out->piv.len = out->n;
                        temp_current_option_value_ptr += out->n;
                        temp_kid_len -= out->n;
                        break;
                }

                /* Get KID context */
                if (out->h == 0) {
                    out->kid_context.len = 0;
                    out->kid_context.ptr = NULL;
                } else {
                    out->kid_context.len = *temp_current_option_value_ptr;
                    out->kid_context.ptr = ++temp_current_option_value_ptr;
                    temp_current_option_value_ptr += out->kid_context.len;
                    temp_kid_len -= (out->kid_context.len + 1);
                }

                /* Get KID */
                if (out->k == 0) {
                    out->kid.len = 0;
                    out->kid.ptr = NULL;
                } else {
                    out->kid.len = temp_kid_len;
                    out->kid.ptr = temp_current_option_value_ptr;
                }
            }

            *oscore_pkt = true;
        }
    }

    return OscoreNoError;
}

/**
 * @brief Decrypt the OSCORE payload (ciphertext)
 * @param out_plaintext: output plaintext
 * @param received_piv_kid_context: received PIV, KID and KID context, will be used to calculate AEAD nonce and AAD
 * @param oscore_packet: complete OSCORE packet which contains the ciphertext to be decrypted
 * @return void
 */
static inline OscoreError payload_decrypt(
    struct context* c,
    struct byte_array* out_plaintext,
    struct o_coap_packet* oscore_packet) {
    struct byte_array oscore_ciphertext = {
        .len = oscore_packet->payload_len,
        .ptr = oscore_packet->payload,
    };
    return cose_decrypt(
        &oscore_ciphertext,
        out_plaintext,
        &c->rrc.nonce,
        &c->rrc.aad,
        &c->rc.recipient_key);
}

/**
 * @brief Reorder E-options and other U-options, and update their delta, and combine them all to normal CoAP packet
 * @param in_oscore_packet: input OSCORE, which contains U-options
 * @param E_options: input pointer to E-options array
 * @param E_options_cnt: count number of input E-options
 * @param out_o_coap_packet: output pointer to CoAP packet, which will have all reordered options
 * @return void
 */
void options_from_oscore_reorder(struct o_coap_packet* in_oscore_packet,
                                 struct o_coap_option* E_options, uint8_t E_options_cnt,
                                 struct o_coap_packet* out_o_coap_packet) {
    uint16_t temp_delta_sum = 0;
    uint8_t temp_opt_cnt = in_oscore_packet->options_cnt + E_options_cnt;
    uint8_t o_coap_opt_cnt = temp_opt_cnt - 1;

    /* Get all option numbers */
    if (o_coap_opt_cnt > 0) {
        uint8_t temp_opt_number[o_coap_opt_cnt];
        memset(temp_opt_number, 0, o_coap_opt_cnt);

        /* Get all option numbers but discard OSCORE option */
        uint8_t j = 0;
        for (uint8_t i = 0; i < o_coap_opt_cnt + 1; i++) {
            if (i < in_oscore_packet->options_cnt) {
                if (in_oscore_packet->options[i].option_number != COAP_OPTION_OSCORE)
                    temp_opt_number[j++] =
                        in_oscore_packet->options[i].option_number;
            } else {
                temp_opt_number[j++] = E_options[i - in_oscore_packet->options_cnt].option_number;
            }
        }

        /* Reorder the option numbers from minimum to maximum */
        // uint8_t min_opt_number;
        for (uint8_t i = 0; i < o_coap_opt_cnt; i++) {
            for (uint8_t k = i + 1; k < o_coap_opt_cnt; k++) {
                if (temp_opt_number[i] > temp_opt_number[k]) {
                    uint8_t temp;
                    temp = temp_opt_number[i];
                    temp_opt_number[i] = temp_opt_number[k];
                    temp_opt_number[k] = temp;
                }
            }
        }

        /* Reset output CoAP options count */
        out_o_coap_packet->options_cnt = 0;

        /* Copy all U-options and E-options in increasing number sequence*/
        uint8_t U_opt_idx = 0;
        uint8_t E_opt_idx = 0;
        for (uint8_t i = 0; i < o_coap_opt_cnt; i++) {
            if (temp_opt_number[i] == in_oscore_packet->options[U_opt_idx].option_number) {
                out_o_coap_packet->options[out_o_coap_packet->options_cnt].delta =
                    in_oscore_packet->options[U_opt_idx].option_number - temp_delta_sum;
                out_o_coap_packet->options[out_o_coap_packet->options_cnt].len =
                    in_oscore_packet->options[U_opt_idx].len;
                out_o_coap_packet->options[out_o_coap_packet->options_cnt].option_number =
                    in_oscore_packet->options[U_opt_idx].option_number;
                out_o_coap_packet->options[out_o_coap_packet->options_cnt].value =
                    in_oscore_packet->options[U_opt_idx].value;

                temp_delta_sum +=
                    out_o_coap_packet->options[out_o_coap_packet->options_cnt].delta;
                out_o_coap_packet->options_cnt++;
                U_opt_idx++;
                continue;
            } else if (temp_opt_number[i] == E_options[E_opt_idx].option_number) {
                out_o_coap_packet->options[out_o_coap_packet->options_cnt].delta =
                    E_options[E_opt_idx].option_number - temp_delta_sum;
                out_o_coap_packet->options[out_o_coap_packet->options_cnt].len =
                    E_options[E_opt_idx].len;
                out_o_coap_packet->options[out_o_coap_packet->options_cnt].option_number =
                    E_options[E_opt_idx].option_number;
                out_o_coap_packet->options[out_o_coap_packet->options_cnt].value =
                    E_options[E_opt_idx].value;

                temp_delta_sum +=
                    out_o_coap_packet->options[out_o_coap_packet->options_cnt].delta;
                out_o_coap_packet->options_cnt++;
                E_opt_idx++;
                continue;
            }
        }
    } else {
        /* No any options! */
        out_o_coap_packet->options_cnt = 0;
    }
}

/**
 * @brief Parse incoming options byte string into options structure
 * @param in_data: pointer to input data in byte string format
 * @param in_data_len: length of input byte string
 * @param out_options: pointer to output options structure array
 * @param out_options_count: count number of output options
 * @return  OscoreError
 */
OscoreError oscore_packet_options_parser(uint8_t* in_data, uint16_t in_data_len,
                                         struct o_coap_option* out_options, uint8_t* out_options_count) {
    uint8_t* temp_options_ptr = in_data;
    uint8_t temp_options_count = 0;
    uint8_t temp_option_header_len = 0;
    uint16_t temp_option_delta = 0;
    uint16_t temp_option_len = 0;
    uint8_t temp_option_number = 0;

    // Go through the in_data to find out how many options are there
    uint16_t i = 0;
    while (i < in_data_len) {
        temp_option_header_len = 1;
        // Parser first byte,lower 4 bits for option value length and higher 4 bits for option delta
        temp_option_delta = ((*temp_options_ptr) & 0xF0) >> 4;
        temp_option_len = (*temp_options_ptr) & 0x0F;

        temp_options_ptr++;

        // Special cases for extended option delta: 13 - 1 extra delta byte, 14 - 2 extra delta bytes, 15 - reserved
        switch (temp_option_delta) {
            case 13:
                temp_option_header_len += 1;
                temp_option_delta = *temp_options_ptr - 13;
                temp_options_ptr += 1;
                break;
            case 14:
                temp_option_header_len += 2;
                temp_option_delta = ((uint16_t)(*temp_options_ptr) << 8 | *(temp_options_ptr + 1)) - 269;
                temp_options_ptr += 2;
                break;
            case 15:
                // ERROR
                return OscoreInPktInvalidOptionDelta;
                break;
            default:
                break;
        }

        // Special cases for extended option value length: 13 - 1 extra length byte, 14 - 2 extra length bytes, 15 - reserved
        switch (temp_option_len) {
            case 13:
                temp_option_header_len += 1;
                temp_option_len = *temp_options_ptr - 13;
                temp_options_ptr += 1;
                break;
            case 14:
                temp_option_header_len += 2;
                temp_option_len = ((uint16_t)(*temp_options_ptr) << 8 | *(temp_options_ptr + 1)) - 269;
                temp_options_ptr += 2;
                break;
            case 15:
                // ERROR
                return OscoreInPktInvalidOptionLen;
                break;
            default:
                break;
        }

        temp_option_number += temp_option_delta;
        // Update in output options
        out_options[temp_options_count].delta = temp_option_delta;
        out_options[temp_options_count].len = temp_option_len;
        out_options[temp_options_count].option_number = temp_option_number;
        if (temp_option_len == 0)
            out_options[temp_options_count].value = NULL;
        else
            out_options[temp_options_count].value = temp_options_ptr;

        // Update parameters
        i += temp_option_header_len + temp_option_len;
        temp_options_ptr += temp_option_len;
        temp_options_count++;
    }

    // Assign options count number
    *out_options_count = temp_options_count;

    return OscoreNoError;
}

/**
 * @brief Parse the decrypted OSCORE payload into code, E-options and original unprotected CoAP payload
 * @param in_payload: input decrypted payload
 * @param out_code: pointer to code number of the request
 * @param out_E_options: output pointer to an array of E-options
 * @param E_options_cnt: count number of E-options
 * @param out_o_coap_payload: output pointer original unprotected CoAP payload
 * @return  OscoreError
 */
OscoreError oscore_decrypted_payload_parser(
    struct byte_array* in_payload,
    uint8_t* out_code, struct o_coap_option* out_E_options,
    uint8_t* E_options_cnt, struct byte_array* out_o_coap_payload) {
    uint8_t* temp_payload_ptr = in_payload->ptr;
    uint16_t temp_payload_len = in_payload->len;

    /* Code */
    *out_code = *(temp_payload_ptr++);
    temp_payload_len--;

    /* Check whether E-options exists */
    if (*temp_payload_ptr == 0xFF || temp_payload_len == 0) {
        *E_options_cnt = 0;
    } else {
        uint8_t* temp_option_ptr = temp_payload_ptr;
        /* Length of E-options byte string*/
        uint16_t options_len = 0;

        /* Move the temp_ptr to the original unprotected payload */
        while (*temp_payload_ptr != 0xFF && temp_payload_len != 0) {
            temp_payload_len--;
            temp_payload_ptr++;
            options_len++;
        }

        /* Parser all options */
        if (options_len > 0) {
            uint8_t r = 0;
            r = oscore_packet_options_parser(
                temp_option_ptr, options_len,
                out_E_options, E_options_cnt);
            if (r != OscoreNoError) return r;
        } else
            *E_options_cnt = 0;
    }

    /* Unprotected CoAP payload */
    ++temp_payload_ptr;
    if (temp_payload_len == 0) {
        out_o_coap_payload->len = 0;
        out_o_coap_payload->ptr = NULL;
    } else {
        /* Minus byte of 0xFF */
        out_o_coap_payload->len = temp_payload_len - 1;
        out_o_coap_payload->ptr = temp_payload_ptr;
    }

    return OscoreNoError;
}

/**
 * @brief Generate CoAP packet from OSCORE packet
 * @param decrypted_payload: decrypted OSCORE payload, which contains code, E-options and original unprotected CoAP payload
 * @param in_oscore_packet:  input OSCORE packet
 * @param out_o_coap_packet: pointer to output CoAP packet
 * @return
 */
static inline OscoreError o_coap_pkg_generate(
    struct byte_array* decrypted_payload,
    struct o_coap_packet* in_oscore_packet,
    struct o_coap_packet* out_o_coap_packet) {
    OscoreError r;
    uint8_t code = 0;
    struct byte_array unprotected_o_coap_payload = {
        .len = 0,
        .ptr = NULL,
    };
    struct o_coap_option E_options[10];
    uint8_t E_options_cnt = 0;

    /* Parse decrypted payload: code + options + unprotected CoAP payload*/
    r = oscore_decrypted_payload_parser(
        decrypted_payload, &code, E_options,
        &E_options_cnt, &unprotected_o_coap_payload);
    if (r != OscoreNoError) return r;
    /* Copy each items from OSCORE packet to CoAP packet */
    /* Header */
    out_o_coap_packet->header.ver = in_oscore_packet->header.ver;
    out_o_coap_packet->header.type = in_oscore_packet->header.type;
    out_o_coap_packet->header.TKL = in_oscore_packet->header.TKL;
    out_o_coap_packet->header.code = code;
    out_o_coap_packet->header.MID = in_oscore_packet->header.MID;

    /* Token */
    if (in_oscore_packet->header.TKL == 0)
        out_o_coap_packet->token = NULL;
    else
        out_o_coap_packet->token = in_oscore_packet->token;

    /* Payload */
    out_o_coap_packet->payload_len = unprotected_o_coap_payload.len;
    if (unprotected_o_coap_payload.len == 0)
        out_o_coap_packet->payload = NULL;
    else
        out_o_coap_packet->payload = unprotected_o_coap_payload.ptr;

    /* reorder all options, and copy it to output coap packet */
    options_from_oscore_reorder(in_oscore_packet, E_options, E_options_cnt,
                                out_o_coap_packet);
    return OscoreNoError;
}

OscoreError oscore2coap(
    uint8_t* buf_in, uint16_t buf_in_len,
    uint8_t* buf_out, uint16_t* buf_out_len,
    bool* oscore_pkg_flag, struct context* c) {
    uint8_t r = OscoreNoError;
    struct o_coap_packet oscore_packet;
    struct compressed_oscore_option oscore_option;
    struct byte_array buf;

    PRINT_MSG("\n\n\noscore2coap*******************************************\n");
    PRINT_ARRAY("Input OSCORE packet", buf_in, buf_in_len);

    buf.ptr = buf_in;
    buf.len = buf_in_len;

    /*Parse the incoming message (buf_in) into a CoAP struct*/
    r = buf2coap(&buf, &oscore_packet);
    if (r != OscoreNoError) return r;

    /* Check if the packet is OSCORE packet and if so parse the OSCORE option */
    r = oscore_option_parser(&oscore_packet, &oscore_option, oscore_pkg_flag);
    if (r != OscoreNoError) return r;

    /* If the incoming packet is OSCORE packet -- analyze and and decrypt it. */
    if (*oscore_pkg_flag) {
        /*In requests the OSCORE packet contains at least a KID = sender ID 
        and eventually sender sequence number*/
        if ((CODE_CLASS_MASK & oscore_packet.header.code) == REQUEST_CLASS) {
            /*Check that the recipient context c->rc has a  Recipient ID that
			 matches the received with the oscore option KID (Sender ID).
			 If this is not true return an error which indicates the caller
			 application to tray another context. This is useful when the caller
			 app doesn't know in advance to which context an incoming packet 
             belongs.*/
            if (!array_equals(&c->rc.recipient_id, &oscore_option.kid)) {
                return OscoreKidRecipentIdMismatch;
            }

            /*If this is a request message we need to calculate the nonce, aad 
            and eventually update the Common IV, Sender and Recipient Keys*/
            r = context_update(
                SERVER,
                (struct o_coap_option*)&oscore_packet.options,
                oscore_packet.options_cnt,
                &oscore_option.piv,
                &oscore_option.kid_context, c);
            if (r != OscoreNoError) return r;
        }

        /* Setup buffer for the plaintext. The plaintext is shorter than the ciphertext because of the authentication tag*/
        uint8_t plaintext_bytes[oscore_packet.payload_len - AUTH_TAG_LEN];
        struct byte_array plaintext = {
            .len = sizeof(plaintext_bytes),
            .ptr = plaintext_bytes,
        };

        /* Decrypt payload */
        r = payload_decrypt(c, &plaintext, &oscore_packet);
        if (r != OscoreNoError) return r;

        /* Generate corresponding CoAP packet */
        struct o_coap_packet o_coap_packet;
        r = o_coap_pkg_generate(&plaintext, &oscore_packet, &o_coap_packet);
        if (r != OscoreNoError) return r;

        /*Convert to byte string*/
        r = coap2buf(&o_coap_packet, buf_out, buf_out_len);
    }
    return r;
}
