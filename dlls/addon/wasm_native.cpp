#include <functional>

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "talkmonster.h"

#include "wasm/wasm_export.h"
#include "wasm_native.hpp"

void native_console_log(wasm_exec_env_t exec_env, ALERT_TYPE dest, const char * msg) {
	ALERT(dest, msg);
}

void native_server_command(wasm_exec_env_t exec_env, const char* cmd) {
	SERVER_COMMAND(cmd);
}

void native_ent_fire(wasm_exec_env_t exec_env, const char* target, USE_TYPE useType, float value) {
	CBaseEntity* caller = static_cast<CBaseEntity*>(wasm_runtime_get_user_data(exec_env));
	FireTargets(target, caller, caller, useType, value);
}

double native_sin(wasm_exec_env_t exec_env, double input) {
	return sin(input);
}

double native_cos(wasm_exec_env_t exec_env, double input) {
	return cos(input);
}

double native_tan(wasm_exec_env_t exec_env, double input) {
	return tan(input);
}

double native_mod(wasm_exec_env_t exec_env, double a, double b) {
	return fmodf(a, b);
}

float native_global_time(wasm_exec_env_t exec_env) {
	return gpGlobals->time;
}

// Returns false to stop the iteration.
typedef std::function<bool(CBaseEntity*)> ent_iterate_cb_t;
static void ent_iterate(const char* key, const char* value, ent_iterate_cb_t cb) {
	edict_t* edict = nullptr;

	for (;;) {
		edict = FIND_ENTITY_BY_STRING(edict, key, value);
		if (FNullEnt(edict)) {
			break;
		}

		CBaseEntity* target = CBaseEntity::Instance(edict);
		if (!target || FBitSet(target->pev->flags,  FL_KILLME)) {
			continue;
		}

		if (!cb(target)) {
			return;
		}
	}
}

bool native_ent_move_cb(Vector pos, CBaseEntity *target) {
	UTIL_SetOrigin(target->pev, pos);
	return true;
}
void native_ent_move(wasm_exec_env_t exec_env, const char* targetName, float x, float y, float z) {
	Vector pos = Vector(x, y, z);
	auto bound = std::bind(native_ent_move_cb, pos, std::placeholders::_1);
	ent_iterate("targetname", targetName, bound);
}

bool native_ent_kill_cb(CBaseEntity* target) {
	UTIL_Remove(target);
	return true;
}
void native_ent_kill(wasm_exec_env_t exec_env, const char* targetName) {
	ent_iterate("targetname", targetName, native_ent_kill_cb);
}

bool native_make_follow_cb(CBaseEntity* leader, CBaseEntity* target) {
	if (!FClassnameIs(target->pev, "monster_barney")
		&& !FClassnameIs(target->pev, "monster_scientist")) {
		ALERT(at_aiconsole, "Targeted a non CTalkMonster %s in make_follow.\n", STRING(target->pev->classname));
		return true;
	}

	auto follower = static_cast<CTalkMonster*>(target);
	follower->StartFollowing(leader);
	return true;
}

void native_make_follow(wasm_exec_env_t exec_env, const char* targetName) {
	auto leader = UTIL_GetLocalPlayer();
	auto bound = std::bind(native_make_follow_cb, leader, std::placeholders::_1);
	ent_iterate("targetname", targetName, bound);
}

void native_precache_sound(wasm_exec_env_t exec_env, const char* sound) {
	PRECACHE_SOUND(sound);
}
void native_precache_model(wasm_exec_env_t exec_env, const char* model) {
	PRECACHE_MODEL(model);
}

// Plays sound at the first found target origin.
void native_play_sound(
	wasm_exec_env_t exec_env,
	const char* sound, const char* emitterTargetName, uint32_t channel,
	float volume, float attenuation
) {
	CBaseEntity* emitter = nullptr;
	if (emitterTargetName == nullptr || strlen(emitterTargetName) == 0) {
		emitter = UTIL_FindEntityByTargetname(nullptr, "player");
	} else {
		emitter = UTIL_FindEntityByTargetname(nullptr, emitterTargetName);
	}

	if (emitter == nullptr) {
		ALERT(at_error, "Emitter %s not found, cannot play sound %s\n", emitterTargetName, sound);
		return;
	}

	EMIT_SOUND(emitter->edict(), channel, sound, volume, attenuation);
}

bool native_set_model_cb(
	CBaseEntity* ent,
	const char* model
) {
	ALERT(at_aiconsole, "Replacing (targetname#%s)->pev->model with \"%s\"\n", STRING(ent->pev->targetname), model);
	if (STRING(ent->pev->model)[0] == '*') {
		ent->pev->origin = VecBModelOrigin(ent->pev);
		UTIL_SetOrigin(ent->pev, ent->pev->origin);
	}

	SET_MODEL(ENT(ent->pev), model);

	return true;
}

void native_set_model(
	wasm_exec_env_t exec_env,
	const char* targetName,
	const char* model
) {
	auto bound = std::bind(native_set_model_cb, std::placeholders::_1, model);
	ent_iterate("targetname", targetName, bound);
}

bool native_set_model_from_cb(
	CBaseEntity* dst,
	CBaseEntity* src
) {
	const bool isSrcBrush = STRING(src->pev->model)[0] == '*';
	const bool isDstBrush = STRING(dst->pev->model)[0] == '*';

	ALERT(at_aiconsole, "Replacing (targetname#%s)->pev->model with \"%s\"\n", STRING(dst->pev->targetname), STRING(src->pev->model));

	if (isSrcBrush || isDstBrush) {
		auto srcOrigin = src->pev->origin;
		if (isSrcBrush) {
			srcOrigin = VecBModelOrigin(src->pev);
		}

		auto dstOrigin = dst->pev->origin;
		if (isDstBrush) {
			dstOrigin = VecBModelOrigin(dst->pev);
		}

		if (isDstBrush && !isSrcBrush) {
			dst->pev->origin = dstOrigin;
		} else {
			dst->pev->origin = dstOrigin - srcOrigin;
		}

		UTIL_SetOrigin(dst->pev, dst->pev->origin);
	}

	SET_MODEL(ENT(dst->pev), STRING(src->pev->model));

	return true;
}

void native_set_model_from(
	wasm_exec_env_t exec_env,
	const char* dstName,
	const char* srcName
) {
	CBaseEntity* src = UTIL_FindEntityByTargetname(nullptr, srcName);
	if (src == nullptr) {
		return;
	}

	auto bound = std::bind(
		native_set_model_from_cb,
		std::placeholders::_1,
		src
	);
	ent_iterate("targetname", dstName, bound);
}

uint32_t wasm_entity_create(wasm_module_inst_t inst, CBaseEntity* src) {
	// A NULL entity stays NULL in sandboxed code.
	if (src == nullptr) {
		return 0;
	}

	wasm_entity_t * dst = nullptr;
	uint32_t entPtr = wasm_runtime_module_malloc(inst, sizeof(wasm_entity_t), reinterpret_cast<void**>(& dst));
	if (dst == nullptr) {
		ALERT(at_error, "Unable to allocate wasm_entity_t.\n");
		return 0;
	}

	dst->classname = wasm_runtime_module_dup_data(
		inst,
		STRING(src->pev->classname),
		strlen(STRING(src->pev->classname))+1
	);
	dst->targetname = wasm_runtime_module_dup_data(
		inst,
		STRING(src->pev->targetname),
		strlen(STRING(src->pev->targetname))+1
	);

	dst->origin[0] = src->pev->origin[0];
	dst->origin[1] = src->pev->origin[1];
	dst->origin[2] = src->pev->origin[2];

	return entPtr;
}

void wasm_entity_free(wasm_module_inst_t inst, uint32_t entPtr) {
	wasm_entity_t* ent = reinterpret_cast<wasm_entity_t*>(wasm_runtime_addr_app_to_native(inst, entPtr));
	if (ent == nullptr) {
		ALERT(at_error, "Unable to free wasm_entity_t.\n");
		return;
	}

	wasm_runtime_module_free(inst, ent->classname);
	wasm_runtime_module_free(inst, ent->targetname);
	wasm_runtime_module_free(inst, entPtr);
}

bool native_flags_add_cb(CBaseEntity* target, int32_t flags) {
	target->pev->flags |= flags;
	return true;
}
void native_flags_add(wasm_exec_env_t exec_env, const char* targetName, int32_t flags) {
	ent_iterate("targetname", targetName, std::bind(
		native_flags_add_cb,
		std::placeholders::_1,
		flags
	));
}

bool native_flags_remove_cb(CBaseEntity* target, int32_t flags) {
	target->pev->flags &= ~flags;
	return true;
}
void native_flags_remove(wasm_exec_env_t exec_env, const char* targetName, int32_t flags) {
	ent_iterate("targetname", targetName, std::bind(
		native_flags_remove_cb,
		std::placeholders::_1,
		flags
	));
}

bool native_flags_get_cb(CBaseEntity* target, int32_t* flags) {
	*flags = target->pev->flags;
	return false;
}

int32_t native_flags_get(wasm_exec_env_t exec_env, const char* targetName) {
	int32_t flags = 0;

	ent_iterate("targetname", targetName, std::bind(
		native_flags_get_cb,
		std::placeholders::_1,
		&flags
	));

	return flags;
}
