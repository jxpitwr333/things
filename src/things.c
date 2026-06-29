#include "things.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Animation ANIMATIONS[] = {
    [ANIM_GREEN] = {.frames = {2, 3}, .ticksPerFrame = 16, .loops = true},
    [ANIM_BULLET] = {.frames = {18, 19}, .ticksPerFrame = 1, .loops = false}};

const i8 SINTABLE[256] = {
       0,    3,    6,    9,   13,   16,   19,   22,   25,   28,   31,   34,   37,   40,   43,   46,
      49,   52,   55,   58,   60,   63,   66,   68,   71,   74,   76,   79,   81,   84,   86,   88,
      91,   93,   95,   97,   99,  101,  103,  105,  106,  108,  110,  111,  113,  114,  116,  117,
     118,  119,  121,  122,  122,  123,  124,  125,  126,  126,  127,  127,  127,  127,  127,  127,
     127,  127,  127,  127,  127,  127,  127,  126,  126,  125,  124,  123,  122,  122,  121,  119,
     118,  117,  116,  114,  113,  111,  110,  108,  106,  105,  103,  101,   99,   97,   95,   93,
      91,   88,   86,   84,   81,   79,   76,   74,   71,   68,   66,   63,   60,   58,   55,   52,
      49,   46,   43,   40,   37,   34,   31,   28,   25,   22,   19,   16,   13,    9,    6,    3,
       0,   -3,   -6,   -9,  -13,  -16,  -19,  -22,  -25,  -28,  -31,  -34,  -37,  -40,  -43,  -46,
     -49,  -52,  -55,  -58,  -60,  -63,  -66,  -68,  -71,  -74,  -76,  -79,  -81,  -84,  -86,  -88,
     -91,  -93,  -95,  -97,  -99, -101, -103, -105, -106, -108, -110, -111, -113, -114, -116, -117,
    -118, -119, -121, -122, -122, -123, -124, -125, -126, -126, -127, -127, -127, -128, -128, -128,
    -128, -128, -128, -128, -127, -127, -127, -126, -126, -125, -124, -123, -122, -122, -121, -119,
    -118, -117, -116, -114, -113, -111, -110, -108, -106, -105, -103, -101,  -99,  -97,  -95,  -93,
     -91,  -88,  -86,  -84,  -81,  -79,  -76,  -74,  -71,  -68,  -66,  -63,  -60,  -58,  -55,  -52,
     -49,  -46,  -43,  -40,  -37,  -34,  -31,  -28,  -25,  -22,  -19,  -16,  -13,   -9,   -6,   -3,
};

const i8 COSTABLE[256]= {
     127,  127,  127,  127,  127,  127,  127,  126,  126,  125,  124,  123,  122,  122,  121,  119,
     118,  117,  116,  114,  113,  111,  110,  108,  106,  105,  103,  101,   99,   97,   95,   93,
      91,   88,   86,   84,   81,   79,   76,   74,   71,   68,   66,   63,   60,   58,   55,   52,
      49,   46,   43,   40,   37,   34,   31,   28,   25,   22,   19,   16,   13,    9,    6,    3,
       0,   -3,   -6,   -9,  -13,  -16,  -19,  -22,  -25,  -28,  -31,  -34,  -37,  -40,  -43,  -46,
     -49,  -52,  -55,  -58,  -60,  -63,  -66,  -68,  -71,  -74,  -76,  -79,  -81,  -84,  -86,  -88,
     -91,  -93,  -95,  -97,  -99, -101, -103, -105, -106, -108, -110, -111, -113, -114, -116, -117,
    -118, -119, -121, -122, -122, -123, -124, -125, -126, -126, -127, -127, -127, -128, -128, -128,
    -128, -128, -128, -128, -127, -127, -127, -126, -126, -125, -124, -123, -122, -122, -121, -119,
    -118, -117, -116, -114, -113, -111, -110, -108, -106, -105, -103, -101,  -99,  -97,  -95,  -93,
     -91,  -88,  -86,  -84,  -81,  -79,  -76,  -74,  -71,  -68,  -66,  -63,  -60,  -58,  -55,  -52,
     -49,  -46,  -43,  -40,  -37,  -34,  -31,  -28,  -25,  -22,  -19,  -16,  -13,   -9,   -6,   -3,
       0,    3,    6,    9,   13,   16,   19,   22,   25,   28,   31,   34,   37,   40,   43,   46,
      49,   52,   55,   58,   60,   63,   66,   68,   71,   74,   76,   79,   81,   84,   86,   88,
      91,   93,   95,   97,   99,  101,  103,  105,  106,  108,  110,  111,  113,  114,  116,  117,
     118,  119,  121,  122,  122,  123,  124,  125,  126,  126,  127,  127,  127,  127,  127,  127,
};

const Kind DRAW_ORDER[] = {
    PARTICLEKIND,
    ALIENKIND,
    BULLETKIND,
    SHIPKIND
};

void init(State *state) {
  state->activeCount = 0;
  memset(state->activeIds, NIL, MAX_THINGS * sizeof(u16));

  state->things[NIL] = (Thing){.id = NIL, .kind = NILKIND};

  for (int i = 1; i < MAX_THINGS - 1; ++i) {
    state->things[i] = (Thing){.id = i, .kind = NILKIND, .nextSibId = i + 1};
    memset(state->things[i].alarms, -1, sizeof(i16) * MAX_ALARMS);
  }

  state->things[MAX_THINGS - 1] =
      (Thing){.id = MAX_THINGS - 1, .kind = NILKIND, .nextSibId = NIL};

  state->nextEmptySlot = 1;

  memset(&state->kindHeads, NIL, sizeof(state->kindHeads));

  state->spawnerCounter = 0;
  state->sleepTime = 0;
  state->screenshake = 0.0f;
  state->camera = (Camera2D){
      .offset = GAME_CENTER,
      .rotation = 0.0f,
      .target = GAME_CENTER,
      .zoom = 1.0f
  };
}

u16 add(State *state, Thing thing) {
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

  state->things[slot].denseId = state->activeCount;
  state->activeIds[state->activeCount] = slot;
  state->activeCount++;

  kindLink(state, slot);

  return slot;
}

Thing *get(Thing *things, u16 id) {
  return (id > 0 && id < MAX_THINGS) ? &things[id] : &things[NIL];
}

void rem(State *state, u16 id) {
  if (id <= NIL || id >= MAX_THINGS || state->things[id].kind == NILKIND) {
    printf("nothing to do\n");
    return;
  }

  u16 deadDenseId = state->things[id].denseId;
  if (deadDenseId < state->activeCount - 1) {
    u16 lastEntityId = state->activeIds[state->activeCount - 1];
    state->activeIds[deadDenseId] = lastEntityId;
    state->things[lastEntityId].denseId = deadDenseId;
  }

  state->activeCount--;
  state->things[id].denseId = 0;
  kindUnlink(state, id);

  memset(state->things[id].alarms, -1, sizeof(state->things[id].alarms));
  state->things[id].kind = NILKIND;
  state->things[id].nextSibId = state->nextEmptySlot;
  state->nextEmptySlot = id;
}

static void drawSprite(Texture2D *spritesheet, i16 spriteId, i16 subX, i16 subY, i8 scaleX, i8 scaleY, u8 rotation) {
  u16 col = spriteId % SHEET_COLUMNS;
  u16 row = spriteId / SHEET_COLUMNS;

  float renderX = floorf(TO_FLOAT_16(subX));
  float renderY = floorf(TO_FLOAT_16(subY));

  float f32TileSize = (float)TILE_SIZE;
  float absScaleX = fabsf(TO_FLOAT_8(scaleX));
  float absScaleY = fabsf(TO_FLOAT_8(scaleY));

  float destWidth = f32TileSize * absScaleX;
  float destHeight = f32TileSize * absScaleY;

  float srcWidth = f32TileSize;
  float srcHeight = f32TileSize;
  if (scaleX < 0) srcWidth = -srcWidth;
  if (scaleY < 0) srcHeight = -srcHeight;

  DrawTexturePro(
      *spritesheet,
      (Rectangle){.x = col * TILE_SIZE, .y = row * TILE_SIZE, .width = srcWidth, .height = srcHeight},
      (Rectangle){.x = renderX, .y = renderY, .width = destWidth, .height = destHeight},
      (Vector2){destWidth * 0.5f, destHeight * 0.5f},
      (float)BRAD2DEG(rotation), WHITE);
}

void drawAnim(Texture2D *spritesheet, Thing *thing, const Animation *anim) {
  i16 currentTick = thing->alarms[0];

  int totalAnimationTicks = anim->ticksPerFrame * MAX_FRAMES;

  if (currentTick >= totalAnimationTicks) {
    if (anim->loops) {
      currentTick = currentTick % totalAnimationTicks;
      thing->alarms[0] = currentTick;
    } else {
      currentTick = totalAnimationTicks - 1;
    }
  }

  i8 frameIndex = currentTick / anim->ticksPerFrame;

  i8 actualSpriteId = anim->frames[frameIndex];

  drawSprite(spritesheet, actualSpriteId, thing->subX, thing->subY, thing->scaleX, thing->scaleY, thing->rotation);
}

void drawThing(Texture2D* spritesheet, Thing* thing) {
    drawSprite(spritesheet, thing->spriteId, thing->subX, thing->subY, thing->scaleX, thing->scaleY, thing->rotation);
}

void kindLink(State *state, u16 id) {
  if (state->things[id].kind == NILKIND)
    return;
  Thing *thing = &state->things[id];
  Kind k = thing->kind;

  u16 head = state->kindHeads[k];

  if (head == NIL) {
    state->kindHeads[k] = id;
    thing->nextSibId = id;
    thing->prevSibId = id;
  } else {
    u16 tail = state->things[head].prevSibId;
    state->things[tail].nextSibId = id;
    thing->prevSibId = tail;

    thing->nextSibId = head;
    state->things[head].prevSibId = id;
  }
}

void kindUnlink(State *state, u16 id) {
  if (state->things[id].kind == NILKIND)
    return;
  Thing *thing = &state->things[id];
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

bool checkAABB(Thing *t1, Thing *t2) {
    i16 s1x = t1->scaleX < 0 ? -t1->scaleX : t1->scaleX;
    i16 s1y = t1->scaleY < 0 ? -t1->scaleY : t1->scaleY;
    i16 s2x = t2->scaleX < 0 ? -t2->scaleX : t2->scaleX;
    i16 s2y = t2->scaleY < 0 ? -t2->scaleY : t2->scaleY;

    // 16 fixed point division leads to 8 fixed point so we instead multiply then bitshift by 3 so we get the half point.
    i16 hx1 = (t1->mask.width  * s1x) << 3;
    i16 hy1 = (t1->mask.height * s1y) << 3;
    i16 hx2 = (t2->mask.width  * s2x) << 3;
    i16 hy2 = (t2->mask.height * s2y) << 3;

    i16 dx = t1->subX - t2->subX;
    i16 dy = t1->subY - t2->subY;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    return (dx < (hx1 + hx2)) && (dy < (hy1 + hy2));
}

void drawThingMask(Thing *thing, Color color) {
    float px = TO_FLOAT_16(thing->subX);
    float py = TO_FLOAT_16(thing->subY);
    float sx = fabsf(TO_FLOAT_8(thing->scaleX));
    float sy = fabsf(TO_FLOAT_8(thing->scaleY));

    float hx = (float)thing->mask.width * sx * 0.5f;
    float hy = (float)thing->mask.height * sy * 0.5f;

    DrawRectangleLines(
        (int)(px - hx),
        (int)(py - hy),
        (int)(hx * 2.0f),
        (int)(hy * 2.0f),
        color
    );
}

void drawDebugMasks(State *state) {
  	Color kind_colors[KIND_AMOUNT] = {
		[NILKIND] = BLANK,
		[SHIPKIND] = hex2Color(GREEN_HEX),
		[ALIENKIND] = hex2Color(RED_HEX),
		[BULLETKIND] = hex2Color(YELLOW_HEX)
	};

  for (int k = 1; k < KIND_AMOUNT; k++) {
    u16 head = state->kindHeads[k];
    if (head == NIL)
      continue;

    u16 current = head;
    do {
      Thing *t = &state->things[current];

      if (t->mask.width != 0 && t->mask.height != 0) {
        drawThingMask(t, kind_colors[k]);
      }

      current = t->nextSibId;
    } while (current != head);
  }
}

void checkCollisions(State *state, Kind k1, Kind k2,
                     CollisionCallback onCollide) {
  u16 head1 = state->kindHeads[k1];
  u16 head2 = state->kindHeads[k2];

  if (head1 == NIL || head2 == NIL)
    return;

  u16 currentThing1 = head1;
  do {
    bool thing1Removed = false;
    u16 next1 = state->things[currentThing1].nextSibId;
    Thing *t1 = &state->things[currentThing1];

    u16 currentThing2 = head2;
    if (currentThing2 != NIL) {
      do {
        u16 next2 = state->things[currentThing2].nextSibId;
        Thing *t2 = &state->things[currentThing2];

        if (checkAABB(t1, t2)) {
          if (currentThing1 == head1)
            head1 = state->things[currentThing1].nextSibId;
          if (currentThing2 == head2)
            head2 = state->things[currentThing2].nextSibId;

          onCollide(state, currentThing1, currentThing2);

          thing1Removed = true;
          break;
        }

        currentThing2 = next2;
        if (state->kindHeads[k2] == NIL) {
          head2 = NIL;
          break;
        }
      } while (currentThing2 != head2);
    }

    if (thing1Removed) {
      currentThing1 = next1;
    } else {
      currentThing1 = t1->nextSibId;
    }

    if (state->kindHeads[k1] == NIL) {
      head1 = NIL;
      break;
    }

  } while (currentThing1 != head1);
}

int clamp(int value, int min, int max) {
  return (value > max ? max : (value < min ? min : value));
}

float fclamp(float value, float min, float max) {
  return (value > max ? max : (value < min ? min : value));
}

int randomRange(int min, int max) {
	// [min, max)
	return (rand() % (max - min)) + min;
}

float nextFloat() {
    return (float)rand() / (float)RAND_MAX;
}

Color hex2Color(i32 hex) {
  return (Color){.r = (u8)((hex >> 16) & 0xFF),
                 .g = (u8)((hex >> 8) & 0xFF),
                 .b = (u8)(hex & 0xFF),
                 .a = 255};
}

void updateScreenshake(State* state) {
    if (state->screenshake >= 10.0) state->screenshake *= 0.8;
    if (state->screenshake > 0.0) {
        state->screenshake -= 1.0;
    } else {
	    state->screenshake = 0.0;
	}

	Vector2 shake_offset = {0, 0};
	if (state->screenshake > 0.0) {
		shake_offset = (Vector2){
			nextFloat() * state->screenshake - state->screenshake / 2.0,
			nextFloat() * state->screenshake - state->screenshake / 2.0,
		};
	}

	state->camera.offset = (Vector2){(GAME_CENTER.x + shake_offset.x), (GAME_CENTER.y + shake_offset.y)};
}
