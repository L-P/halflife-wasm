// {{{ Made available to WASM.
float native_global_time(wasm_exec_env_t exec_env);
void native_console_log(wasm_exec_env_t exec_env, ALERT_TYPE dest, const char * msg);
void native_server_command(wasm_exec_env_t exec_env, const char* command);

void native_ent_fire(wasm_exec_env_t exec_env, const char* target, USE_TYPE useType, float value);
void native_ent_kill(wasm_exec_env_t exec_env, const char* target);
void native_ent_move(wasm_exec_env_t exec_env, const char* target, float x, float y, float z);
void native_make_follow(wasm_exec_env_t exec_env, const char* target);

void native_play_sound(wasm_exec_env_t exec_env, const char* sound, const char* emitter, uint32_t channel, float volume, float attenuation);
void native_precache_sound(wasm_exec_env_t exec_env, const char* name);
void native_precache_model(wasm_exec_env_t exec_env, const char* name);

void native_set_model(wasm_exec_env_t exec_env, const char* target, const char* model);
void native_set_model_from(wasm_exec_env_t exec_env, const char* dstTarget, const char* srcTarget);

void native_flags_add(wasm_exec_env_t exec_env, const char* target, int32_t flags);
void native_flags_remove(wasm_exec_env_t exec_env, const char* target, int32_t flags);
int32_t native_flags_get(wasm_exec_env_t exec_env, const char* target);

double native_cos(wasm_exec_env_t exec_env, double);
double native_mod(wasm_exec_env_t exec_env, double, double);
double native_sin(wasm_exec_env_t exec_env, double);
double native_tan(wasm_exec_env_t exec_env, double);

typedef struct {
	uint32_t classname;
	uint32_t targetname;
	float origin[3];
} wasm_entity_t;
// }}}

// {{{ Internal utils.
uint32_t wasm_entity_create(wasm_module_inst_t inst, CBaseEntity* src);
void wasm_entity_free(wasm_module_inst_t inst, uint32_t entPtr);
// }}}
