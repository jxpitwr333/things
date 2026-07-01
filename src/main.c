/*
 * TODO: standardize thing stats in templates, otherwise i have to keep chasing
 * values around everytime i do a rewrite.
 */

#include "game.h"
#include "things.h"
#include <stdio.h>

_Bool g_IsRunning = 1;
State state;

void platform_create_window(int width, int height);
void platform_process_events(void);
void platform_present_buffer(const void *buffer, int width, int height);
void clearBuffer(u32* backbuffer, u32 color);

int main(void) {
    printf("Rat Engine starting up...\n");

    platform_create_window(WINDOW_WIDTH, WINDOW_HEIGHT);

	init(&state);

	u16 ship_id = add(&state, (Thing){
		.kind = SHIPKIND,
		.subX = TO_FIXED_16(64),
		.subY = TO_FIXED_16(64),
		.scaleX = TO_FIXED_8(1),
		.scaleY = TO_FIXED_8(1),
		.spriteId = 0,
		.mask = {.width = 8, .height = 8}
	});

    while (g_IsRunning) {
        platform_process_events();

		// update logic goes here.

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
		// updateScreenshake(&state);

		// draw logic goes here.

		clearBuffer(state.backbuffer, BLACK_HEX);

		for (u16 i = 0; i < KIND_AMOUNT - 1; ++i) {
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
						drawAnim(&state, thing, &ANIMATIONS[ANIM_BULLET]);
						break;
					case ALIENKIND:
						drawAnim(&state, thing, &ANIMATIONS[ANIM_GREEN]);
						break;
					default:
						drawThing(&state, thing);
						break;
				}
				current = thing->nextSibId;
			} while (current != head);
		}

        platform_present_buffer(state.backbuffer, GAME_WIDTH, GAME_HEIGHT);
    }

    return 0;
}

void clearBuffer(u32* backbuffer, u32 color) {
	for (int i = 0; i < GAME_WIDTH * GAME_HEIGHT; ++i) {
		backbuffer[i] = color;
	}
}