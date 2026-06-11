#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint16_t u16;
typedef int16_t i16;

#define MAX_THINGS ((u16)1024)
#define NIL ((u16)0)
#define MAX_ALARMS ((u16)4)

#define GAME_WIDTH 128
#define GAME_HEIGHT 128
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

#define TILE_SIZE 8
#define HALF_TILE_SIZE 4

#define SHEET_COLUMNS ((u16)11)
#define SHEET_ROWS ((u16)2)

#define SHIP_SPD 2

typedef enum {
	NILKIND,
	SHIPKIND,
	ALIENKIND,
	BULLETKIND,
} Kind;

typedef struct {} ShipTrait;
typedef struct {} AlienTrait;

typedef struct {
	u16 id;
	Kind kind;
	
	Vector2 position;
	Vector2 scale;
	float rotation;

	u16 sprite_id;
	i16 alarms[MAX_ALARMS];

	union {
		ShipTrait ship;
		AlienTrait alien;
	};

	u16 parentId;
	u16 firstChildId;
	u16 nextSibId;
	u16 prevSibId;
} Thing;

typedef struct {
	Thing* things;
	u16 nextEmptySlot;
	Texture* spritesheet;
} State;

void init(State* state);
u16 add(State* state, Thing thing);
Thing *get(Thing* things, u16 id);
void rem(State* state, u16 id);
void draw(Texture2D* spritesheet, Thing* thing); //, _Bool center

inline Vector2 addVector2(Vector2 v1, Vector2 v2);
inline int iRandomRange(int min, int max);

int main(void) {
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "hi!");
	SetTargetFPS(60);

	State state;
	init(&state);

	Texture2D spritesheet = LoadTexture("assets/sheet.png");
	RenderTexture2D renderTexture = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);

	while (!WindowShouldClose()) {

		/*for (int i = 1; i < MAX_THINGS; ++i) {
			if (state.things[i].kind == NILKIND) continue;

			//update here
		}*/

		BeginTextureMode(renderTexture);
		ClearBackground(BLACK);

		for (int i = 1; i < MAX_THINGS; ++i) {
			if (state.things[i].kind == NILKIND) continue;

			draw(&spritesheet, &state.things[i]);
		}
		EndTextureMode();

		BeginDrawing();
		ClearBackground(BLACK);

		SetTextureFilter(renderTexture.texture, TEXTURE_FILTER_POINT);
		DrawTexturePro(renderTexture.texture, (Rectangle){0, 0, GAME_WIDTH, -GAME_HEIGHT}, (Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}, (Vector2){0, 0}, 0.0, WHITE);

		EndDrawing();
	}

	free(state.things);
	UnloadRenderTexture(renderTexture);
	UnloadTexture(spritesheet);
	CloseWindow();
	return 0;
}

void init(State* state) {
	state->things = malloc(MAX_THINGS * sizeof(Thing));

	state->things[NIL] = (Thing){.id = NIL, .kind = NILKIND};

	for (int i = 1; i < MAX_THINGS - 1; ++i) {
		state->things[i] = (Thing){.id = i, .kind = NILKIND, .nextSibId = i + 1};
		memset(&state->things[i].alarms, -1, sizeof(state->things[i].alarms));
	}

	state->things[MAX_THINGS - 1] = (Thing){.id = MAX_THINGS - 1, .kind = NILKIND, .nextSibId = NIL};

	state->nextEmptySlot = 1;
}

u16 add(State* state, Thing thing) {
	if (state->nextEmptySlot == NIL) {
		printf("out of memory\n");
		return NIL;
	}

	u16 slot = state->nextEmptySlot;
	state->nextEmptySlot = state->things[slot].nextSibId;

	state->things[slot] = thing;
	state->things[slot].id = slot;

	state->things[slot].parentId = NIL;
	state->things[slot].firstChildId = NIL;
	state->things[slot].nextSibId = NIL;
	state->things[slot].prevSibId = NIL;

	return slot;
}

Thing* get(Thing* things, u16 id) {
	return (id > 0 && id < MAX_THINGS) ? &things[id] : &things[NIL];
}

void rem(State* state, u16 id) {
	if (id <= NIL || id >= MAX_THINGS || state->things[id].kind == NILKIND) {
		printf("nothing to do\n");
		return;
	}

	memset(&state->things[id].alarms, -1, sizeof(state->things[id].alarms));
	state->things[id].kind = NILKIND;
	state->things[id].nextSibId = state->nextEmptySlot;
	state->nextEmptySlot = id;
}

void draw(Texture2D* spritesheet, Thing* thing) { //, _Bool center
	u16 col = thing->sprite_id % SHEET_COLUMNS;
	u16 row = thing->sprite_id / SHEET_COLUMNS;

	DrawTextureRec(*spritesheet, (Rectangle){
		.x = col * TILE_SIZE,
		.y = row * TILE_SIZE,
		.width = TILE_SIZE,
		.height = TILE_SIZE,
	}, (Vector2){thing->position.x - HALF_TILE_SIZE, thing->position.y - HALF_TILE_SIZE}, WHITE);
}

inline Vector2 addVector2(Vector2 v1, Vector2 v2) { return (Vector2){v1.x + v2.x, v1.y + v2.y}; }
inline int iRandomRange(int min, int max) { return (rand() % (max - min + 1)) + min; }
