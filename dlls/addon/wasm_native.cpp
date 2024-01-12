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
		if (FNullEnt(edict))
			break;

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

bool native_play_sound_cb(
		const char* sound, uint32_t channel,
		float volume, float attenuation,
		CBaseEntity* emitter
) {
	EMIT_SOUND(emitter->edict(), channel, sound, volume, attenuation);
	return false;
}

// Plays sound at the first found target origin.
void native_play_sound(
	wasm_exec_env_t exec_env,
	const char* sound, const char* emitterTargetName, uint32_t channel,
	float volume, float attenuation
) {
	auto bound = std::bind(
		native_play_sound_cb,
		sound, channel,
		volume, attenuation,
		std::placeholders::_1
	);

	if (emitterTargetName != nullptr && strlen(emitterTargetName) != 0) {
		ent_iterate("targetname", emitterTargetName, bound);
	} else {
		ent_iterate("classname", "player", bound);
	}
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
