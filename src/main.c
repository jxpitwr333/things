/*
 * TODO: standardize thing stats in templates, otherwise i have to keep chasing
 * values around everytime i do a rewrite.
 */

#include "game.h"
#include "things.h"
#include <stdio.h>

State state = {0};
Director director = {0};

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Things");
  SetTargetFPS(60);

  init(&state);

  Texture2D spritesheet = LoadTexture("assets/sheet.png");
  state.spritesheet = &spritesheet;
  RenderTexture2D renderTexture = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);

  	u16 ship_id = add(&state, (Thing){.kind = SHIPKIND,
		.subX = TO_FIXED_16(64),
		.subY = TO_FIXED_16(64),
		.scaleX = TO_FIXED_8(1),
		.scaleY = TO_FIXED_8(1),
		.spriteId = 0,
		.maskWidth = TILE_SIZE - 2,
		.maskHeight = TILE_SIZE - 2,
		.health = 3
	});

  while (!WindowShouldClose()) {

	for (u16 i = state.activeCount; i-- > 0;) {
		u16 id = state.activeIds[i];
		Kind kind = state.things.kind[id];

		switch(kind) {
			case PARTICLEKIND:
				particleUpdate(&state, id);
				break;
			case ALIENKIND:
				alienUpdate(&state, id);
				break;
			case BULLETKIND:
				bulletUpdate(&state, id);
				break;
			default:
				break;
		}

		ANIMATION_TICK(&state, id)++;
		for (i16 j = 1; j < MAX_ALARMS; ++j) {
			if (state.things.alarms[id][j] > 0)
				state.things.alarms[id][j]--;
		}
	}

	shipUpdate(&state, ship_id);
	spawnerUpdate(&state, &director);
	checkCollisions(&state, BULLETKIND, ALIENKIND, onBulletHitAlien);
	updateScreenshake(&state);

    BeginTextureMode(renderTexture);
    BeginMode2D(state.camera);
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
            switch (k) {
                case PARTICLEKIND:
                    particleDraw(&state, current);
                    break;
                case BULLETKIND:
                    drawAnim(&state, current, &ANIMATIONS[ANIM_BULLET]);
                    break;
                case ALIENKIND: {
						drawAnim(&state, current, &ANIMATIONS[state.things.spriteId[current]]);
						break;
					}
				default:
					drawThing(&state, current);
				break;
            }
            current = state.things.nextSibId[current];
        } while (current != head);
    }

    //drawDebugMasks(&state);
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