#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

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

#define ABS(x) ((x) < 0 ? -(x) : (x))
typedef enum {
	NILKIND,
	SHIPKIND,
	ALIENKIND,
	BULLETKIND,
	KIND_AMOUNT,
} Kind;

#define KIND_COUNT (KIND_AMOUNT - 1)

typedef struct {} ShipTrait;
typedef struct {} AlienTrait;

typedef struct {
	Vector2 dimensions;
	Vector2 offset;
} Mask;

typedef struct {
	u16 id;
	u16 kind;
	
	Vector2 position;
	Vector2 scale;
	float rotation;
	Mask mask;

	u16 sprite_id;
	i16 alarms[MAX_ALARMS];

	u16 parentId;
	u16 firstChildId;
	u16 nextSibId;
	u16 prevSibId;
} Thing;

typedef struct {
	Thing* things;
	u16 nextEmptySlot;
	Texture* spritesheet;
	u16 kindHeads[KIND_COUNT];
} State;

void init(State* state);
u16 add(State* state, Thing thing);
Thing *get(Thing* things, u16 id);
void rem(State* state, u16 id);
void draw(Texture2D* spritesheet, Thing* thing); //, _Bool center
void kind_link(State* state, u16 id);
void kind_unlink(State* state, u16 id);

inline Vector2 addVector2(Vector2 v1, Vector2 v2);
inline int iRandomRange(int min, int max);

bool checkOBB(Thing* t1, Thing* t2);

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

void kind_link(State* state, u16 id) {
	if (state->things[id].kind == NILKIND) return;
	Thing* thing = &state->things[id];
	Kind k = thing->kind;

	u16 head = state->kindHeads[k];

	if (head == NIL) {
		state->kindHeads[k] = id;
		thing->nextSibId = id;
		thing->prevSibId = id;
	} else {
		u16 tail = state->things[head].prevSibId;
		state->things[tail].nextSibId = id;
		state->things[id].prevSibId = tail;

		state->things[id].nextSibId = head;
		state->things[tail].prevSibId = id;
	}
}

void kind_unlink(State* state, u16 id) {
	if (state->things[id].kind == NILKIND) return;
	Thing* thing = &state->things[id];
	Kind k = thing->kind;

	u16 next = state->things[id].nextSibId;
    u16 prev = state->things[id].prevSibId;

    if (next == id) {
        state->kindHeads[k] = NIL;
    } else {
        state->things[prev].nextSibId = next;
        state->things[next].prevSibId = prev;

        if (state->kindHeads[k] == id) {
            state->kindHeads[k] = next;
        }
    }
}

inline Vector2 addVector2(Vector2 v1, Vector2 v2) { return (Vector2){v1.x + v2.x, v1.y + v2.y}; }
inline int iRandomRange(int min, int max) { return (rand() % (max - min + 1)) + min; }

// i really need operator overloading
bool checkOBB(Thing* t1, Thing* t2) {
	float ax1 = cosf(t1->rotation);
	float ay1 = sinf(t1->rotation);
    float bx1 = -ay1;
	float by1 = ax1;

    float ex1 = (t1->mask.dimensions.x * t1->scale.x) * 0.5;
    float ey1 = (t1->mask.dimensions.y * t1->scale.y) * 0.5;

    float cx1 = t1->position.x + (t1->mask.offset.x * t1->scale.x * ax1) + (t1->mask.offset.y * t1->scale.y * bx1);
    float cy1 = t1->position.y + (t1->mask.offset.x * t1->scale.x * ay1) + (t1->mask.offset.y * t1->scale.y * by1);

    float ax2 = cosf(t2->rotation);
	float ay2 = sinf(t2->rotation);
    float bx2 = -ay2;
	float by2 = ax2;

    float ex2 = (t2->mask.dimensions.x * t2->scale.x) * 0.5;
    float ey2 = (t2->mask.dimensions.y * t2->scale.y) * 0.5;

    float cx2 = t2->position.x + (t2->mask.offset.x * t2->scale.x * ax2) + (t2->mask.offset.y * t2->scale.y * bx2);
    float cy2 = t2->position.y + (t2->mask.offset.x * t2->scale.x * ay2) + (t2->mask.offset.y * t2->scale.y * by2);

	Vector2 axes[4] = {
        {ax1, ay1}, {bx1, by1}, 
        {ax2, ay2}, {bx2, by2}
    };

    for(int i = 0; i < 4; i++) {
        float c1 = cx1 * axes[i].x + cy1 * axes[i].y;
		float c2 = cx2 * axes[i].x + cy2 * axes[i].y;
		float r1 = ex1 * ABS(ax1 * axes[i].x + ay1 * axes[i].y) + ey1 * ABS(bx1 * axes[i].x + by1 * axes[i].y); 
		float r2 = ex2 * ABS(ax2 * axes[i].x + ay2 * axes[i].y) + ey2 * ABS(bx2 * axes[i].x + by2 * axes[i].y); 
		if (ABS(c1 - c2) > (r1 + r2)) return false; 
    }

	return true;
}
