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
#define SCREEN_PIXELS (SCREEN_WIDTH*SCREEN_HEIGHT)
static const float ups = 4;
static uint8_t previous[SCREEN_PIXELS] = {0};

typedef struct {
    float time;
    bool enable;
    uint8_t board[SCREEN_PIXELS];
} state_t;

static state_t state = {0};

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

static void pixeli(size_t i, bool pixelState) {
    if (i >= SCREEN_PIXELS) {
        return;
    }

    if (pixelState) {
        ent_fire(screen[i], use_on, 0);
    } else {
        ent_fire(screen[i], use_off, 0);
    }
}

static void draw(void)  {
    for (size_t i = 0; i < SCREEN_PIXELS; i++) {
        pixeli(i, state.board[i] == 1);
    }
}

static int check(uint8_t* board, int x, int y) {
    if (x < 0) {
        x += SCREEN_WIDTH;
    }

    if (y < 0) {
        y += SCREEN_HEIGHT;
    }

    x %= SCREEN_WIDTH;
    y %= SCREEN_HEIGHT;

    return board[(y * SCREEN_WIDTH) + x];
}

static int neighbors(uint8_t* board, int x, int y) {
    int ret = 0;

    ret += check(board, x-1, y-1);
    ret += check(board, x+0, y-1);
    ret += check(board, x+1, y-1);
    ret += check(board, x-1, y+0);

    ret += check(board, x+1, y+0);
    ret += check(board, x-1, y+1);
    ret += check(board, x+0, y+1);
    ret += check(board, x+1, y+1);

    return ret;
}

static void update(void) {
    memcpy(&previous, &state.board, sizeof(previous));

    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            const size_t i = (y * SCREEN_WIDTH) + x;
            const int cnt = neighbors(previous, (int) x, (int) y);

            if (previous[i]) {
                state.board[i] = cnt >= 2 && cnt <= 3;
            } else {
                state.board[i] = cnt == 3;
            }
        }
    }
}

static void reset(void) {
    state.enable = false;
    memset(previous, 0, SCREEN_PIXELS);
    memset(state.board, 0, SCREEN_PIXELS);
    for (size_t i = 0; i < SCREEN_PIXELS; i++) {
        pixeli(i, false);
    }
}

EXPORT float on_think(float dt) {
    if (!state.enable) {
        return .1f;
    }

    state.time += dt;

    update();
    draw();

    return 1 / ups;
}

void handle_pixel_toggle(const char* name) {
    char buf[10+1];
    memcpy(buf, name, 11);

    buf[0] = 's';
    buf[1] = 'c';
    buf[2] = 'r';
    buf[3] = 'e';
    buf[4] = 'e';
    buf[5] = 'n';

    for (size_t i = 0; i < SCREEN_PIXELS; i++) {
        if (strcmp(buf, screen[i]) == 0) {
            state.board[i] = 1;
            pixeli(i, true);
            return;
        }
    }
}

EXPORT int32_t on_fire(
    const entity_t* activator,
    const entity_t* caller,
    use_type_t use_type,
    float value
) {
    if (ent_matches(caller, "func_button", "toggle")) {
        state.enable = !state.enable;
        console_logf(log_info, "WASM: Toggling sim, current state: %d\n", state.enable);
        return true;
    }

    if (ent_matches(caller, "func_button", "reset")) {
        console_log(log_info, "WASM: Resetting sim.\n");
        reset();
        return true;
    }

    if (ent_matches(caller, "func_button", NULL)) {
        handle_pixel_toggle(caller->name);
        return true;
    }

    return false;
}

EXPORT void on_activate(void) {
    state.enable = false;
    reset();
    console_log(log_info, "WASM: Activated, disabling sim.\n");
}

EXPORT void on_save(char* buf, size_t bufSize) {
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

    memcpy(&state, buf, sizeof(state_t));
    memcpy(previous, state.board, sizeof(previous));
}
