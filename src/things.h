#ifndef THINGS_H
#define THINGS_H

#include "raylib.h"

#define MAX_THINGS 4096
#define NIL 0

typedef enum {
	NILKIND,
	SHIPKIND,
	ALIENKIND,
	PARTICLEKIND
} Kind;

typedef struct {
	int id;
	Kind kind;
	Vector2 position;

	int parent_id;
	int first_child_id;
	int next_sib_id;
	int prev_sib_id;
} Thing;

typedef struct {
	Thing *things;
	int next_empty_slot;
} State;

void init(State *state);
int add(State *state, Kind kind, Vector2 pos);
void rem(State *state, int id);
Thing *get(Thing *things, int id);

#endif