#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "lib/base.h"
#include "lib/vec.h"
#include "lib/native.h"
#include "lib/ent.h"

static const char* pickup_sound = "buttons/blip2.wav";

EXPORT void on_activate() {
    precache_sound(pickup_sound);
}

EXPORT int32_t on_fire(
    const entity_t* activator,
    const entity_t* caller,
    use_type_t use_type,
    float value
) {
    if (ent_matches(caller, "monster_barney", "barney")) {
        vec3_t delta = {0, 0, 12};
        ent_movev("card", vec3_add(caller->origin, delta));
        return true;
    }

    if (ent_matches(caller, "item_security", "card")) {
        console_log(log_debug, "Card pickup.\n");
        play_sound(pickup_sound, NULL, chan_item, 1, sound_att_norm);
        return true;
    }

    return false;
}
