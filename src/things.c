#include "things.h"
#include <stdio.h>
#include <stdlib.h>

void init(State *state) {
	state->things = malloc(MAX_THINGS * sizeof(Thing));

	state->things[NIL] = (Thing){.id = NIL, .kind = NILKIND};

	for (int i = 1; i < MAX_THINGS - 1; ++i) {
		state->things[i] = (Thing){.id = i, .kind = NILKIND, .next_sib_id = i + 1};
	}

	state->things[MAX_THINGS - 1] = (Thing){.id = MAX_THINGS - 1, .kind = NILKIND, .next_sib_id = NIL};

	state->next_empty_slot = 1;
}

int add(State *state, Kind kind, Vector2 pos) {
	if (state->next_empty_slot == NIL) {
		printf("out of memory\n");
		return NIL;
	}

	int slot = state->next_empty_slot;
	state->next_empty_slot = state->things[slot].next_sib_id;

	state->things[slot] = (Thing){
		.kind = kind,
		.position = pos,
		.parent_id = NIL,
		.first_child_id = NIL,
		.next_sib_id = NIL,
		.prev_sib_id = NIL,
	};

	return slot;
}

Thing *get(Thing *things, int id) {
	if (id > 0 && id < MAX_THINGS) {
		return &things[id];
	} else {
		return &things[NIL];
	}
}

void rem(State *state, int id) {
	if (id <= NIL || id >= MAX_THINGS || state->things[id].kind == NILKIND) {
		printf("nothing to do\n");
		return;
	}

	state->things[id].kind = NILKIND;
	state->things[id].next_sib_id = state->next_empty_slot;
	state->next_empty_slot = id;
}