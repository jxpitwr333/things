#ifndef THINGS_H
#define THINGS_H

#include <raylib.h>
#include <stdint.h>

typedef uint16_t u16;
typedef int16_t i16;
typedef int8_t i8;
typedef uint8_t u8;

#define MAX_THINGS ((u16)4096)
#define NIL ((u16)0)
#define MAX_ALARMS ((u16)4)
#define MAX_FRAMES 2

#define GAME_WIDTH 128
#define GAME_HEIGHT 128
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

#define TILE_SIZE 8
#define HALF_TILE_SIZE 4

#define SHEET_COLUMNS ((u16)11)
#define SHEET_ROWS ((u16)2)

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SUB_POSITION ((float)256.0f)
#define TO_FIXED(f) ((i16)((f) * SUB_POSITION))
#define TO_FLOAT(fx) ((float)(fx) / SUB_POSITION)

typedef enum {
  NILKIND,
  SHIPKIND,
  ALIENKIND,
  BULLETKIND,
  PARTICLEKIND,
  KIND_AMOUNT,
} Kind;

typedef struct {
  i8 width, height;
} Mask;

typedef struct {
  u16 id;
  u16 denseId;

  u8 kind;
  i8 spriteId;

  i16 subX;
  i16 subY;
  i16 scaleX;
  i16 scaleY;
  i16 rotation;
  Mask mask;

  i16 alarms[MAX_ALARMS]; // alarm[0] is reserved for ticking the thing's
                          // animation.
  u16 parentId;
  u16 firstChildId;
  u16 nextSibId;
  u16 prevSibId;
} Thing;

typedef struct {
  Thing *things;
  u16 *activeIds;
  Texture *spritesheet;
  u16 kindHeads[KIND_AMOUNT];
  u16 activeCount;
  u16 nextEmptySlot;
  i16 spawnerCounter;
} State;

typedef enum { ANIM_GREEN, ANIM_BULLET } AnimNames;

typedef struct {
	bool loops;
	i8 ticksPerFrame;
	i8 frames[MAX_FRAMES];
} Animation;

extern const Animation ANIMATIONS[];

typedef void (*CollisionCallback)(State *state, u16 id1, u16 id2);

void init(State *state);
u16 add(State *state, Thing thing);
Thing *get(Thing *things, u16 id);
void rem(State *state, u16 id);
void drawThing(Texture2D *spritesheet, Thing *thing);
void drawAnim(Texture2D *spritesheet, Thing *thing, const Animation *anim);
void kindLink(State *state, u16 id);
void kindUnlink(State *state, u16 id);

bool checkOBB(Thing *t1, Thing *t2);
void drawThingMask(Thing *thing, Color color);
void drawDebugMasks(State *state);
void checkCollisions(State *state, Kind k1, Kind k2,
                     CollisionCallback onCollide);

static inline int clamp(int value, int min, int max) {
  return (value > max ? max : (value < min ? min : value));
}
static inline float fclamp(float value, float min, float max) {
  return (value > max ? max : (value < min ? min : value));
}
int randomRange(int min, int max);

#endif
