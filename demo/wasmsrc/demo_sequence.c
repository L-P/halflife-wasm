#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "lib/base.h"
#include "lib/vec.h"
#include "lib/native.h"
#include "lib/ent.h"
#include "lib/format.h"
#include "lib/sequence.h"

typedef struct {
	sequence_t seq;
} state_t;
static state_t state = {};
IMPLEMENT_SAVERESTORE(state_t, state)

void all_sprites_off(void*) {
    ent_fire("spr1", use_off, 0);
    ent_fire("spr2", use_off, 0);
    ent_fire("spr3", use_off, 0);
    ent_fire("spr4", use_off, 0);
    ent_fire("spr5", use_off, 0);
}

static sequence_event_t evs[] = {
    {EVENT_CALLBACK, 0.f, .callback = all_sprites_off},
    {EVENT_FIRE, 0.f, .target = "spr1", .use_type = use_on},
    {EVENT_FIRE, .5f, .target = "spr1", .use_type = use_off},
    {EVENT_FIRE, 0.f, .target = "spr2", .use_type = use_on},
    {EVENT_FIRE, .5f, .target = "spr2", .use_type = use_off},
    {EVENT_FIRE, 0.f, .target = "spr3", .use_type = use_on},
    {EVENT_FIRE, .5f, .target = "spr3", .use_type = use_off},
    {EVENT_FIRE, 0.f, .target = "spr4", .use_type = use_on},
    {EVENT_FIRE, .5f, .target = "spr4", .use_type = use_off},
    {EVENT_FIRE, 0.f, .target = "spr5", .use_type = use_on},
    {EVENT_FIRE, .5f, .target = "spr5", .use_type = use_off},
    {EVENT_JUMP, 0.f, .jump_to = 0},
};

EXPORT void on_activate() {
    sequence_init(&state.seq, evs, sizeof(evs) / sizeof(sequence_event_t));
}

EXPORT int32_t on_fire(
    const entity_t* activator,
    const entity_t* caller,
    use_type_t use_type,
    float value
) {
    sequence_reset(&state.seq);
    return true;
}

EXPORT float on_think(float time) {
    sequence_think(&state.seq, global_time());
    return .01f;
}
