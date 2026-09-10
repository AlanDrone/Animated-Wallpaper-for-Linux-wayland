#ifndef MPV_CLIENT_H_
#define MPV_CLIENT_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mpv_handle mpv_handle;

typedef enum mpv_error {
    MPV_ERROR_SUCCESS           = 0,
    MPV_ERROR_EVENT_QUEUE_FULL  = -1,
    MPV_ERROR_NOMEM             = -2,
    MPV_ERROR_UNINITIALIZED     = -3,
    MPV_ERROR_INVALID_PARAMETER = -4,
    MPV_ERROR_OPTION_NOT_FOUND  = -5,
    MPV_ERROR_OPTION_FORMAT     = -6,
    MPV_ERROR_OPTION_ERROR      = -7,
    MPV_ERROR_PROPERTY_NOT_FOUND= -8,
    MPV_ERROR_PROPERTY_FORMAT   = -9,
    MPV_ERROR_PROPERTY_UNAVAILABLE= -10,
    MPV_ERROR_PROPERTY_ERROR    = -11,
    MPV_ERROR_COMMAND           = -12,
    MPV_ERROR_LOADING_FAILED    = -13,
    MPV_ERROR_AO_INIT_FAILED    = -14,
    MPV_ERROR_VO_INIT_FAILED    = -15,
    MPV_ERROR_NOT_IMPLEMENTED   = -16,
    MPV_ERROR_UNKNOWN_FORMAT    = -17,
    MPV_ERROR_GENERIC           = -18
} mpv_error;

typedef enum mpv_format {
    MPV_FORMAT_NONE             = 0,
    MPV_FORMAT_STRING           = 1,
    MPV_FORMAT_OSD_STRING       = 2,
    MPV_FORMAT_FLAG             = 3,
    MPV_FORMAT_INT64            = 4,
    MPV_FORMAT_DOUBLE           = 5,
    MPV_FORMAT_NODE             = 6,
    MPV_FORMAT_NODE_ARRAY       = 7,
    MPV_FORMAT_NODE_MAP         = 8,
    MPV_FORMAT_BYTE_ARRAY       = 9
} mpv_format;

typedef enum mpv_event_id {
    MPV_EVENT_NONE              = 0,
    MPV_EVENT_SHUTDOWN          = 1,
    MPV_EVENT_LOG_MESSAGE       = 2,
    MPV_EVENT_GET_PROPERTY_REPLY= 3,
    MPV_EVENT_SET_PROPERTY_REPLY= 4,
    MPV_EVENT_COMMAND_REPLY     = 5,
    MPV_EVENT_START_FILE        = 6,
    MPV_EVENT_END_FILE          = 7,
    MPV_EVENT_FILE_LOADED       = 8,
    MPV_EVENT_IDLE              = 11,
    MPV_EVENT_TICK              = 14,
    MPV_EVENT_SCRIPT_INPUT_DISPATCH = 15,
    MPV_EVENT_CLIENT_MESSAGE    = 16,
    MPV_EVENT_VIDEO_RECONFIG    = 17,
    MPV_EVENT_AUDIO_RECONFIG    = 18,
    MPV_EVENT_SEEK              = 20,
    MPV_EVENT_PLAYBACK_RESTART  = 21,
    MPV_EVENT_PROPERTY_CHANGE   = 22,
    MPV_EVENT_QUEUE_OVERFLOW    = 24,
    MPV_EVENT_HOOK              = 25
} mpv_event_id;

typedef struct mpv_event_property {
    const char *name;
    mpv_format format;
    void *data;
} mpv_event_property;

typedef struct mpv_event {
    mpv_event_id event_id;
    int error;
    uint64_t reply_userdata;
    void *data;
} mpv_event;

const char *mpv_error_string(int error);
mpv_handle *mpv_create(void);
int mpv_initialize(mpv_handle *ctx);
void mpv_destroy(mpv_handle *ctx);
void mpv_terminate_destroy(mpv_handle *ctx);
int mpv_command(mpv_handle *ctx, const char **args);
int mpv_command_string(mpv_handle *ctx, const char *args);
int mpv_command_async(mpv_handle *ctx, uint64_t reply_userdata, const char **args);
int mpv_set_option(mpv_handle *ctx, const char *name, mpv_format format, void *data);
int mpv_set_option_string(mpv_handle *ctx, const char *name, const char *data);
int mpv_get_property(mpv_handle *ctx, const char *name, mpv_format format, void *data);
char *mpv_get_property_string(mpv_handle *ctx, const char *name);
int mpv_set_property(mpv_handle *ctx, const char *name, mpv_format format, void *data);
int mpv_set_property_string(mpv_handle *ctx, const char *name, const char *data);
int mpv_observe_property(mpv_handle *mpv, uint64_t reply_userdata, const char *name, mpv_format format);
mpv_event *mpv_wait_event(mpv_handle *ctx, double timeout);
void mpv_wakeup(mpv_handle *ctx);
void mpv_free(void *data);

#ifdef __cplusplus
}
#endif

#endif
