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

// From const.h, to use with flags_*().
#define FL_FLY           (1 << 0)
#define FL_SWIM          (1 << 1)
#define FL_CONVEYOR      (1 << 2)
#define FL_CLIENT        (1 << 3)
#define FL_INWATER       (1 << 4)
#define FL_MONSTER       (1 << 5)
#define FL_GODMODE       (1 << 6)
#define FL_NOTARGET      (1 << 7)
#define FL_SKIPLOCALHOST (1 << 8)
#define FL_ONGROUND      (1 << 9)
#define FL_PARTIALGROUND (1 << 10)
#define FL_WATERJUMP     (1 << 11)
#define FL_FROZEN        (1 << 12)
#define FL_FAKECLIENT    (1 << 13)
#define FL_DUCKING       (1 << 14)
#define FL_FLOAT         (1 << 15)
#define FL_GRAPHED       (1 << 16)
#define FL_IMMUNE_WATER  (1 << 17)
#define FL_IMMUNE_SLIME  (1 << 18)
#define FL_IMMUNE_LAVA   (1 << 19)
#define FL_PROXY         (1 << 20)
#define FL_ALWAYSTHINK   (1 << 21)
#define FL_BASEVELOCITY  (1 << 22)
#define FL_MONSTERCLIP   (1 << 23)
#define FL_ONTRAIN       (1 << 24)
#define FL_WORLDBRUSH    (1 << 25)
#define FL_SPECTATOR     (1 << 26)
#define FL_CUSTOMENTITY  (1 << 29)
#define FL_KILLME        (1 << 30)
#define FL_DORMANT       (1 << 31)

static const float sound_att_norm = 0.8f;

float global_time(void);
void console_log(log_level_t log_level, const char* msg);
void server_command(const char* msg);

// Functions that accept a targetname accept "!player" to refer to the player.
void ent_fire(const char* target, use_type_t use_type, float value);
void ent_kill(const char* target);
void ent_move(const char* target, float x, float y, float z);

void flags_add(const char* target, int32_t flags);
void flags_remove(const char* target, int32_t flags);
int32_t flags_get(const char* target);

void make_follow(const char* target);

// Sound file path is relative to the sound/ dir.
void precache_sound(const char* path);
void precache_model(const char* path);

// Defaults to emitting sound from player if emitterTargetName is NULL.
void play_sound(const char* path, const char* emitterTargetName, sound_channel_t channel, float volume, float attenuation);

void set_model(const char* target, const char* model);

// Sets the model of an entity (dst) using the same model as another entity (src).
void set_model_from(const char* dstTargetName, const char* srcTargetName);
