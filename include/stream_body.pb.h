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
/* Generated by nanopb-0.3.9.2 at Mon Nov 19 21:19:17 2018. */

#ifndef PB_GOOGLE_RPC_STREAM_BODY_PB_H_INCLUDED
#define PB_GOOGLE_RPC_STREAM_BODY_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _google_rpc_Status {
    int32_t code;
    pb_callback_t message;
/* @@protoc_insertion_point(struct:google_rpc_Status) */
} google_rpc_Status;

typedef struct _google_rpc_StreamBody {
    pb_callback_t message;
    google_rpc_Status status;
    pb_callback_t noop;
/* @@protoc_insertion_point(struct:google_rpc_StreamBody) */
} google_rpc_StreamBody;

/* Default values for struct fields */

/* Initializer values for message structs */
#define google_rpc_Status_init_default           {0, {{NULL}, NULL}}
#define google_rpc_StreamBody_init_default       {{{NULL}, NULL}, google_rpc_Status_init_default, {{NULL}, NULL}}
#define google_rpc_Status_init_zero              {0, {{NULL}, NULL}}
#define google_rpc_StreamBody_init_zero          {{{NULL}, NULL}, google_rpc_Status_init_zero, {{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define google_rpc_Status_code_tag               1
#define google_rpc_Status_message_tag            2
#define google_rpc_StreamBody_message_tag        1
#define google_rpc_StreamBody_status_tag         2
#define google_rpc_StreamBody_noop_tag           15

/* Struct field encoding specification for nanopb */
extern const pb_field_t google_rpc_Status_fields[3];
extern const pb_field_t google_rpc_StreamBody_fields[4];

/* Maximum encoded size of messages (where known) */
/* google_rpc_Status_size depends on runtime parameters */
/* google_rpc_StreamBody_size depends on runtime parameters */

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define STREAM_BODY_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif
