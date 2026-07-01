#ifndef GAME_H
#define GAME_H

#include "things.h"

#define SHIP_SPD 2
#define BULLET_SPD (TO_FIXED_16(5))
#define SCREEN_TILES 16
#define MAX_FORMATION_OFFSETS 8
#define SECONDS(n) (n * ((i16)60))
// max colors for particle palettes
#define MAX_COLORS 8

#define ALIEN_ROTATION_SPD 4
#define ALIEN_ROTATION_AMPLITUDE DEG2BRAD(15)

typedef enum {
  FORMATION_V,
  FORMATION_THREE_WALL,
  FORMATION_FOUR_WALL,
  FORMATION_COUNT
} FormationType;

typedef struct {
  i8 x, y;
} Vector2_i8;

typedef struct {
  i8 min_tile;
  i8 max_tile;
  Vector2_i8 offsets[MAX_FORMATION_OFFSETS];
  u8 count;
} Formation;

extern const Formation FORMATIONS[];

// base values for particle templates
typedef struct {
  const i32 *colorPalette;
  i16 speed; // 8.8
  i8 shrink;
  i8 scale;
  i8 lifetime;
  u8 colorCount;
} Particle;

typedef enum {
  PARTICLE_EXHAUST,
  PARTICLE_EXPLOSION,
} ParticleType;

typedef enum {
   ALIEN_GREEN,
   ALIEN_ORANGE,
   ALIEN_RED
} AlienType;

extern const Particle PARTICLES[];
extern const i32 ALIEN_COLORS[];
extern const i32 PLACEHOLDER_PALETTE[];
extern const i32 EXHAUST_PALETTE[MAX_COLORS];

void particleUpdate(State* state, Thing* t);
void particleDraw(Thing* t);
void createExplosion(State* state, Thing* t);
void alienUpdate(Thing* t);
void bulletUpdate(State* state, Thing* t);
void shipUpdate(State *state, u16 id);
void spawnerUpdate(State *state);
void onBulletHitAlien(State *state, u16 bulletId, u16 alienId);

#endif
