/*
 * TODO: standardize thing stats in templates, otherwise i have to keep chasing
 * values around everytime i do a rewrite.
 */

#include "game.h"
#include <stdio.h>

State state;

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Things");
  SetTargetFPS(60);

  init(&state);

  Texture2D spritesheet = LoadTexture("assets/sheet.png");
  RenderTexture2D renderTexture = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
  Vector2 f_GameCenter =
      (Vector2){(float)(GAME_WIDTH) * 0.5f, (float)(GAME_HEIGHT) * 0.5f};
  Camera2D camera = {.offset = f_GameCenter,
                     .rotation = 0.0f,
                     .target = f_GameCenter,
                     .zoom = 1.0f};

  u16 ship_id = add(&state, (Thing){.kind = SHIPKIND,
                                    .subX = TO_FIXED_16(64),
                                    .subY = TO_FIXED_16(64),
                                    .scaleX = TO_FIXED_8(1),
                                    .scaleY = TO_FIXED_8(1),
                                    .spriteId = 0,
                                    .mask = {.width = 8, .height = 8}});

  while (!WindowShouldClose()) {
    for (u16 i = state.activeCount; i-- > 0;) {
      u16 id = state.activeIds[i];
      Thing *t = &state.things[id];

      switch(t->kind) {
          case PARTICLEKIND:
          particleUpdate(&state, t);
          break;
          case ALIENKIND:
          alienUpdate(t);
          break;
          case BULLETKIND:
          bulletUpdate(&state, t);
          break;
          default:
          break;
      }

      // increment alarm[0] for animations, decrement every other alarm.
      t->alarms[0]++;
      for (i16 j = 1; j < MAX_ALARMS; ++j) {
        if (t->alarms[j] > 0)
          t->alarms[j]--;
      }
    }

    shipUpdate(&state, ship_id);
    spawnerUpdate(&state);
    checkCollisions(&state, BULLETKIND, ALIENKIND, onBulletHitAlien);

    BeginTextureMode(renderTexture);
    BeginMode2D(camera);
    ClearBackground(BLACK);

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "things: %d", state.activeCount);
    DrawText(buffer, 20, 20, 8, hex2Color(GREEN_HEX));

    for (u16 i = 0; i < KIND_AMOUNT; ++i) {
        Kind k = DRAW_ORDER[i];
        u16 head = state.kindHeads[k];
        if (head == NIL) continue;

        u16 current = head;
        do {
            Thing *thing = &state.things[current];

            switch (k) {
                case PARTICLEKIND:
                    particleDraw(thing);
                    break;
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
            current = thing->nextSibId;
        } while (current != head);
    }

    drawDebugMasks(&state);
    EndMode2D();
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

  UnloadRenderTexture(renderTexture);
  UnloadTexture(spritesheet);
  CloseWindow();
  return 0;
}
