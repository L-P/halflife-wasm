#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "native.h"
#include "format.h"
#include "vec.h"
#include "ent.h"
#include "sequence.h"
#include "strings.h"

void sequence_init(sequence_t *seq, sequence_event_t events[], size_t num_events) {
	if (seq->num_events != 0) {
		// Don't re-init a sequence. Allows initializing sequences in
		// on_activate without interfering with loading.
		return;
	}

	seq->num_events = num_events;
	seq->events = events;

	sequence_reset(seq);
}

void sequence_reset(sequence_t *seq) {
	seq->cur_event = 0;
	seq->last_update = global_time();
}

bool sequence_ended(sequence_t *seq) {
	return seq->cur_event >= seq->num_events;
}

static bool sequence_advance(sequence_t *seq) {
	console_logf(log_debug, "sequence_advance: #%d -> #%d\n", seq->cur_event,  seq->cur_event+1);
	seq->cur_event++;
	seq->last_update = global_time();
	return !sequence_ended(seq);
}

bool sequence_think(sequence_t *seq) {
	if (sequence_ended(seq)) {
		console_log(log_debug, "sequence_think: sequence_ended, bailing\n");
		return false;
	}

	sequence_event_t event = seq->events[seq->cur_event];

	// Delay not elapsed yet.
	if (global_time() < (seq->last_update + event.delay)) {
		return true;
	}

	// Loop over all 0 delay events to fire them all at once.
	do {
		switch (event.type) {
			case EVENT_WAIT_TRIGGER:
				[[fallthrough]];
			case EVENT_PAUSE:
				// Only continue automatically if we explicitely asked for it.
				// 0 is allowed because an EVENT_PAUSE with a 0 delay would be
				// a NOOP and would prevent a zero-value EVENT_PAUSE from
				// working properly.
				if (event.delay <= 0.f) {
					return true;
				}
				break;
			case EVENT_FIRE:
				ent_fire(event.target, event.use_type, 0);
				break;
			case EVENT_KILL:
				ent_kill(event.target);
				break;
			case EVENT_CALLBACK:
				// Callbacks can return false to _not_ advance.
				if (!event.callback(seq, event.payload)) {
					return true;
				}

				break;
			case EVENT_JUMP:
				sequence_jump(seq, event.jump_to);
				return true;
			default:
				console_log(log_error, "unhandled event type\n");
				break;
		}

		if (!sequence_advance(seq)) {
			return false;
		}
		event = seq->events[seq->cur_event];
	} while (event.delay == 0.f);

	return !sequence_ended(seq);
}

void sequence_jump(sequence_t *seq, size_t jump_to) {
	console_logf(log_debug, "sequence_think: jumping to #%d\n", jump_to);
	seq->cur_event = jump_to;
	seq->last_update = global_time();
}

bool sequence_resume(sequence_t *seq) {
	if (sequence_ended(seq)) {
		console_log(log_error, "seq->cur_event OOB\n");
		return false;
	}

	const sequence_event_t event = seq->events[seq->cur_event];
	if (event.type != EVENT_PAUSE) {
		console_log(log_error, "sequence is not paused\n");
		return false;
	}

	sequence_advance(seq);

	return true;
}

bool sequence_fire(sequence_t *seq, const entity_t* activator, const entity_t* caller) {
	if (sequence_ended(seq)) {
		console_log(log_error, "seq->cur_event OOB\n");
		return false;
	}

	const sequence_event_t event = seq->events[seq->cur_event];
	if (event.type != EVENT_WAIT_TRIGGER) {
		console_log(log_error, "sequence is not waiting for a trigger\n");
		return false;
	}

	if (
		(caller    == NULL || ent_matches(caller,    event.caller_class,    event.caller_name)) &&
		(activator == NULL || ent_matches(activator, event.activator_class, event.activator_name))
	) {
		sequence_advance(seq);
		return true;
	}

	return false;
}
