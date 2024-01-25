#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "filesystem_utils.h"

#include "wasm/wasm_export.h"
#include "wasm_native.hpp"

#include "game_wasm.hpp"

#define NATIVE(name, args) {#name, reinterpret_cast<void*>(native_##name), args}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static NativeSymbol native_symbols[] = {
	NATIVE(console_log, "(i$)"),
	NATIVE(cos, "(F)F"),
	NATIVE(ent_fire, "($if)"),
	NATIVE(ent_move, "($fff)"),
	NATIVE(global_time, "()f"),
	NATIVE(make_follow, "($)"),
	NATIVE(mod, "(FF)F"),
	NATIVE(play_sound, "($$iff)"),
	NATIVE(precache_sound, "($)"),
	NATIVE(precache_model, "($)"),
	NATIVE(server_command, "($)"),
	NATIVE(sin, "(F)F"),
	NATIVE(tan, "(F)F"),
	NATIVE(set_model, "($$)"),
	NATIVE(set_model_from, "($$)"),
};
#pragma GCC diagnostic pop
#undef NATIVE

LINK_ENTITY_TO_CLASS(game_wasm, CGameWASM);
TYPEDESCRIPTION CGameWASM::m_SaveData[] = {
	DEFINE_ARRAY(CGameWASM, m_persisted, FIELD_CHARACTER, WASM_PERSISTED_SIZE),
	DEFINE_FIELD(CGameWASM, m_path, FIELD_STRING),
};

bool CGameWASM::Save(CSave& save)
{
	if (!CBaseEntity::Save(save)) {
		return false;
	}

	wasm_val_t argv[2] = {};
	uint8_t* nativeBuf = nullptr;

	auto func = wasm_runtime_lookup_function(m_moduleInst, "on_save", "(*~)");
	if (!func) {
		goto cleanup;
	}

	argv[0].kind = WASM_I32;
	argv[0].of.i32 = wasm_runtime_module_malloc(
		m_moduleInst,
		WASM_PERSISTED_SIZE,
		reinterpret_cast<void**>(&nativeBuf)
	);
	if (nativeBuf == nullptr) {
		ALERT(at_error, "Unable to allocate save buffer.\n");
		goto cleanup;
	}

	argv[1].kind = WASM_I32;
	argv[1].of.i32 = WASM_PERSISTED_SIZE;

	ALERT(at_aiconsole, "calling %s::on_save\n", STRING(m_path));
	if (!wasm_runtime_call_wasm_a(m_execEnv, func, 0, nullptr, 2, argv)) {
		ALERT(at_error, "unable to call %s::on_save: %s\n", STRING(m_path), wasm_runtime_get_exception(m_moduleInst));
		goto cleanup;
	}
	memcpy(m_persisted, nativeBuf, WASM_PERSISTED_SIZE);

cleanup:
	if (argv[0].of.i32 != 0) {
		wasm_runtime_module_free(m_moduleInst, argv[0].of.i32);
	}

	return save.WriteFields("CGameWASM", this, m_SaveData, ARRAYSIZE(m_SaveData));
}

bool CGameWASM::Restore(CRestore& restore)
{
	if (!CBaseEntity::Restore(restore)) {
		return false;
	}

	m_shouldRestore = true;

	return !restore.ReadFields("CGameWASM", this, m_SaveData, ARRAYSIZE(m_SaveData));
}

bool CGameWASM::KeyValue(KeyValueData* pkvd) {
	if (FStrEq(pkvd->szKeyName, "path")) {
		m_path = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CGameWASM::Spawn(void) {
	Precache();
	m_shouldRestore = false;
	memset(&m_persisted, 0, WASM_PERSISTED_SIZE);
}

void CGameWASM::Precache(void) {
	PRECACHE_GENERIC(STRING(m_path));
}

static_assert(std::is_same_v<uint8_t, unsigned char>);
void CGameWASM::Activate(void) {
	RuntimeInitArgs initArgs;
	memset(&m_globalHeap, 0, WASM_GLOBAL_HEAP_SIZE);
	memset(&initArgs, 0, sizeof(RuntimeInitArgs));
	initArgs.mem_alloc_type = Alloc_With_Pool;
	initArgs.mem_alloc_option.pool.heap_buf = m_globalHeap;
	initArgs.mem_alloc_option.pool.heap_size = sizeof(m_globalHeap);

	initArgs.native_module_name = "env";
	initArgs.n_native_symbols = sizeof(native_symbols) / sizeof(NativeSymbol);
	initArgs.native_symbols = native_symbols;

	if (!wasm_runtime_full_init(&initArgs)) {
		ALERT(at_error, "Unable to init WASM.\n");
		return;
	}

	char error_buf[128];
	memset(&error_buf, 0, sizeof(error_buf));
	m_wasmExe = FileSystem_LoadFileIntoBuffer(
		STRING(m_path),
		FileContentFormat::Binary,
		"GAMECONFIG"
	);
	if (m_wasmExe.empty()) {
		ALERT(at_error, "No data in in %s\n", STRING(m_path));
		goto fail;
	}

	m_module = wasm_runtime_load(
		reinterpret_cast<uint8_t*>(m_wasmExe.data()),
		m_wasmExe.size(),
		error_buf, sizeof(error_buf)
	);
	if (!m_module) {
		ALERT(at_error, "Unable to load WASM runtime: %s\n", error_buf);
		goto fail;
	}

	m_moduleInst = wasm_runtime_instantiate(m_module, WASM_STACK_SIZE, WASM_HEAP_SIZE, error_buf, sizeof(error_buf));
	if (!m_moduleInst) {
		ALERT(at_error, "Unable to instanciate WASM module: %s\n", error_buf);
		goto fail;
	}

	m_execEnv = wasm_runtime_create_exec_env(m_moduleInst, WASM_STACK_SIZE);
	if (!m_execEnv) {
		ALERT(at_error, "Unable to create WASM exec env.\n");
		goto fail;
	}

	wasm_runtime_set_user_data(m_execEnv, this);
	ALERT(at_aiconsole, "loaded WASM at %s\n", STRING(m_path));

	// HACK: Don't restore if we're not fresh out of a loaded game avoid
	// blanking the statically defined WASM state.
	if (m_shouldRestore) {
		RestorePersisted();
	}
	SetupEntrypoints();

	return;

fail:
	DestroyWASMRuntime();
}

void CGameWASM::RestorePersisted(void) {
	auto func = wasm_runtime_lookup_function(m_moduleInst, "on_restore", "(*~)");
	if (!func) {
		return;
	}

	wasm_val_t argv[2] = {};
	argv[0].kind = WASM_I32;
	argv[0].of.i32 = wasm_runtime_module_dup_data(
		m_moduleInst,
		reinterpret_cast<const char*>(m_persisted),
		WASM_PERSISTED_SIZE
	);
	argv[1].kind = WASM_I32;
	argv[1].of.i32 = WASM_PERSISTED_SIZE;

	ALERT(at_aiconsole, "calling %s::on_restore\n", STRING(m_path));
	if (!wasm_runtime_call_wasm_a(m_execEnv, func, 0, nullptr, 2, argv)) {
		ALERT(at_error, "unable to call %s::on_restore: %s\n", STRING(m_path), wasm_runtime_get_exception(m_moduleInst));
	}

	wasm_runtime_module_free(m_moduleInst, argv[0].of.i32);
}

void CGameWASM::SetupEntrypoints(void) {
	if (wasm_runtime_lookup_function(m_moduleInst, "on_think", "(f)f")) {
		ALERT(at_aiconsole, "found %s::on_think, setting it up.\n", STRING(m_path));
		pev->nextthink = gpGlobals->time + .1f;
		SetThink(&CGameWASM::ThinkWASM);
	}

	if (auto func = wasm_runtime_lookup_function(m_moduleInst, "on_activate", "()")) {
		ALERT(at_aiconsole, "found %s::on_activate, calling it.\n", STRING(m_path));
		if (!wasm_runtime_call_wasm(m_execEnv, func, 0, nullptr)) {
			ALERT(at_error, "unable to call %s::on_activate: %s\n", STRING(m_path), wasm_runtime_get_exception(m_moduleInst));
		}
	}
}

void CGameWASM::ThinkWASM(void) {
	auto func = wasm_runtime_lookup_function(m_moduleInst, "on_think", "(f)i");
	if (func == nullptr) {
		ALERT(at_error, "unable to find %s::on_think\n", STRING(m_path));
		SetThink(nullptr);
		return;
	}

	wasm_val_t resv[1] = {};
	resv[0].kind = WASM_F32;
	resv[0].of.f32 = 0;

	wasm_val_t argv[1] = {};
	argv[0].kind = WASM_F32;
	argv[0].of.f32 = pev->nextthink - pev->ltime;

	pev->ltime = gpGlobals->time;

	if (!wasm_runtime_call_wasm_a(m_execEnv, func, 1, resv, 1, argv)) {
		ALERT(at_error, "unable to call %s::on_think: %s\n", STRING(m_path), wasm_runtime_get_exception(m_moduleInst));
		return;
	}

	if (resv[0].of.f32 < 0) {
		ALERT(at_error, "%s::on_think returned < 0: %f\n", STRING(m_path), resv[0].of.f32);
		return;
	}

	pev->nextthink = gpGlobals->time + resv[0].of.f32;
}

bool CGameWASM::IsTriggered(CBaseEntity* activator, CBaseEntity* caller) {
	auto func = wasm_runtime_lookup_function(m_moduleInst, "on_master_check", "(i)i");
	if (!func) {
		ALERT(at_error, "%s used as master but no %s::on_master_check defined.\n", STRING(pev->targetname), STRING(m_path));
		return false;
	}

	wasm_val_t resv[1] = {};
	resv[0].kind = WASM_I32;
	resv[0].of.i32 = 0;

	wasm_val_t argv[2] = {};
	argv[0].kind = WASM_I32;
	argv[0].of.i32 = wasm_entity_create(m_moduleInst, activator);
	argv[1].kind = WASM_I32;
	argv[1].of.i32 = wasm_entity_create(m_moduleInst, caller);

	ALERT(
		at_aiconsole,
		"calling %s::on_master_check(%s#%s, %s#%s)\n",
		STRING(m_path),
		activator == nullptr ? "NULL" : STRING(activator->pev->classname),
		activator == nullptr ? ""     : STRING(activator->pev->targetname),
		caller    == nullptr ? "NULL" : STRING(caller->pev->classname),
		caller    == nullptr ? ""     : STRING(caller->pev->targetname)
	);

	if (!wasm_runtime_call_wasm_a(m_execEnv, func, 1, resv, 2, argv)) {
		ALERT(at_error, "unable to call %s::on_master_check: %s\n", STRING(m_path), wasm_runtime_get_exception(m_moduleInst));
		return false;
	}

	wasm_entity_free(m_moduleInst, argv[0].of.i32);
	wasm_entity_free(m_moduleInst, argv[1].of.i32);

	return static_cast<bool>(resv[0].of.i32);
}

void CGameWASM::DestroyWASMRuntime(void) {
	if (m_execEnv) {
		wasm_runtime_destroy_exec_env(m_execEnv);
		m_execEnv = nullptr;
	}

	if (m_moduleInst) {
		wasm_runtime_deinstantiate(m_moduleInst);
		m_moduleInst = nullptr;
	}

	if (m_module) {
		wasm_runtime_unload(m_module);
		m_module = nullptr;
	}

	if (!m_wasmExe.empty()) {
		m_wasmExe.clear();
	}

	wasm_runtime_destroy();
}

void CGameWASM::Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value) {
	if (!m_execEnv) {
		ALERT(at_error, "use called but we have no exec env\n");
		return;
	}

	ALERT(
		at_aiconsole,
		"calling %s::on_fire(%s#%s, %s#%s)\n",
		STRING(m_path),
		activator == nullptr ? "NULL" : STRING(activator->pev->classname),
		activator == nullptr ? ""     : STRING(activator->pev->targetname),
		caller    == nullptr ? "NULL" : STRING(caller->pev->classname),
		caller    == nullptr ? ""     : STRING(caller->pev->targetname)
	);

	auto func = wasm_runtime_lookup_function(m_moduleInst, "on_fire", "(if)i");
	if (func == nullptr) {
		ALERT(at_error, "unable to find %s::on_fire\n", STRING(m_path));
		return;
	}

	wasm_val_t resv[1] = {};
	resv[0].kind = WASM_I32;
	resv[0].of.i32 = 0;

	// No matter how I try to move this init out of the method I end up
	// corrupting the stack when the helper function returns. Doesn't happen in
	// debug builds of course.
	wasm_val_t argv[4] = {}; // i activator, i caller, i useType, f value
	argv[0].kind = WASM_I32;
	argv[0].of.i32 = wasm_entity_create(m_moduleInst, activator);
	argv[1].kind = WASM_I32;
	argv[1].of.i32 = wasm_entity_create(m_moduleInst, caller);
	argv[2].kind = WASM_I32;
	argv[2].of.i32 = useType;
	argv[3].kind = WASM_F32;
	argv[3].of.f32 = value;

	if (!wasm_runtime_call_wasm_a(m_execEnv, func, 1, resv, 4, argv)) {
		ALERT(
			at_error,
			"unable to call %s::on_fire: %s\n",
			STRING(m_path), wasm_runtime_get_exception(m_moduleInst)
		);
		goto cleanup;
	}

	if (resv[0].of.i32 != 1) {
		ALERT(
			at_error,
			"%s::on_fire returned error code: %d\n",
			STRING(m_path),
			resv[0].of.i32
		);
		goto cleanup;
	}
cleanup:
	wasm_entity_free(m_moduleInst, argv[0].of.i32);
	wasm_entity_free(m_moduleInst, argv[1].of.i32);
}
