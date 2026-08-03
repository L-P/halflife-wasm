typedef enum {
	// Pauses the sequence until it has been resume()d.
	// If .delay is >= 0 then the sequence will resume automatically after
	// .delay seconds.
	EVENT_PAUSE,

	// Fires the given target after .delay seconds.
	EVENT_FIRE,

	// Kills the given target after .delay seconds.
	EVENT_KILL,

	// Calls .callback after .delay seconds.
	EVENT_CALLBACK,

	// Resets sequence to index .jump_to. Jump to 0 to loop the entire sequence.
	EVENT_JUMP,
} sequence_event_type_t;

// Forward decl to fix cyclic dependency callback->sequence->event->callback.
typedef struct sequence_s sequence_t;
typedef bool (*event_callback_t)(sequence_t*, void*); // (seq, payload)

typedef struct {
	sequence_event_type_t type;
	float delay;

	union {
		struct { // EVENT_{FIRE,KILL}
			char *target;
			use_type_t use_type;
		};
		struct { // EVENT_CALLBACK
			event_callback_t callback;
			void* payload;
		};
		size_t jump_to; // EVENT_JUMP
	};
} sequence_event_t;

// A sequence_t is a _linear_ sequence of targets to fire in time.
typedef struct sequence_s {
	size_t cur_event; // index of the current event in .events
	float last_update; // time when the .cur event last changed

	size_t num_events;
	sequence_event_t *events;
} sequence_t;

void sequence_init(sequence_t *seq, sequence_event_t events[], size_t num_events);
void sequence_reset(sequence_t *seq);
void sequence_jump(sequence_t *seq, size_t jump_to);

// Returns false if the sequence has ended and must be stopped.
bool sequence_think(sequence_t *seq);
bool sequence_resume(sequence_t *seq);
