/* Copyright (c) 2011, TrafficLab, Ericsson Research, Hungary
 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Ericsson Research nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Author: Zoltán Lajos Kis <zoltan.lajos.kis@ericsson.com>
 */

#ifndef PACKET_H
#define PACKET_H 1

#include <stdbool.h>
#include "action_set.h"
#include "datapath.h"
#include "packet_handle_std.h"
#include "ofpbuf.h"
#include "oflib/ofl-structs.h"
#include "packets.h"


/****************************************************************************
 * Represents a packet received on the datapath, and its associated processing
 * state.
 ****************************************************************************/

#define L3_OPTION_FRAGMENT 0x01 << 0
#define L3_OPTION_IP_OPTION 0x01 << 1
#define L3_OPTION_MTU 0x01 << 2
#define L3_OPTION_TTL 0x01 << 3
#define L3_OPTION_NONE 0

struct packet {
    struct datapath    *dp;
    struct ofpbuf      *buffer;    /* buffer containing the packet */
    unsigned int            in_port;
    struct action_set  *action_set; /* action set associated with the packet */
    struct action_set  *action_set_apply; /* action set associated with the packet, include apply instruction*/
    bool                alta_supported;
    bool                packet_out; /* true if the packet arrived in a packet out msg */

    unsigned int            out_group; /* OFPG_ANY = no out group */
    unsigned int            out_port;  /* OFPP_ANY = no out port */
    unsigned short int            out_port_max_len;  /* max length to send, if out_port is OFPP_CONTROLLER */
    unsigned int            out_queue;
    unsigned char             table_id; /* table in which is processed */
    unsigned int            buffer_id; /* if packet is stored in buffer, buffer_id;
                                      otherwise OFP_NO_BUFFER */

    struct packet_handle_std  *handle_std; /* handler for standard match structure */
    bool mis_match_entry_hited;

    unsigned int l3_options;
    
    unsigned char             reason;  
    unsigned int            hash;
    unsigned long long int cookie;
};

/* Creates a packet. */
struct packet *
packet_create(struct datapath *dp, unsigned int in_port, struct ofpbuf *buf, bool packet_out);

/* Converts the packet to a string representation. */
char *
packet_to_string(struct packet *pkt);

/* Destroys a packet along with all its associated structures */
void
packet_destroy(struct packet *pkt);

/* Clones a packet deeply, i.e. all associated structures are also cloned. */
struct packet *
packet_clone(struct packet *pkt);

void 
packet_match_reset(struct packet *pkt);


#endif /* PACKET_H */
