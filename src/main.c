#include "raylib.h"
#include "stdlib.h"
#include "things.h"
#include "utils.h"

#define SHIP_SPD 2

int main(void) {
	InitWindow(128, 128, "anton");
	SetTargetFPS(60);

	State state;
	init(&state);

	int ship_id = add(&state, SHIPKIND, (Vector2){64, 64});

	while (!WindowShouldClose()) {

	Thing *ship = get(state.things, ship_id);
	int move_x = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
	int move_y = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);

	ship->position = (Vector2){
		ship->position.x + (move_x * SHIP_SPD),
		ship->position.y + (move_y * SHIP_SPD)
	};

	if (move_x != 0 || move_y != 0) {
		add(&state, PARTICLEKIND,
			vec2add(ship->position, (Vector2){
			.x = irandom_range(-2, 2),
			.y = irandom_range(-2, 2),
		}));
	}

	BeginDrawing();
	ClearBackground(BLACK);

	for (int i = 1; i < MAX_THINGS; ++i) {
		Thing *thing = &state.things[i];

		if (thing->kind == NILKIND) continue;

		if (thing->kind == SHIPKIND) {
			DrawRectangleV(thing->position, (Vector2){64, 64}, RED);
		}

		if (thing->kind == PARTICLEKIND) {
			DrawRectangleV(thing->position, (Vector2){64, 64}, RED);
		}
	}

	EndDrawing();
	}

	free(state.things);

	CloseWindow();
	return 0;
}
