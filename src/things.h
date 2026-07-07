#ifndef THINGS_H
#define THINGS_H

#include <stdint.h>
#include <stdbool.h>
#include <raylib.h>

typedef uint16_t u16;
typedef int16_t i16;
typedef int8_t i8;
typedef uint8_t u8;
typedef int32_t i32;

#define MAX_THINGS ((u16)4096)
#define NIL ((u16)0)
#define MAX_ALARMS ((u16)4)
#define MAX_FRAMES 2

#define GAME_WIDTH 128
#define GAME_HEIGHT 128
#define GAME_HALFWIDTH 64
#define GAME_HALFHEIGHT 64
#define GAME_CENTER (Vector2){(float)GAME_HALFWIDTH, (float)GAME_HALFHEIGHT}
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define TILE_SIZE 8
#define HALF_TILE_SIZE 4

#define SHEET_COLUMNS ((u16)16)
#define SHEET_ROWS ((u16)16)

#define SCALE_16 ((float)256.0f)
#define SCALE_8 ((float)16.0f)
#define TO_FIXED_16(f) ((i16)((f) * SCALE_16))
#define TO_FLOAT_16(fx) ((float)(fx) / SCALE_16)
#define TO_FIXED_8(f) ((i8)((f) * SCALE_8))
#define TO_FLOAT_8(fx) ((float)(fx) / SCALE_8)

#define DEG2BRAD(theta) ((u8)((float)(theta) * (256.0f / 360.0f)))
#define BRAD2DEG(theta) ((float)(theta) * (360.0f / 256.0f))
#define RAD2BRAD(theta) ((u8)((float)(theta) * (256.0f / (2.0f * PI))))
#define BRAD2RAD(theta) ((float)(theta) * ((2.0f * PI) / 256.0f))

#define BLUE_HEX 0x1D2B53
#define MAROON_HEX 0x7E2553
#define GREEN_HEX 0x008751
#define BROWN_HEX 0xAB5236
#define DARK_GREY_HEX 0x5F574F
#define LIGHT_GREY_HEX 0xC2C3C7
#define WHITE_HEX 0xFFF1E8
#define RED_HEX 0xFF004D
#define ORANGE_HEX 0xFFA300
#define YELLOW_HEX 0xFFEC27
#define LIME_HEX 0x00E436
#define SKYBLUE_HEX 0x29ADFF
#define GREY_HEX 0x83769C
#define PINK_HEX 0xFF77A8
#define PEACH_HEX 0xFFCCAA

typedef enum {
  NILKIND,
  SHIPKIND,
  ALIENKIND,
  BULLETKIND,
  ENEMYBULLETKIND,
  PARTICLEKIND,
  KIND_AMOUNT,
} Kind;

extern const Kind DRAW_ORDER[];

typedef struct {
  i8 width, height;
} Mask;

typedef struct {
  // alarm[0] is reserved for ticking the thing's animation.
  i16 alarms[MAX_ALARMS];
  u16 id;
  u16 denseId;
  i16 subX;
  i16 subY;
  u16 parentId;
  u16 firstChildId;
  u16 nextSibId;
  u16 prevSibId;
  Mask mask;
  i8 scaleX;
  i8 scaleY;
  u8 rotation;
  u8 kind;
  i8 spriteId;
  i8 health;
} Thing;

#define ANIMATION_TICK(t) ((t)->alarms[0])
#define PARTICLE_LIFETIME(t) ((t)->alarms[1])
#define WEAPON_COOLDOWN(t) ((t)->alarms[1])
#define ALIEN_ROTATION_TIMER(t) ((t)->alarms[1])
#define ALIEN_HITFLASH_TIMER(t) ((t)->alarms[2])

#define PARTICLE_TYPE(t) ((t)->parentId)
#define ALIEN_COLOR(t) ((t)->firstChildId)

// for aliens
#define ANIMATION_ID(t) ((t)->spriteId)

typedef struct {
	Thing things[MAX_THINGS];
	Camera2D camera;
	Texture *spritesheet;
	float screenshake;
	u16 activeIds[MAX_THINGS];
	u16 kindHeads[KIND_AMOUNT];
	u16 activeCount;
	u16 nextEmptySlot;
} State;

typedef enum {
	ANIM_GREEN,
	ANIM_BULLET,
	ANIM_RED,
	ANIM_GREEN_FLASH,
	ANIM_RED_FLASH
} AnimNames;

typedef struct {
  bool loops;
  i8 ticksPerFrame;
  i8 frames[MAX_FRAMES];
} Animation;

extern const Animation ANIMATIONS[];

extern const i8 SINTABLE[256];
extern const i8 COSTABLE[256];

typedef void (*CollisionCallback)(State *state, u16 id1, u16 id2);

void init(State *state);
u16 add(State *state, Thing thing);
Thing *get(Thing *things, u16 id);
void rem(State *state, u16 id);
void drawThing(Texture2D *spritesheet, Thing *thing);
void drawAnim(Texture2D *spritesheet, Thing *thing, const Animation *anim);
void kindLink(State *state, u16 id);
void kindUnlink(State *state, u16 id);

bool checkAABB(Thing *t1, Thing *t2);
void drawThingMask(Thing *thing, Color color);
void drawDebugMasks(State *state);
void checkCollisions(State *state, Kind k1, Kind k2,
                     CollisionCallback onCollide);

int clamp(int value, int min, int max);
float fclamp(float value, float min, float max);

int randomRange(int min, int max);
float nextFloat();
Color hex2Color(i32 hex);
static inline void addScreenshake(State* state, float amount) { state->screenshake += amount; }
void updateScreenshake(State* state);

#endif
