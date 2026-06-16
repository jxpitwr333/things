#include "things.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Animation ANIMATIONS[] = {
    [ANIM_GREEN] = {.frames = {2, 3}, .ticksPerFrame = 12, .loops = true},
    [ANIM_BULLET] = {.frames = {18, 19}, .ticksPerFrame = 1, .loops = false}};

void init(State *state) {
  state->things = malloc(MAX_THINGS * sizeof(Thing));
  state->activeIds = malloc(MAX_THINGS * sizeof(u16));
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

  kind_link(state, slot);

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
  kind_unlink(state, id);

  memset(state->things[id].alarms, -1, sizeof(state->things[id].alarms));
  state->things[id].kind = NILKIND;
  state->things[id].nextSibId = state->nextEmptySlot;
  state->nextEmptySlot = id;
}

void draw(Texture2D *spritesheet, Thing *thing) {
  u16 col = thing->spriteId % SHEET_COLUMNS;
  u16 row = thing->spriteId / SHEET_COLUMNS;

  float renderX = TO_FLOAT(thing->subX);
  float renderY = TO_FLOAT(thing->subY);

  float srcWidth = (float)TILE_SIZE * (float)thing->scaleX;
  float srcHeight = (float)TILE_SIZE * (float)thing->scaleY;

  DrawTexturePro(
      *spritesheet,
      (Rectangle){.x = col * TILE_SIZE,
                  .y = row * TILE_SIZE,
                  .width = srcWidth,
                  .height = srcHeight},
      (Rectangle){
          .x = renderX, .y = renderY, .width = TILE_SIZE, .height = TILE_SIZE},
      (Vector2){HALF_TILE_SIZE, HALF_TILE_SIZE}, (float)thing->rotation, WHITE);
}

void drawanim(Texture2D *spritesheet, Thing *thing, const Animation *anim) {
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

  u8 frameIndex = currentTick / anim->ticksPerFrame;

  u8 actualSpriteId = anim->frames[frameIndex];

  u16 col = actualSpriteId % SHEET_COLUMNS;
  u16 row = actualSpriteId / SHEET_COLUMNS;

  float renderX = TO_FLOAT(thing->subX);
  float renderY = TO_FLOAT(thing->subY);
  float srcWidth = (float)TILE_SIZE * (float)thing->scaleX;
  float srcHeight = (float)TILE_SIZE * (float)thing->scaleY;

  DrawTexturePro(
      *spritesheet,
      (Rectangle){col * TILE_SIZE, row * TILE_SIZE, srcWidth, srcHeight},
      (Rectangle){renderX, renderY, TILE_SIZE, TILE_SIZE},
      (Vector2){HALF_TILE_SIZE, HALF_TILE_SIZE}, (float)thing->rotation, WHITE);
}

void kind_link(State *state, u16 id) {
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

void kind_unlink(State *state, u16 id) {
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
  float rad1 = (float)t1->rotation * (PI / 180.0f);
  float rad2 = (float)t2->rotation * (PI / 180.0f);

  float c1 = cosf(rad1);
  float s1 = sinf(rad1);
  float c2 = cosf(rad2);
  float s2 = sinf(rad2);

  Vector2 axes[4] = {{c1, s1}, {-s1, c1}, {c2, s2}, {-s2, c2}};

  float p1x = TO_FLOAT(t1->subX);
  float p1y = TO_FLOAT(t1->subY);
  float p2x = TO_FLOAT(t2->subX);
  float p2y = TO_FLOAT(t2->subY);

  float s1x = (float)t1->scaleX;
  float s1y = (float)t1->scaleY;
  float s2x = (float)t2->scaleX;
  float s2y = (float)t2->scaleY;

  float hx1 = (float)t1->mask.width * s1x * 0.5f;
  float hy1 = (float)t1->mask.height * s1y * 0.5f;

  Vector2 center1 = {p1x + ((float)t1->mask.offsetX * s1x * c1) -
                         ((float)t1->mask.offsetY * s1y * s1),
                     p1y + ((float)t1->mask.offsetX * s1x * s1) +
                         ((float)t1->mask.offsetY * s1y * c1)};

  float hx2 = (float)t2->mask.width * s2x * 0.5f;
  float hy2 = (float)t2->mask.height * s2y * 0.5f;

  Vector2 center2 = {p2x + ((float)t2->mask.offsetX * s2x * c2) -
                         ((float)t2->mask.offsetY * s2y * s2),
                     p2y + ((float)t2->mask.offsetX * s2x * s2) +
                         ((float)t2->mask.offsetY * s2y * c2)};

  Vector2 d = {center2.x - center1.x, center2.y - center1.y};

  for (int i = 0; i < 4; i++) {
    Vector2 a = axes[i];
    float dist = ABS(d.x * a.x + d.y * a.y);

    float r1 = hx1 * ABS(axes[0].x * a.x + axes[0].y * a.y) +
               hy1 * ABS(axes[1].x * a.x + axes[1].y * a.y);
    float r2 = hx2 * ABS(axes[2].x * a.x + axes[2].y * a.y) +
               hy2 * ABS(axes[3].x * a.x + axes[3].y * a.y);

    if (dist > (r1 + r2))
      return false;
  }

  return true;
}

void draw_thing_mask(Thing *thing, Color color) {
  float px = TO_FLOAT(thing->subX);
  float py = TO_FLOAT(thing->subY);
  float sx = (float)thing->scaleX;
  float sy = (float)thing->scaleY;

  float rad = (float)thing->rotation * (PI / 180.0f);
  float c = cosf(rad);
  float s = sinf(rad);

  float hx = (float)thing->mask.width * sx * 0.5f;
  float hy = (float)thing->mask.height * sy * 0.5f;

  Vector2 center = {px + ((float)thing->mask.offsetX * sx * c) -
                        ((float)thing->mask.offsetY * sy * s),
                    py + ((float)thing->mask.offsetX * sx * s) +
                        ((float)thing->mask.offsetY * sy * c)};

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

void draw_debug_masks(State *state) {
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
        draw_thing_mask(t, kind_colors[k]);
      }

      current = t->nextSibId;
    } while (current != head);
  }
}
