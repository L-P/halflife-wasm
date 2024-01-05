#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "lib/base.h"
#include "lib/vec.h"
#include "lib/strings.h"
#include "lib/native.h"
#include "lib/ent.h"
#include "lib/format.h"

#define SCREEN_WIDTH 10
#define SCREEN_HEIGHT 10

static const size_t SCREEN_PIXELS = (SCREEN_WIDTH*SCREEN_HEIGHT);
static const float fps = 60;
static const float damp = .75f;
static const vec2_t gravity = {0, .01f};

static float time = .0f;
static bool enable = false;
static vec3_t ball_offset = {
    .x = 80,
    .y = 256 - 32,
    .z = 128 + 32,
};

static size_t lastPixelIndex = 0;
static vec2_t pos;
static vec2_t vel;

static const char * screen[] = {
    "screen_0_0", "screen_1_0", "screen_2_0", "screen_3_0", "screen_4_0", "screen_5_0", "screen_6_0", "screen_7_0", "screen_8_0", "screen_9_0",
    "screen_0_1", "screen_1_1", "screen_2_1", "screen_3_1", "screen_4_1", "screen_5_1", "screen_6_1", "screen_7_1", "screen_8_1", "screen_9_1",
    "screen_0_2", "screen_1_2", "screen_2_2", "screen_3_2", "screen_4_2", "screen_5_2", "screen_6_2", "screen_7_2", "screen_8_2", "screen_9_2",
    "screen_0_3", "screen_1_3", "screen_2_3", "screen_3_3", "screen_4_3", "screen_5_3", "screen_6_3", "screen_7_3", "screen_8_3", "screen_9_3",
    "screen_0_4", "screen_1_4", "screen_2_4", "screen_3_4", "screen_4_4", "screen_5_4", "screen_6_4", "screen_7_4", "screen_8_4", "screen_9_4",
    "screen_0_5", "screen_1_5", "screen_2_5", "screen_3_5", "screen_4_5", "screen_5_5", "screen_6_5", "screen_7_5", "screen_8_5", "screen_9_5",
    "screen_0_6", "screen_1_6", "screen_2_6", "screen_3_6", "screen_4_6", "screen_5_6", "screen_6_6", "screen_7_6", "screen_8_6", "screen_9_6",
    "screen_0_7", "screen_1_7", "screen_2_7", "screen_3_7", "screen_4_7", "screen_5_7", "screen_6_7", "screen_7_7", "screen_8_7", "screen_9_7",
    "screen_0_8", "screen_1_8", "screen_2_8", "screen_3_8", "screen_4_8", "screen_5_8", "screen_6_8", "screen_7_8", "screen_8_8", "screen_9_8",
    "screen_0_9", "screen_1_9", "screen_2_9", "screen_3_9", "screen_4_9", "screen_5_9", "screen_6_9", "screen_7_9", "screen_8_9", "screen_9_9",
};

static void pixeli(size_t i, bool state) {
    if (i >= SCREEN_PIXELS) {
        return;
    }

    if (state) {
        ent_fire(screen[i], use_on, 0);
    } else {
        ent_fire(screen[i], use_off, 0);
    }
}

static size_t pixelxy(int x, int y, bool state) {
    const size_t i = (size_t) ((y * SCREEN_WIDTH) + x);
    pixeli(i, state);
    return i;
}

static void draw(void)  {
    const size_t newPixelIndex = pixelxy((int) pos.x, (int) pos.y, true);
    if (newPixelIndex != lastPixelIndex) {
        pixeli(lastPixelIndex, false);
        lastPixelIndex = newPixelIndex;
    }
}

static void update(void) {
    vel = vec2_add(vel, gravity);
    pos = vec2_add(pos, vel);

    if (pos.x <= 0) {
        pos.x = 0;
        vel.x = -vel.x * damp;
    }
    if (pos.x > (SCREEN_WIDTH - 1)) {
        pos.x = SCREEN_WIDTH - 1;
        vel.x = -vel.x * damp;
    }

    if (pos.y <= 0) {
        pos.y = 0;
        vel.y = -vel.y * damp;
    }
    if (pos.y > (SCREEN_HEIGHT - 1)) {
        pos.y = SCREEN_HEIGHT - 1;
        vel.y = -vel.y * damp;

        if (vel.y < .01f && vel.y > -0.01f) {
            vel.y = 0;
            vel.x *= 0.98;
        }
    }

    vec3_t delta = {0, .y = -pos.x * 51.2, .z = -pos.y * 25.6};
    ent_movev("ball", vec3_add(ball_offset, delta));
}

static void reset(void) {
    vel.x = .3f + mod(time, 8.f);
    vel.y = 0;
    pos.x = mod(time, 10.f);
    pos.y = 0;
}

EXPORT float on_think(float dt) {
    if (!enable) {
        return .1f;
    }

    time += dt;

    update();
    draw();

    return 1 / fps;
}

EXPORT int32_t on_fire(
    const entity_t* activator,
    const entity_t* caller,
    use_type_t use_type,
    float value
) {
    if (ent_matches(caller, NULL, "_wasm_toggle")) {
        enable = !enable;
        console_logf(log_info, "WASM: Toggling sim, current state: %d\n", enable);
        return true;
    }

    if (ent_matches(caller, NULL, "_wasm_reset")) {
        console_log(log_info, "WASM: Resetting sim.\n");
        reset();
        return true;
    }

    return false;
}

EXPORT void on_activate(void) {
    enable = false;
    console_log(log_info, "WASM: Activated, disabling sim.\n");
}

typedef struct {
    size_t lastPixelIndex;
    vec2_t pos;
    vec2_t vel;
} state_t;

EXPORT void on_save(char* buf, size_t bufSize) {
    const state_t state = {
        .lastPixelIndex = lastPixelIndex,
        .pos = pos,
        .vel = vel,
    };

    if (sizeof(state_t) > bufSize) {
        console_log(log_error, "sizeof(state_t) > bufSize\n");
        return;
    }

    memcpy(buf, &state, sizeof(state_t));
}

EXPORT void on_restore(const char* buf, size_t bufSize) {
    if (sizeof(state_t) > bufSize) {
        console_log(log_error, "sizeof(state_t) > bufSize\n");
        return;
    }

    state_t state = {0};
    memcpy(&state, buf, sizeof(state_t));

    lastPixelIndex = state.lastPixelIndex;
    pos = state.pos;
    vel = state.vel;
}
