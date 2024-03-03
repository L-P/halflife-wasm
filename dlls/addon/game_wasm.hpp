#define WASM_GLOBAL_HEAP_SIZE 512 * 1024
#define WASM_STACK_SIZE 8092
#define WASM_HEAP_SIZE 8092
#define WASM_PERSISTED_SIZE 1024

class CGameWASM: public CBaseEntity
{
	public:
		bool KeyValue(KeyValueData* pkvd) override;
		void Use(CBaseEntity* pActivator, CBaseEntity* pOther, USE_TYPE useType, float value);
		void Spawn(void) override;
		void Precache(void) override;
		void Activate(void) override;
		bool Save(CSave& save) override;
		bool Restore(CRestore& restore) override;
		bool IsTriggered(CBaseEntity* activator, CBaseEntity* caller) override;
		int ObjectCaps() override {
			auto caps =  CBaseEntity::ObjectCaps();
			caps &= ~FCAP_ACROSS_TRANSITION;
			caps |= FCAP_MASTER;

			return caps;
		}

		void EXPORT ThinkWASM(void);
		string_t m_path;
		uint8_t m_persisted[WASM_PERSISTED_SIZE];
		static TYPEDESCRIPTION m_SaveData[];

	private:
		void DestroyWASMRuntime(void);
		void SetupEntrypoints(void);
		void RestorePersisted(void);

		/* (...) the runtime can make modifications to the buffer for its
		 * internal purposes. Thus, in general, it isn't safe to create
		 * multiple modules from a single buffer.
		 * -- core/iwasm/include/wasm_export.h
		 *
		 * This needs to stay alive and untouched as long as the module is loaded.
		 * */
		std::vector<std::byte> m_wasmExe;

		uint8_t m_globalHeap[WASM_GLOBAL_HEAP_SIZE];
		wasm_module_t m_module;
		wasm_module_inst_t m_moduleInst;
		wasm_exec_env_t m_execEnv;
		bool m_shouldRestore;
};
