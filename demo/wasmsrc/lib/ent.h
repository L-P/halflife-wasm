typedef struct {
    const char * class;
    const char * name;
    vec3_t origin;
} entity_t;

bool ent_matches(const entity_t* ent, const char* class, const char* name);
void ent_print(log_level_t level, const entity_t* ent);
void ent_movev(const char* target, vec3_t pos);
