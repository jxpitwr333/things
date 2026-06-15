#include "things.h"
#include <stdlib.h>

void ship_update(State *state, u16 id);

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Things");
  SetTargetFPS(60);

  State state;
  init(&state);

  Texture2D spritesheet = LoadTexture("assets/sheet.png");
  RenderTexture2D renderTexture = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);

  u16 ship_id = add(&state, (Thing){
    .kind = SHIPKIND,
    .subX = TO_FIXED(64),
    .subY = TO_FIXED(64),
    .scaleX = 1,
    .scaleY = 1,
    .spriteId = 0,
    .mask = {
        .width = 8,
        .height = 8
    }});

  kind_link(&state, ship_id);
  kind_link(&state, add(&state, (Thing){
    .kind = ALIENKIND,
    .subX = TO_FIXED(64),
    .subY = TO_FIXED(24),
    .scaleX = 1,
    .scaleY = 1,
    .spriteId = 2,
    .mask = {
        .width = 6,
        .height = 6
    }
  }));

  while (!WindowShouldClose()) {
    for (u16 i = 0; i < state.activeCount; ++i) {
      u16 id = state.activeIds[i];
      Thing *t = &state.things[id];

      t->alarms[0]++; // increment alarm[0] for animations decrement every other alarm.
      for (i16 j = 1; j < MAX_ALARMS; ++j) {
        if (t->alarms[j] > 0)
          t->alarms[j]--;
      }
    }

    ship_update(&state, ship_id);

    BeginTextureMode(renderTexture);
    ClearBackground(BLACK);

    for (u16 i = 0; i < state.activeCount; ++i) {
      u16 id = state.activeIds[i];

      if (state.things[id].kind == ALIENKIND) {
        drawanim(&spritesheet, &state.things[id], &ANIMATIONS[ANIM_GREEN]);
        continue;
      }

      draw(&spritesheet, &state.things[id]);
    }

    // draw_debug_masks(&state);
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

void ship_update(State *state, u16 id) {
  Thing *ship = get(state->things, id);

  int moveX = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
  int moveY = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);

  if (moveX != 0) {
    ship->spriteId = 1;
    ship->scaleX = (i8)moveX;
  } else {
    ship->spriteId = 0;
    ship->scaleX = (i8)1;
  }

  ship->subX += TO_FIXED(SHIP_SPD * moveX);
  ship->subY += TO_FIXED(SHIP_SPD * moveY);
}
