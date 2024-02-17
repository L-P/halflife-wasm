#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "stb_sprintf.h"
#pragma clang diagnostic pop

#include "base.h"
#include "vec.h"
#include "strings.h"
#include "native.h"
#include "format.h"

#include "ent.h"

bool ent_matches(
    const entity_t* ent,
    const char* class,
    const char* name
) {
    if (ent == NULL) {
        return false;
    }

    if (strlen(class) > 0) {
        if (strcmp(ent->class, class) != 0) {
            return false;
        }
    }

    if (strlen(name) > 0) {
        if (strcmp(ent->name, name) != 0) {
            return false;
        }
    }

    return true;
}

void ent_print(log_level_t level, const entity_t* ent) {
    if (ent == NULL) {
        console_log(level, "NULL\n");
        return;
    }

    console_logf(level, "%s#%s\n", ent->class, ent->name);
    console_logf(
        level,
        "  origin: %f %f %f\n",
        ent->origin.x,
        ent->origin.y,
        ent->origin.z
    );
}

void ent_movev(const char* target, vec3_t pos) {
    ent_move(target, pos.x, pos.y, pos.z);
}
