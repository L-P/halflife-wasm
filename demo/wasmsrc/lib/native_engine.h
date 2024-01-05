// Mapped 1:1 to ALERT_TYPE in eiface.h.
typedef enum {
    log_debug = 2,
    log_info  = 1,
    log_warn  = 3,
    log_error = 4,
} log_level_t;

// Mapped 1:1 to USE_TYPE in cbase.h.
typedef enum {
    use_off    = 0,
    use_on     = 1,
    use_set    = 2,
    use_toggle = 3,
} use_type_t;

typedef enum {
    chan_auto   = 0,
    chan_weapon = 1,
    chan_voice  = 2,
    chan_item   = 3,
    chan_body   = 4,
    chan_stream = 5,
    chan_static = 6,
} sound_channel_t;

static const float sound_att_norm = 0.8f;

void make_follow(const char* target);
float global_time(void);
void console_log(log_level_t log_level, const char* msg);
void ent_fire(const char* target, use_type_t use_type, float value);
void ent_kill(const char* target);
void ent_move(const char* target, float x, float y, float z);

// Sound file path is relative to the sound/ dir.
void precache_sound(const char* path);

// Defaults to emitting sound from player if emitterTargetName is NULL.
void play_sound(const char* path, const char* emitterTargetName, sound_channel_t channel, float volume, float attenuation);
