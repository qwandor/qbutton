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

#ifndef PB_GOOGLE_ASSISTANT_EMBEDDED_V1ALPHA2_GOOGLE_ASSISTANT_EMBEDDED_V1ALPHA2_EMBEDDED_ASSISTANT_PB_H_INCLUDED
#define PB_GOOGLE_ASSISTANT_EMBEDDED_V1ALPHA2_GOOGLE_ASSISTANT_EMBEDDED_V1ALPHA2_EMBEDDED_ASSISTANT_PB_H_INCLUDED
#include <pb.h>
#include "annotations.pb.h"
#include "latlng.pb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _google_assistant_embedded_v1alpha2_AssistResponse_EventType {
    google_assistant_embedded_v1alpha2_AssistResponse_EventType_EVENT_TYPE_UNSPECIFIED = 0,
    google_assistant_embedded_v1alpha2_AssistResponse_EventType_END_OF_UTTERANCE = 1
} google_assistant_embedded_v1alpha2_AssistResponse_EventType;

typedef enum _google_assistant_embedded_v1alpha2_AudioInConfig_Encoding {
    google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_ENCODING_UNSPECIFIED = 0,
    google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_LINEAR16 = 1,
    google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_FLAC = 2
} google_assistant_embedded_v1alpha2_AudioInConfig_Encoding;

typedef enum _google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding {
    google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_ENCODING_UNSPECIFIED = 0,
    google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_LINEAR16 = 1,
    google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_MP3 = 2,
    google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_OPUS_IN_OGG = 3
} google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding;

typedef enum _google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode {
    google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_SCREEN_MODE_UNSPECIFIED = 0,
    google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_OFF = 1,
    google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_PLAYING = 3
} google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode;

typedef enum _google_assistant_embedded_v1alpha2_ScreenOut_Format {
    google_assistant_embedded_v1alpha2_ScreenOut_Format_FORMAT_UNSPECIFIED = 0,
    google_assistant_embedded_v1alpha2_ScreenOut_Format_HTML = 1
} google_assistant_embedded_v1alpha2_ScreenOut_Format;

typedef enum _google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode {
    google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode_MICROPHONE_MODE_UNSPECIFIED = 0,
    google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode_CLOSE_MICROPHONE = 1,
    google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode_DIALOG_FOLLOW_ON = 2
} google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode;

/* Struct definitions */
typedef struct _google_assistant_embedded_v1alpha2_AudioOut {
    pb_callback_t audio_data;
} google_assistant_embedded_v1alpha2_AudioOut;

typedef struct _google_assistant_embedded_v1alpha2_DebugInfo {
    pb_callback_t aog_agent_to_assistant_json;
} google_assistant_embedded_v1alpha2_DebugInfo;

typedef struct _google_assistant_embedded_v1alpha2_DeviceAction {
    pb_callback_t device_request_json;
} google_assistant_embedded_v1alpha2_DeviceAction;

typedef struct _google_assistant_embedded_v1alpha2_AudioInConfig {
    google_assistant_embedded_v1alpha2_AudioInConfig_Encoding encoding;
    int32_t sample_rate_hertz;
} google_assistant_embedded_v1alpha2_AudioInConfig;

typedef struct _google_assistant_embedded_v1alpha2_AudioOutConfig {
    google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding encoding;
    int32_t sample_rate_hertz;
    int32_t volume_percentage;
} google_assistant_embedded_v1alpha2_AudioOutConfig;

typedef struct _google_assistant_embedded_v1alpha2_DebugConfig {
    bool return_debug_info;
} google_assistant_embedded_v1alpha2_DebugConfig;

typedef struct _google_assistant_embedded_v1alpha2_DeviceConfig {
    char device_id[20];
    char device_model_id[40];
} google_assistant_embedded_v1alpha2_DeviceConfig;

typedef struct _google_assistant_embedded_v1alpha2_DeviceLocation {
    pb_size_t which_type;
    union {
        google_type_LatLng coordinates;
    } type;
} google_assistant_embedded_v1alpha2_DeviceLocation;

typedef struct _google_assistant_embedded_v1alpha2_DialogStateOut {
    pb_callback_t supplemental_display_text;
    pb_callback_t conversation_state;
    google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode microphone_mode;
    int32_t volume_percentage;
} google_assistant_embedded_v1alpha2_DialogStateOut;

typedef struct _google_assistant_embedded_v1alpha2_ScreenOut {
    google_assistant_embedded_v1alpha2_ScreenOut_Format format;
    pb_callback_t data;
} google_assistant_embedded_v1alpha2_ScreenOut;

typedef struct _google_assistant_embedded_v1alpha2_ScreenOutConfig {
    google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode screen_mode;
} google_assistant_embedded_v1alpha2_ScreenOutConfig;

typedef struct _google_assistant_embedded_v1alpha2_SpeechRecognitionResult {
    pb_callback_t transcript;
    float stability;
} google_assistant_embedded_v1alpha2_SpeechRecognitionResult;

typedef struct _google_assistant_embedded_v1alpha2_AssistResponse {
    google_assistant_embedded_v1alpha2_AssistResponse_EventType event_type;
    pb_callback_t speech_results;
    bool has_audio_out;
    google_assistant_embedded_v1alpha2_AudioOut audio_out;
    bool has_screen_out;
    google_assistant_embedded_v1alpha2_ScreenOut screen_out;
    bool has_dialog_state_out;
    google_assistant_embedded_v1alpha2_DialogStateOut dialog_state_out;
    bool has_device_action;
    google_assistant_embedded_v1alpha2_DeviceAction device_action;
    bool has_debug_info;
    google_assistant_embedded_v1alpha2_DebugInfo debug_info;
} google_assistant_embedded_v1alpha2_AssistResponse;

typedef struct _google_assistant_embedded_v1alpha2_DialogStateIn {
    pb_callback_t conversation_state;
    char language_code[6];
    bool has_device_location;
    google_assistant_embedded_v1alpha2_DeviceLocation device_location;
    bool is_new_conversation;
} google_assistant_embedded_v1alpha2_DialogStateIn;

typedef struct _google_assistant_embedded_v1alpha2_AssistConfig {
    pb_size_t which_type;
    union {
        google_assistant_embedded_v1alpha2_AudioInConfig audio_in_config;
        char text_query[200];
    } type;
    bool has_audio_out_config;
    google_assistant_embedded_v1alpha2_AudioOutConfig audio_out_config;
    bool has_dialog_state_in;
    google_assistant_embedded_v1alpha2_DialogStateIn dialog_state_in;
    bool has_device_config;
    google_assistant_embedded_v1alpha2_DeviceConfig device_config;
    bool has_debug_config;
    google_assistant_embedded_v1alpha2_DebugConfig debug_config;
    bool has_screen_out_config;
    google_assistant_embedded_v1alpha2_ScreenOutConfig screen_out_config;
} google_assistant_embedded_v1alpha2_AssistConfig;

typedef struct _google_assistant_embedded_v1alpha2_AssistRequest {
    pb_size_t which_type;
    union {
        google_assistant_embedded_v1alpha2_AssistConfig config;
        pb_callback_t audio_in;
    } type;
} google_assistant_embedded_v1alpha2_AssistRequest;


/* Helper constants for enums */
#define _google_assistant_embedded_v1alpha2_AssistResponse_EventType_MIN google_assistant_embedded_v1alpha2_AssistResponse_EventType_EVENT_TYPE_UNSPECIFIED
#define _google_assistant_embedded_v1alpha2_AssistResponse_EventType_MAX google_assistant_embedded_v1alpha2_AssistResponse_EventType_END_OF_UTTERANCE
#define _google_assistant_embedded_v1alpha2_AssistResponse_EventType_ARRAYSIZE ((google_assistant_embedded_v1alpha2_AssistResponse_EventType)(google_assistant_embedded_v1alpha2_AssistResponse_EventType_END_OF_UTTERANCE+1))

#define _google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_MIN google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_ENCODING_UNSPECIFIED
#define _google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_MAX google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_FLAC
#define _google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_ARRAYSIZE ((google_assistant_embedded_v1alpha2_AudioInConfig_Encoding)(google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_FLAC+1))

#define _google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_MIN google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_ENCODING_UNSPECIFIED
#define _google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_MAX google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_OPUS_IN_OGG
#define _google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_ARRAYSIZE ((google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding)(google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_OPUS_IN_OGG+1))

#define _google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_MIN google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_SCREEN_MODE_UNSPECIFIED
#define _google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_MAX google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_PLAYING
#define _google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_ARRAYSIZE ((google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode)(google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_PLAYING+1))

#define _google_assistant_embedded_v1alpha2_ScreenOut_Format_MIN google_assistant_embedded_v1alpha2_ScreenOut_Format_FORMAT_UNSPECIFIED
#define _google_assistant_embedded_v1alpha2_ScreenOut_Format_MAX google_assistant_embedded_v1alpha2_ScreenOut_Format_HTML
#define _google_assistant_embedded_v1alpha2_ScreenOut_Format_ARRAYSIZE ((google_assistant_embedded_v1alpha2_ScreenOut_Format)(google_assistant_embedded_v1alpha2_ScreenOut_Format_HTML+1))

#define _google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode_MIN google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode_MICROPHONE_MODE_UNSPECIFIED
#define _google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode_MAX google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode_DIALOG_FOLLOW_ON
#define _google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode_ARRAYSIZE ((google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode)(google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode_DIALOG_FOLLOW_ON+1))


/* Initializer values for message structs */
#define google_assistant_embedded_v1alpha2_AssistRequest_init_default {0, {google_assistant_embedded_v1alpha2_AssistConfig_init_default}}
#define google_assistant_embedded_v1alpha2_AssistResponse_init_default {_google_assistant_embedded_v1alpha2_AssistResponse_EventType_MIN, {{NULL}, NULL}, false, google_assistant_embedded_v1alpha2_AudioOut_init_default, false, google_assistant_embedded_v1alpha2_ScreenOut_init_default, false, google_assistant_embedded_v1alpha2_DialogStateOut_init_default, false, google_assistant_embedded_v1alpha2_DeviceAction_init_default, false, google_assistant_embedded_v1alpha2_DebugInfo_init_default}
#define google_assistant_embedded_v1alpha2_DebugInfo_init_default {{{NULL}, NULL}}
#define google_assistant_embedded_v1alpha2_AssistConfig_init_default {0, {google_assistant_embedded_v1alpha2_AudioInConfig_init_default}, false, google_assistant_embedded_v1alpha2_AudioOutConfig_init_default, false, google_assistant_embedded_v1alpha2_DialogStateIn_init_default, false, google_assistant_embedded_v1alpha2_DeviceConfig_init_default, false, google_assistant_embedded_v1alpha2_DebugConfig_init_default, false, google_assistant_embedded_v1alpha2_ScreenOutConfig_init_default}
#define google_assistant_embedded_v1alpha2_AudioInConfig_init_default {_google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_MIN, 0}
#define google_assistant_embedded_v1alpha2_AudioOutConfig_init_default {_google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_MIN, 0, 0}
#define google_assistant_embedded_v1alpha2_ScreenOutConfig_init_default {_google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_MIN}
#define google_assistant_embedded_v1alpha2_DialogStateIn_init_default {{{NULL}, NULL}, "", false, google_assistant_embedded_v1alpha2_DeviceLocation_init_default, 0}
#define google_assistant_embedded_v1alpha2_DeviceConfig_init_default {"", ""}
#define google_assistant_embedded_v1alpha2_AudioOut_init_default {{{NULL}, NULL}}
#define google_assistant_embedded_v1alpha2_ScreenOut_init_default {_google_assistant_embedded_v1alpha2_ScreenOut_Format_MIN, {{NULL}, NULL}}
#define google_assistant_embedded_v1alpha2_DeviceAction_init_default {{{NULL}, NULL}}
#define google_assistant_embedded_v1alpha2_SpeechRecognitionResult_init_default {{{NULL}, NULL}, 0}
#define google_assistant_embedded_v1alpha2_DialogStateOut_init_default {{{NULL}, NULL}, {{NULL}, NULL}, _google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode_MIN, 0}
#define google_assistant_embedded_v1alpha2_DebugConfig_init_default {0}
#define google_assistant_embedded_v1alpha2_DeviceLocation_init_default {0, {google_type_LatLng_init_default}}
#define google_assistant_embedded_v1alpha2_AssistRequest_init_zero {0, {google_assistant_embedded_v1alpha2_AssistConfig_init_zero}}
#define google_assistant_embedded_v1alpha2_AssistResponse_init_zero {_google_assistant_embedded_v1alpha2_AssistResponse_EventType_MIN, {{NULL}, NULL}, false, google_assistant_embedded_v1alpha2_AudioOut_init_zero, false, google_assistant_embedded_v1alpha2_ScreenOut_init_zero, false, google_assistant_embedded_v1alpha2_DialogStateOut_init_zero, false, google_assistant_embedded_v1alpha2_DeviceAction_init_zero, false, google_assistant_embedded_v1alpha2_DebugInfo_init_zero}
#define google_assistant_embedded_v1alpha2_DebugInfo_init_zero {{{NULL}, NULL}}
#define google_assistant_embedded_v1alpha2_AssistConfig_init_zero {0, {google_assistant_embedded_v1alpha2_AudioInConfig_init_zero}, false, google_assistant_embedded_v1alpha2_AudioOutConfig_init_zero, false, google_assistant_embedded_v1alpha2_DialogStateIn_init_zero, false, google_assistant_embedded_v1alpha2_DeviceConfig_init_zero, false, google_assistant_embedded_v1alpha2_DebugConfig_init_zero, false, google_assistant_embedded_v1alpha2_ScreenOutConfig_init_zero}
#define google_assistant_embedded_v1alpha2_AudioInConfig_init_zero {_google_assistant_embedded_v1alpha2_AudioInConfig_Encoding_MIN, 0}
#define google_assistant_embedded_v1alpha2_AudioOutConfig_init_zero {_google_assistant_embedded_v1alpha2_AudioOutConfig_Encoding_MIN, 0, 0}
#define google_assistant_embedded_v1alpha2_ScreenOutConfig_init_zero {_google_assistant_embedded_v1alpha2_ScreenOutConfig_ScreenMode_MIN}
#define google_assistant_embedded_v1alpha2_DialogStateIn_init_zero {{{NULL}, NULL}, "", false, google_assistant_embedded_v1alpha2_DeviceLocation_init_zero, 0}
#define google_assistant_embedded_v1alpha2_DeviceConfig_init_zero {"", ""}
#define google_assistant_embedded_v1alpha2_AudioOut_init_zero {{{NULL}, NULL}}
#define google_assistant_embedded_v1alpha2_ScreenOut_init_zero {_google_assistant_embedded_v1alpha2_ScreenOut_Format_MIN, {{NULL}, NULL}}
#define google_assistant_embedded_v1alpha2_DeviceAction_init_zero {{{NULL}, NULL}}
#define google_assistant_embedded_v1alpha2_SpeechRecognitionResult_init_zero {{{NULL}, NULL}, 0}
#define google_assistant_embedded_v1alpha2_DialogStateOut_init_zero {{{NULL}, NULL}, {{NULL}, NULL}, _google_assistant_embedded_v1alpha2_DialogStateOut_MicrophoneMode_MIN, 0}
#define google_assistant_embedded_v1alpha2_DebugConfig_init_zero {0}
#define google_assistant_embedded_v1alpha2_DeviceLocation_init_zero {0, {google_type_LatLng_init_zero}}

/* Field tags (for use in manual encoding/decoding) */
#define google_assistant_embedded_v1alpha2_AudioOut_audio_data_tag 1
#define google_assistant_embedded_v1alpha2_DebugInfo_aog_agent_to_assistant_json_tag 1
#define google_assistant_embedded_v1alpha2_DeviceAction_device_request_json_tag 1
#define google_assistant_embedded_v1alpha2_AudioInConfig_encoding_tag 1
#define google_assistant_embedded_v1alpha2_AudioInConfig_sample_rate_hertz_tag 2
#define google_assistant_embedded_v1alpha2_AudioOutConfig_encoding_tag 1
#define google_assistant_embedded_v1alpha2_AudioOutConfig_sample_rate_hertz_tag 2
#define google_assistant_embedded_v1alpha2_AudioOutConfig_volume_percentage_tag 3
#define google_assistant_embedded_v1alpha2_DebugConfig_return_debug_info_tag 6
#define google_assistant_embedded_v1alpha2_DeviceConfig_device_id_tag 1
#define google_assistant_embedded_v1alpha2_DeviceConfig_device_model_id_tag 3
#define google_assistant_embedded_v1alpha2_DeviceLocation_coordinates_tag 1
#define google_assistant_embedded_v1alpha2_DialogStateOut_supplemental_display_text_tag 1
#define google_assistant_embedded_v1alpha2_DialogStateOut_conversation_state_tag 2
#define google_assistant_embedded_v1alpha2_DialogStateOut_microphone_mode_tag 3
#define google_assistant_embedded_v1alpha2_DialogStateOut_volume_percentage_tag 4
#define google_assistant_embedded_v1alpha2_ScreenOut_format_tag 1
#define google_assistant_embedded_v1alpha2_ScreenOut_data_tag 2
#define google_assistant_embedded_v1alpha2_ScreenOutConfig_screen_mode_tag 1
#define google_assistant_embedded_v1alpha2_SpeechRecognitionResult_transcript_tag 1
#define google_assistant_embedded_v1alpha2_SpeechRecognitionResult_stability_tag 2
#define google_assistant_embedded_v1alpha2_AssistResponse_event_type_tag 1
#define google_assistant_embedded_v1alpha2_AssistResponse_audio_out_tag 3
#define google_assistant_embedded_v1alpha2_AssistResponse_screen_out_tag 4
#define google_assistant_embedded_v1alpha2_AssistResponse_device_action_tag 6
#define google_assistant_embedded_v1alpha2_AssistResponse_speech_results_tag 2
#define google_assistant_embedded_v1alpha2_AssistResponse_dialog_state_out_tag 5
#define google_assistant_embedded_v1alpha2_AssistResponse_debug_info_tag 8
#define google_assistant_embedded_v1alpha2_DialogStateIn_conversation_state_tag 1
#define google_assistant_embedded_v1alpha2_DialogStateIn_language_code_tag 2
#define google_assistant_embedded_v1alpha2_DialogStateIn_device_location_tag 5
#define google_assistant_embedded_v1alpha2_DialogStateIn_is_new_conversation_tag 7
#define google_assistant_embedded_v1alpha2_AssistConfig_audio_in_config_tag 1
#define google_assistant_embedded_v1alpha2_AssistConfig_text_query_tag 6
#define google_assistant_embedded_v1alpha2_AssistConfig_audio_out_config_tag 2
#define google_assistant_embedded_v1alpha2_AssistConfig_screen_out_config_tag 8
#define google_assistant_embedded_v1alpha2_AssistConfig_dialog_state_in_tag 3
#define google_assistant_embedded_v1alpha2_AssistConfig_device_config_tag 4
#define google_assistant_embedded_v1alpha2_AssistConfig_debug_config_tag 5
#define google_assistant_embedded_v1alpha2_AssistRequest_config_tag 1
#define google_assistant_embedded_v1alpha2_AssistRequest_audio_in_tag 2

/* Struct field encoding specification for nanopb */
#define google_assistant_embedded_v1alpha2_AssistRequest_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,config,type.config),   1) \
X(a, CALLBACK, ONEOF,    BYTES,    (type,audio_in,type.audio_in),   2)
#define google_assistant_embedded_v1alpha2_AssistRequest_CALLBACK pb_default_field_callback
#define google_assistant_embedded_v1alpha2_AssistRequest_DEFAULT NULL
#define google_assistant_embedded_v1alpha2_AssistRequest_type_config_MSGTYPE google_assistant_embedded_v1alpha2_AssistConfig

#define google_assistant_embedded_v1alpha2_AssistResponse_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    event_type,        1) \
X(a, CALLBACK, REPEATED, MESSAGE,  speech_results,    2) \
X(a, STATIC,   OPTIONAL, MESSAGE,  audio_out,         3) \
X(a, STATIC,   OPTIONAL, MESSAGE,  screen_out,        4) \
X(a, STATIC,   OPTIONAL, MESSAGE,  dialog_state_out,   5) \
X(a, STATIC,   OPTIONAL, MESSAGE,  device_action,     6) \
X(a, STATIC,   OPTIONAL, MESSAGE,  debug_info,        8)
#define google_assistant_embedded_v1alpha2_AssistResponse_CALLBACK pb_default_field_callback
#define google_assistant_embedded_v1alpha2_AssistResponse_DEFAULT NULL
#define google_assistant_embedded_v1alpha2_AssistResponse_speech_results_MSGTYPE google_assistant_embedded_v1alpha2_SpeechRecognitionResult
#define google_assistant_embedded_v1alpha2_AssistResponse_audio_out_MSGTYPE google_assistant_embedded_v1alpha2_AudioOut
#define google_assistant_embedded_v1alpha2_AssistResponse_screen_out_MSGTYPE google_assistant_embedded_v1alpha2_ScreenOut
#define google_assistant_embedded_v1alpha2_AssistResponse_dialog_state_out_MSGTYPE google_assistant_embedded_v1alpha2_DialogStateOut
#define google_assistant_embedded_v1alpha2_AssistResponse_device_action_MSGTYPE google_assistant_embedded_v1alpha2_DeviceAction
#define google_assistant_embedded_v1alpha2_AssistResponse_debug_info_MSGTYPE google_assistant_embedded_v1alpha2_DebugInfo

#define google_assistant_embedded_v1alpha2_DebugInfo_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   aog_agent_to_assistant_json,   1)
#define google_assistant_embedded_v1alpha2_DebugInfo_CALLBACK pb_default_field_callback
#define google_assistant_embedded_v1alpha2_DebugInfo_DEFAULT NULL

#define google_assistant_embedded_v1alpha2_AssistConfig_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,audio_in_config,type.audio_in_config),   1) \
X(a, STATIC,   ONEOF,    STRING,   (type,text_query,type.text_query),   6) \
X(a, STATIC,   OPTIONAL, MESSAGE,  audio_out_config,   2) \
X(a, STATIC,   OPTIONAL, MESSAGE,  dialog_state_in,   3) \
X(a, STATIC,   OPTIONAL, MESSAGE,  device_config,     4) \
X(a, STATIC,   OPTIONAL, MESSAGE,  debug_config,      5) \
X(a, STATIC,   OPTIONAL, MESSAGE,  screen_out_config,   8)
#define google_assistant_embedded_v1alpha2_AssistConfig_CALLBACK NULL
#define google_assistant_embedded_v1alpha2_AssistConfig_DEFAULT NULL
#define google_assistant_embedded_v1alpha2_AssistConfig_type_audio_in_config_MSGTYPE google_assistant_embedded_v1alpha2_AudioInConfig
#define google_assistant_embedded_v1alpha2_AssistConfig_audio_out_config_MSGTYPE google_assistant_embedded_v1alpha2_AudioOutConfig
#define google_assistant_embedded_v1alpha2_AssistConfig_dialog_state_in_MSGTYPE google_assistant_embedded_v1alpha2_DialogStateIn
#define google_assistant_embedded_v1alpha2_AssistConfig_device_config_MSGTYPE google_assistant_embedded_v1alpha2_DeviceConfig
#define google_assistant_embedded_v1alpha2_AssistConfig_debug_config_MSGTYPE google_assistant_embedded_v1alpha2_DebugConfig
#define google_assistant_embedded_v1alpha2_AssistConfig_screen_out_config_MSGTYPE google_assistant_embedded_v1alpha2_ScreenOutConfig

#define google_assistant_embedded_v1alpha2_AudioInConfig_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    encoding,          1) \
X(a, STATIC,   SINGULAR, INT32,    sample_rate_hertz,   2)
#define google_assistant_embedded_v1alpha2_AudioInConfig_CALLBACK NULL
#define google_assistant_embedded_v1alpha2_AudioInConfig_DEFAULT NULL

#define google_assistant_embedded_v1alpha2_AudioOutConfig_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    encoding,          1) \
X(a, STATIC,   SINGULAR, INT32,    sample_rate_hertz,   2) \
X(a, STATIC,   SINGULAR, INT32,    volume_percentage,   3)
#define google_assistant_embedded_v1alpha2_AudioOutConfig_CALLBACK NULL
#define google_assistant_embedded_v1alpha2_AudioOutConfig_DEFAULT NULL

#define google_assistant_embedded_v1alpha2_ScreenOutConfig_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    screen_mode,       1)
#define google_assistant_embedded_v1alpha2_ScreenOutConfig_CALLBACK NULL
#define google_assistant_embedded_v1alpha2_ScreenOutConfig_DEFAULT NULL

#define google_assistant_embedded_v1alpha2_DialogStateIn_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, BYTES,    conversation_state,   1) \
X(a, STATIC,   SINGULAR, STRING,   language_code,     2) \
X(a, STATIC,   OPTIONAL, MESSAGE,  device_location,   5) \
X(a, STATIC,   SINGULAR, BOOL,     is_new_conversation,   7)
#define google_assistant_embedded_v1alpha2_DialogStateIn_CALLBACK pb_default_field_callback
#define google_assistant_embedded_v1alpha2_DialogStateIn_DEFAULT NULL
#define google_assistant_embedded_v1alpha2_DialogStateIn_device_location_MSGTYPE google_assistant_embedded_v1alpha2_DeviceLocation

#define google_assistant_embedded_v1alpha2_DeviceConfig_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, STRING,   device_id,         1) \
X(a, STATIC,   SINGULAR, STRING,   device_model_id,   3)
#define google_assistant_embedded_v1alpha2_DeviceConfig_CALLBACK NULL
#define google_assistant_embedded_v1alpha2_DeviceConfig_DEFAULT NULL

#define google_assistant_embedded_v1alpha2_AudioOut_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, BYTES,    audio_data,        1)
#define google_assistant_embedded_v1alpha2_AudioOut_CALLBACK pb_default_field_callback
#define google_assistant_embedded_v1alpha2_AudioOut_DEFAULT NULL

#define google_assistant_embedded_v1alpha2_ScreenOut_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    format,            1) \
X(a, CALLBACK, SINGULAR, BYTES,    data,              2)
#define google_assistant_embedded_v1alpha2_ScreenOut_CALLBACK pb_default_field_callback
#define google_assistant_embedded_v1alpha2_ScreenOut_DEFAULT NULL

#define google_assistant_embedded_v1alpha2_DeviceAction_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   device_request_json,   1)
#define google_assistant_embedded_v1alpha2_DeviceAction_CALLBACK pb_default_field_callback
#define google_assistant_embedded_v1alpha2_DeviceAction_DEFAULT NULL

#define google_assistant_embedded_v1alpha2_SpeechRecognitionResult_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   transcript,        1) \
X(a, STATIC,   SINGULAR, FLOAT,    stability,         2)
#define google_assistant_embedded_v1alpha2_SpeechRecognitionResult_CALLBACK pb_default_field_callback
#define google_assistant_embedded_v1alpha2_SpeechRecognitionResult_DEFAULT NULL

#define google_assistant_embedded_v1alpha2_DialogStateOut_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   supplemental_display_text,   1) \
X(a, CALLBACK, SINGULAR, BYTES,    conversation_state,   2) \
X(a, STATIC,   SINGULAR, UENUM,    microphone_mode,   3) \
X(a, STATIC,   SINGULAR, INT32,    volume_percentage,   4)
#define google_assistant_embedded_v1alpha2_DialogStateOut_CALLBACK pb_default_field_callback
#define google_assistant_embedded_v1alpha2_DialogStateOut_DEFAULT NULL

#define google_assistant_embedded_v1alpha2_DebugConfig_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, BOOL,     return_debug_info,   6)
#define google_assistant_embedded_v1alpha2_DebugConfig_CALLBACK NULL
#define google_assistant_embedded_v1alpha2_DebugConfig_DEFAULT NULL

#define google_assistant_embedded_v1alpha2_DeviceLocation_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,coordinates,type.coordinates),   1)
#define google_assistant_embedded_v1alpha2_DeviceLocation_CALLBACK NULL
#define google_assistant_embedded_v1alpha2_DeviceLocation_DEFAULT NULL
#define google_assistant_embedded_v1alpha2_DeviceLocation_type_coordinates_MSGTYPE google_type_LatLng

extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_AssistRequest_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_AssistResponse_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_DebugInfo_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_AssistConfig_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_AudioInConfig_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_AudioOutConfig_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_ScreenOutConfig_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_DialogStateIn_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_DeviceConfig_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_AudioOut_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_ScreenOut_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_DeviceAction_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_SpeechRecognitionResult_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_DialogStateOut_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_DebugConfig_msg;
extern const pb_msgdesc_t google_assistant_embedded_v1alpha2_DeviceLocation_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define google_assistant_embedded_v1alpha2_AssistRequest_fields &google_assistant_embedded_v1alpha2_AssistRequest_msg
#define google_assistant_embedded_v1alpha2_AssistResponse_fields &google_assistant_embedded_v1alpha2_AssistResponse_msg
#define google_assistant_embedded_v1alpha2_DebugInfo_fields &google_assistant_embedded_v1alpha2_DebugInfo_msg
#define google_assistant_embedded_v1alpha2_AssistConfig_fields &google_assistant_embedded_v1alpha2_AssistConfig_msg
#define google_assistant_embedded_v1alpha2_AudioInConfig_fields &google_assistant_embedded_v1alpha2_AudioInConfig_msg
#define google_assistant_embedded_v1alpha2_AudioOutConfig_fields &google_assistant_embedded_v1alpha2_AudioOutConfig_msg
#define google_assistant_embedded_v1alpha2_ScreenOutConfig_fields &google_assistant_embedded_v1alpha2_ScreenOutConfig_msg
#define google_assistant_embedded_v1alpha2_DialogStateIn_fields &google_assistant_embedded_v1alpha2_DialogStateIn_msg
#define google_assistant_embedded_v1alpha2_DeviceConfig_fields &google_assistant_embedded_v1alpha2_DeviceConfig_msg
#define google_assistant_embedded_v1alpha2_AudioOut_fields &google_assistant_embedded_v1alpha2_AudioOut_msg
#define google_assistant_embedded_v1alpha2_ScreenOut_fields &google_assistant_embedded_v1alpha2_ScreenOut_msg
#define google_assistant_embedded_v1alpha2_DeviceAction_fields &google_assistant_embedded_v1alpha2_DeviceAction_msg
#define google_assistant_embedded_v1alpha2_SpeechRecognitionResult_fields &google_assistant_embedded_v1alpha2_SpeechRecognitionResult_msg
#define google_assistant_embedded_v1alpha2_DialogStateOut_fields &google_assistant_embedded_v1alpha2_DialogStateOut_msg
#define google_assistant_embedded_v1alpha2_DebugConfig_fields &google_assistant_embedded_v1alpha2_DebugConfig_msg
#define google_assistant_embedded_v1alpha2_DeviceLocation_fields &google_assistant_embedded_v1alpha2_DeviceLocation_msg

/* Maximum encoded size of messages (where known) */
/* google_assistant_embedded_v1alpha2_AssistRequest_size depends on runtime parameters */
/* google_assistant_embedded_v1alpha2_AssistResponse_size depends on runtime parameters */
/* google_assistant_embedded_v1alpha2_DebugInfo_size depends on runtime parameters */
/* google_assistant_embedded_v1alpha2_AssistConfig_size depends on runtime parameters */
#define google_assistant_embedded_v1alpha2_AudioInConfig_size 13
#define google_assistant_embedded_v1alpha2_AudioOutConfig_size 24
#define google_assistant_embedded_v1alpha2_ScreenOutConfig_size 2
/* google_assistant_embedded_v1alpha2_DialogStateIn_size depends on runtime parameters */
#define google_assistant_embedded_v1alpha2_DeviceConfig_size 62
/* google_assistant_embedded_v1alpha2_AudioOut_size depends on runtime parameters */
/* google_assistant_embedded_v1alpha2_ScreenOut_size depends on runtime parameters */
/* google_assistant_embedded_v1alpha2_DeviceAction_size depends on runtime parameters */
/* google_assistant_embedded_v1alpha2_SpeechRecognitionResult_size depends on runtime parameters */
/* google_assistant_embedded_v1alpha2_DialogStateOut_size depends on runtime parameters */
#define google_assistant_embedded_v1alpha2_DebugConfig_size 2
#define google_assistant_embedded_v1alpha2_DeviceLocation_size (5 + google_type_LatLng_size)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
