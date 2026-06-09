#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_THINGS 1024
#define NIL 0

typedef enum {
  NILKIND,
  PLAYERKIND,
  TROLLKIND,
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

void init(State *state) {
  state->things = malloc(MAX_THINGS * sizeof(Thing));

  state->things[NIL] = (Thing){.id = NIL, .kind = NILKIND};

  for (int i = 1; i < MAX_THINGS - 1; ++i) {
    state->things[i] = (Thing){.id = i, .kind = NILKIND, .next_sib_id = i + 1};
  }

  state->things[MAX_THINGS - 1] =
      (Thing){.id = MAX_THINGS - 1, .kind = NILKIND, .next_sib_id = NIL};

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

int main(void) {
  InitWindow(512, 512, "anton");
  SetTargetFPS(60);

  State state;
  init(&state);

  int player_id = add(&state, PLAYERKIND, (Vector2){256, 256});

  while (!WindowShouldClose()) {

    if (IsKeyPressed(KEY_SPACE)) {
      rem(&state, player_id);
    }

    BeginDrawing();
    ClearBackground(BLACK);

    for (int i = 1; i < MAX_THINGS; ++i) {
      Thing *thing = &state.things[i];

      if (thing->kind == NILKIND)
        continue;

      if (thing->kind == PLAYERKIND) {
        DrawRectangleV(thing->position, (Vector2){64, 64}, RED);
      }
    }

    EndDrawing();
  }

  free(state.things);

  CloseWindow();
  return 0;
}
