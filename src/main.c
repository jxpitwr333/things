/*
 * TODO: standardize thing stats in templates, otherwise i have to keep chasing
 * values around everytime i do a rewrite.
 * TODO: draw order
 */

#include "things.h"
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#define SHIP_SPD 2
#define BULLET_SPD 5
#define SCREEN_TILES 16
#define MAX_FORMATION_OFFSETS 8
#define SECONDS(n) (n * ((i16)60))
#define RAW_TO_COLOR(ptr) ((Color){(ptr)[0], (ptr)[1], (ptr)[2], (ptr)[3]})
#define ALIEN_ROTATION_SPD 5.0f
#define ALIEN_ROTATION_AMPLITUDE 15.0f

typedef enum {
  FORMATION_V,
  FORMATION_THREE_WALL,
  FORMATION_FOUR_WALL,
  FORMATION_COUNT
} FormationType;

typedef struct {
  i16 x, y;
} Vector2Short;

typedef struct {
  u8 count;
  i16 min_tile;
  i16 max_tile;
  Vector2Short offsets[MAX_FORMATION_OFFSETS];
} Formation;

const Formation FORMATIONS[] = {
    [FORMATION_V] = {.min_tile = 2,
                     .max_tile = SCREEN_TILES - 3,
                     .count = 5,
                     .offsets = {{0, 0}, {-1, -1}, {1, -1}, {-2, -2}, {2, -2}}},
    [FORMATION_THREE_WALL] = {.min_tile = 0,
                              .max_tile = SCREEN_TILES - 3,
                              .count = 3,
                              .offsets = {{0, 0}, {1, 0}, {2, 0}}},
    [FORMATION_FOUR_WALL] = {.min_tile = 0,
                             .max_tile = SCREEN_TILES - 4,
                             .count = 4,
                             .offsets = {{0, 0}, {1, 0}, {2, 0}, {3, 0}}},
};

// max colors for particle palettes
#define MAX_COLORS 8

// base values for particle templates
typedef struct {
  i8 lifetime;
  i8 speed;
  u8 colorCount;
  i16 shrink;
  i16 scale; // fixed point
  const u8 (*colorPalette)[MAX_COLORS][4];
} Particle;

typedef enum {
  PARTICLE_EXHAUST,
} ParticleType;

const u8 exhaustPalette[MAX_COLORS][4] = {
    {130, 130, 130, 255},
    {255, 161, 0, 255},
    {253, 249, 0, 255},
    {255, 255, 255, 255},
};

Particle PARTICLES[] = {
    [PARTICLE_EXHAUST] = {.scale = TO_FIXED(2.5),
                          .shrink = TO_FIXED(0.1),
                          .lifetime = 16,
                          .colorCount = 4,
                          .colorPalette = &exhaustPalette},
};

void shipUpdate(State *state, u16 id);
void spawnerUpdate(State *state);
void onBulletHitAlien(State *state, u16 bulletId, u16 alienId);

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Things");
  SetTargetFPS(60);

  State state;
  init(&state);

  Texture2D spritesheet = LoadTexture("assets/sheet.png");
  RenderTexture2D renderTexture = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);

  u16 ship_id = add(&state, (Thing){.kind = SHIPKIND,
                                    .subX = TO_FIXED(64),
                                    .subY = TO_FIXED(64),
                                    .scaleX = TO_FIXED(1),
                                    .scaleY = TO_FIXED(1),
                                    .spriteId = 0,
                                    .mask = {.width = 8, .height = 8}});

  while (!WindowShouldClose()) {
    for (u16 i = state.activeCount; i-- > 0;) {
      u16 id = state.activeIds[i];
      Thing *t = &state.things[id];

      if (t->kind == PARTICLEKIND) {
        // alarm[1] is used for lifetime.
        if (t->alarms[1] == 0) {
          rem(&state, id);
          continue;
        }

        // for particles, the parentId field is repurposed for the particle type
        // indicator
        Particle template = PARTICLES[t->parentId];
        if (template.shrink != 0) {
          t->scaleX -= template.shrink;
          t->scaleY -= template.shrink;
        }
      }

      if (t->kind == ALIENKIND) {
        t->rotation = (i16)(sinf(GetTime() * ALIEN_ROTATION_SPD) *
                            ALIEN_ROTATION_AMPLITUDE);
        t->subY += TO_FIXED(0.25);
      }

      if (t->kind == BULLETKIND) {
        float rad = (float)t->rotation * (PI / 180.0f);

        t->subX += TO_FIXED(cosf(rad) * BULLET_SPD);
        t->subY += TO_FIXED(sinf(rad) * BULLET_SPD);

        if (t->subY <= 0) {
          rem(&state, id);
          continue;
        }
      }

      t->alarms[0]++; // increment alarm[0] for animations decrement every other
                      // alarm.
      for (i16 j = 1; j < MAX_ALARMS; ++j) {
        if (t->alarms[j] > 0)
          t->alarms[j]--;
      }
    }

    shipUpdate(&state, ship_id);
    spawnerUpdate(&state);
    checkCollisions(&state, BULLETKIND, ALIENKIND, onBulletHitAlien);

    BeginTextureMode(renderTexture);
    ClearBackground(BLACK);

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "things: %d", state.activeCount);
    DrawText(buffer, 20, 20, 8, GREEN);

    for (u16 i = 0; i < state.activeCount; ++i) {
      u16 id = state.activeIds[i];
      Thing *thing = &state.things[id];

      switch (thing->kind) {
      case PARTICLEKIND: {
        Particle template = PARTICLES[thing->parentId];
        if (template.colorCount != 0) {
          const u8(*palette)[4] = *template.colorPalette;
          if (template.colorCount == 1) {
            DrawEllipse((int)TO_FLOAT(thing->subX), (int)TO_FLOAT(thing->subY),
                        TO_FLOAT(thing->scaleX), TO_FLOAT(thing->scaleY),
                        RAW_TO_COLOR(palette[0]));
          } else {
            float percentage =
                (float)thing->alarms[1] / (float)template.lifetime;
            int idx = (int)floorf(percentage * (float)template.colorCount);

            DrawEllipse(
                (int)TO_FLOAT(thing->subX), (int)TO_FLOAT(thing->subY),
                TO_FLOAT(thing->scaleX), TO_FLOAT(thing->scaleY),
                RAW_TO_COLOR(palette[clamp(idx, 0, template.colorCount - 1)]));
          }
        }
        break;
      }

      case BULLETKIND:
        drawAnim(&spritesheet, thing, &ANIMATIONS[ANIM_BULLET]);
        break;

      case ALIENKIND:
        drawAnim(&spritesheet, thing, &ANIMATIONS[ANIM_GREEN]);
        break;

      default:
        drawThing(&spritesheet, thing);
        break;
      }
    }

    // drawDebugMasks(&state);
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);

    SetTextureFilter(renderTexture.texture, TEXTURE_FILTER_POINT);
    DrawTexturePro(renderTexture.texture,
                   (Rectangle){0, 0, GAME_WIDTH, -GAME_HEIGHT},
                   (Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT},
                   (Vector2){0, 0}, 0.0, WHITE);

    EndDrawing();
  }

  free(state.things);
  free(state.activeIds);
  UnloadRenderTexture(renderTexture);
  UnloadTexture(spritesheet);
  CloseWindow();
  return 0;
}

void shipUpdate(State *state, u16 id) {
  Thing *ship = get(state->things, id);

  int moveX = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
  int moveY = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);

  if (IsKeyDown(KEY_SPACE) && ship->alarms[1] == 0) {
    ship->alarms[1] = 7;

    add(state, (Thing){
                   .kind = BULLETKIND,
                   .subX = ship->subX,
                   .subY = ship->subY,
                   .rotation = 270,
                   .scaleX = TO_FIXED(1),
                   .scaleY = TO_FIXED(1),
                   .mask = {.width = 8, .height = 8},
               });
  }

  if (moveX != 0) {
    ship->spriteId = 1;
    ship->scaleX = TO_FIXED(moveX);
  } else {
    ship->spriteId = 0;
    ship->scaleX = TO_FIXED(1);
  }

  if (moveX != 0 || moveY != 0) {
    i16 scale = PARTICLES[PARTICLE_EXHAUST].scale;
    add(state, (Thing){.kind = PARTICLEKIND,
                       .parentId = PARTICLE_EXHAUST,
                       .subX = ship->subX + TO_FIXED(randomRange(-2, 2)),
                       .subY = ship->subY + TO_FIXED(randomRange(-2, 2)),
                       .scaleX = scale,
                       .scaleY = scale,
                       .alarms = {[1] = PARTICLES[PARTICLE_EXHAUST].lifetime}});
  }

  ship->subX += TO_FIXED(SHIP_SPD * moveX);
  ship->subY += TO_FIXED(SHIP_SPD * moveY);

  ship->subX = TO_FIXED(fclamp(TO_FLOAT(ship->subX), (float)HALF_TILE_SIZE,
                               (float)(GAME_WIDTH - HALF_TILE_SIZE)));
  ship->subY = TO_FIXED(fclamp(TO_FLOAT(ship->subY), (float)HALF_TILE_SIZE,
                               (float)(GAME_HEIGHT - HALF_TILE_SIZE)));
}

void spawnerUpdate(State *state) {
  state->spawnerCounter++;

  if (state->spawnerCounter >= SECONDS(3)) {
    state->spawnerCounter = 0;

    Formation chosenFormation = FORMATIONS[randomRange(0, FORMATION_COUNT)];
    i16 baseTile =
        randomRange(chosenFormation.min_tile, chosenFormation.max_tile + 1);

    for (i16 i = 0; i < chosenFormation.count; ++i) {
      Vector2Short offset = chosenFormation.offsets[i];
      i16 tileX = baseTile + offset.x;
      i16 posY = -TILE_SIZE + (offset.y * TILE_SIZE);
      add(state, (Thing){
                     .kind = ALIENKIND,
                     .subX = TO_FIXED(tileX * TILE_SIZE + HALF_TILE_SIZE),
                     .subY = TO_FIXED(posY),
                     .mask = {.width = 6, .height = 6},
                     .scaleX = TO_FIXED(1),
                     .scaleY = TO_FIXED(1),
                 });
    }
  }
}

void onBulletHitAlien(State *state, u16 bulletId, u16 alienId) {
  rem(state, bulletId);
  rem(state, alienId);
}
