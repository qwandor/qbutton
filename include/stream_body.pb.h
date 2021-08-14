// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.1 */

#ifndef PB_GOOGLE_RPC_GOOGLE_STREAM_BODY_PB_H_INCLUDED
#define PB_GOOGLE_RPC_GOOGLE_STREAM_BODY_PB_H_INCLUDED
#include <pb.h>
#include "any.pb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _google_rpc_Status {
    int32_t code;
    pb_callback_t message;
    pb_callback_t details;
} google_rpc_Status;

typedef struct _google_rpc_StreamBody {
    pb_callback_t message;
    bool has_status;
    google_rpc_Status status;
    pb_callback_t noop;
} google_rpc_StreamBody;


/* Initializer values for message structs */
#define google_rpc_Status_init_default           {0, {{NULL}, NULL}, {{NULL}, NULL}}
#define google_rpc_StreamBody_init_default       {{{NULL}, NULL}, false, google_rpc_Status_init_default, {{NULL}, NULL}}
#define google_rpc_Status_init_zero              {0, {{NULL}, NULL}, {{NULL}, NULL}}
#define google_rpc_StreamBody_init_zero          {{{NULL}, NULL}, false, google_rpc_Status_init_zero, {{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define google_rpc_Status_code_tag               1
#define google_rpc_Status_message_tag            2
#define google_rpc_Status_details_tag            3
#define google_rpc_StreamBody_message_tag        1
#define google_rpc_StreamBody_status_tag         2
#define google_rpc_StreamBody_noop_tag           15

/* Struct field encoding specification for nanopb */
#define google_rpc_Status_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    code,              1) \
X(a, CALLBACK, SINGULAR, STRING,   message,           2) \
X(a, CALLBACK, REPEATED, MESSAGE,  details,           3)
#define google_rpc_Status_CALLBACK pb_default_field_callback
#define google_rpc_Status_DEFAULT NULL
#define google_rpc_Status_details_MSGTYPE google_protobuf_Any

#define google_rpc_StreamBody_FIELDLIST(X, a) \
X(a, CALLBACK, REPEATED, BYTES,    message,           1) \
X(a, STATIC,   OPTIONAL, MESSAGE,  status,            2) \
X(a, CALLBACK, REPEATED, BYTES,    noop,             15)
#define google_rpc_StreamBody_CALLBACK pb_default_field_callback
#define google_rpc_StreamBody_DEFAULT NULL
#define google_rpc_StreamBody_status_MSGTYPE google_rpc_Status

extern const pb_msgdesc_t google_rpc_Status_msg;
extern const pb_msgdesc_t google_rpc_StreamBody_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define google_rpc_Status_fields &google_rpc_Status_msg
#define google_rpc_StreamBody_fields &google_rpc_StreamBody_msg

/* Maximum encoded size of messages (where known) */
/* google_rpc_Status_size depends on runtime parameters */
/* google_rpc_StreamBody_size depends on runtime parameters */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif