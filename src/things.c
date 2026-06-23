#include "things.h"
#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Animation ANIMATIONS[] = {
    [ANIM_GREEN] = {.frames = {2, 3}, .ticksPerFrame = 16, .loops = true},
    [ANIM_BULLET] = {.frames = {18, 19}, .ticksPerFrame = 1, .loops = false}};

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

bool checkOBB(Thing *t1, Thing *t2) {
  float rad1 = BRAD2RAD(t1->rotation);
  float rad2 = BRAD2RAD(t2->rotation);

  float c1 = cosf(rad1);
  float s1 = sinf(rad1);
  float c2 = cosf(rad2);
  float s2 = sinf(rad2);

  Vector2 axes[4] = {{c1, s1}, {-s1, c1}, {c2, s2}, {-s2, c2}};

  float p1x = TO_FLOAT_16(t1->subX);
  float p1y = TO_FLOAT_16(t1->subY);
  float p2x = TO_FLOAT_16(t2->subX);
  float p2y = TO_FLOAT_16(t2->subY);

  float s1x = TO_FLOAT_8(t1->scaleX);
  float s1y = TO_FLOAT_8(t1->scaleY);
  float s2x = TO_FLOAT_8(t2->scaleX);
  float s2y = TO_FLOAT_8(t2->scaleY);

  float hx1 = (float)t1->mask.width * s1x * 0.5f;
  float hy1 = (float)t1->mask.height * s1y * 0.5f;

  Vector2 center1 = {p1x, p1y};

  float hx2 = (float)t2->mask.width * s2x * 0.5f;
  float hy2 = (float)t2->mask.height * s2y * 0.5f;

  Vector2 center2 = {p2x, p2y};

  Vector2 d = {center2.x - center1.x, center2.y - center1.y};

  for (int i = 0; i < 4; i++) {
    Vector2 a = axes[i];
    float dist = fabsf(d.x * a.x + d.y * a.y);

    float r1 = hx1 * fabsf(axes[0].x * a.x + axes[0].y * a.y) +
               hy1 * fabsf(axes[1].x * a.x + axes[1].y * a.y);
    float r2 = hx2 * fabsf(axes[2].x * a.x + axes[2].y * a.y) +
               hy2 * fabsf(axes[3].x * a.x + axes[3].y * a.y);

    if (dist > (r1 + r2))
      return false;
  }

  return true;
}

void drawThingMask(Thing *thing, Color color) {
  float px = TO_FLOAT_16(thing->subX);
  float py = TO_FLOAT_16(thing->subY);

  float sx = TO_FLOAT_8(thing->scaleX);
  float sy = TO_FLOAT_8(thing->scaleY);

  float rad = BRAD2RAD(thing->rotation);
  float c = cosf(rad);
  float s = sinf(rad);

  float hx = (float)thing->mask.width * sx * 0.5f;
  float hy = (float)thing->mask.height * sy * 0.5f;

  Vector2 center = {px, py};

  Vector2 local_corners[4] = {{-hx, -hy}, {hx, -hy}, {hx, hy}, {-hx, hy}};

  Vector2 world_corners[4];
  for (int i = 0; i < 4; i++) {
    world_corners[i].x =
        center.x + (local_corners[i].x * c) - (local_corners[i].y * s);
    world_corners[i].y =
        center.y + (local_corners[i].x * s) + (local_corners[i].y * c);
  }

  for (int i = 0; i < 4; i++) {
    DrawLineV(world_corners[i], world_corners[(i + 1) % 4], color);
  }
}

void drawDebugMasks(State *state) {
  Color kind_colors[KIND_AMOUNT] = {[NILKIND] = BLANK,
                                    [SHIPKIND] = GREEN,
                                    [ALIENKIND] = RED,
                                    [BULLETKIND] = YELLOW};

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

        if (checkOBB(t1, t2)) {
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

int randomRange(int min, int max) {
	// [min, max)
	return (rand() % (max - min)) + min;
}