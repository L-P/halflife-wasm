#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "lib/base.h"
#include "lib/vec.h"
#include "lib/native.h"
#include "lib/ent.h"

#include "lib/stb_sprintf.h"

static const char* sentences[] = {
    "Hello there, I'm the one talking.",
    "Yes, me, the computer.",
    "Or maybe the computer inside the computer.",
    "This is a small demonstration of server_command.",
    "This native call sends commands to be executed as the server.",
    "Here you're the server so I can just 'say' all of this.",
    "Imagining what's achievable with call is left as an exercise to the reader.",
    "You may press the button one last time to open the console.",
};

static const size_t sentences_size = sizeof(sentences) / sizeof(char*);

static size_t sentence_index = 0;

EXPORT int32_t on_fire(
    const entity_t* activator,
    const entity_t* caller,
    use_type_t use_type,
    float value
) {
    if (sentence_index >= sentences_size) {
        server_command("toggleconsole\n");
        return false;
    }

    char buf [512];
    stbsp_snprintf(buf, sizeof(buf), "say \"%s\"\n", sentences[sentence_index]);
    server_command(buf);

    sentence_index++;

    return true;
}

EXPORT void on_save(char* buf, size_t bufSize) {
    memcpy(buf, &sentence_index, sizeof(sentence_index));
}

EXPORT void on_restore(const char* buf, size_t bufSize) {
    memcpy(&sentence_index, buf, sizeof(sentence_index));
}
