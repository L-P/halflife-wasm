#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "native_engine.h"
#include "format.h"
#include "sequence.h"

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

static bool sequence_ended(sequence_t *seq) {
	return seq->cur_event >= seq->num_events;
}

static bool sequence_advance(sequence_t *seq) {
	console_logf(log_debug, "sequence_advance: #%d -> #%d\n", seq->cur_event,  seq->cur_event+1);
	seq->cur_event++;
	seq->last_update = global_time();
	return !sequence_ended(seq);
}

bool sequence_think(sequence_t *seq, float cur_time) {
	if (sequence_ended(seq)) {
		console_log(log_debug, "sequence_think: sequence_ended, bailing\n");
		return false;
	}

	sequence_event_t event = seq->events[seq->cur_event];

	// Delay not elapsed yet.
	if (cur_time < (seq->last_update + event.delay)) {
		return true;
	}

	// Loop over all 0 delay events to fire them all at once.
	do {
		switch (event.type) {
			case EVENT_PAUSE:
				// Only continue automatically if we explicitely asked for it.
				if (event.delay < 0.f) {
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
				event.callback(seq);
				break;
			case EVENT_JUMP:
				console_logf(log_debug, "sequence_think: jumping to #%d\n", event.jump_to);
				seq->cur_event = event.jump_to;
				seq->last_update = global_time();
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

bool sequence_resume(sequence_t *seq) {
	if (seq->cur_event >= seq->num_events) {
		console_log(log_error, "seq->cur_event OOB\n");
		return false;
	}

	const sequence_event_t event = seq->events[seq->cur_event];
	if (event.type != EVENT_PAUSE) {
		console_log(log_error, "sequence is not paused\n");
		return true;
	}

	return sequence_advance(seq);
}
