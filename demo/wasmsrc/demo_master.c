#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "lib/base.h"
#include "lib/vec.h"
#include "lib/native.h"
#include "lib/ent.h"
#include "lib/format.h"

static const char* buzzoff_sound = "buttons/button11.wav";

static bool open = false;
static float lastBuzz = 0;

EXPORT void on_activate(void) {
    precache_sound(buzzoff_sound);
}

EXPORT int32_t on_master_check(const entity_t* activator, const entity_t* caller) {
    if (caller == NULL) {
        console_log(log_error, "on_master_check called will NULL caller.\n");
        return false;
    }

    bool state = false;
    if (ent_matches(caller, "func_door", "door_a")) {
        state = open;
    } else if (ent_matches(caller, "func_door", "door_b")) {
        state = !open;
    }

    console_logf(
        log_info, 
        "Master checked for %s:%s: %d\n",
        caller->class, caller->name,
        state
    );

    if (!state) {
        float curTime = global_time();
        if ((curTime - lastBuzz) > 1.f) {
            play_sound(buzzoff_sound, caller->name, chan_static, 1, sound_att_norm);
            lastBuzz = curTime;
        }
    }

    return state;
}

EXPORT int32_t on_fire(
    const entity_t* activator,
    const entity_t* caller,
    use_type_t use_type,
    float value
) {
    if (ent_matches(caller, "trigger_multiple", "open")) {
        open = true;
        return true;
    }

    if (ent_matches(caller, "trigger_multiple", "close")) {
        open = false;
        return true;
    }

    return false;
}
