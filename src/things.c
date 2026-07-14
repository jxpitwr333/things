#include "things.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Animation ANIMATIONS[] = {
    [ANIM_GREEN] = {.frames = {2, 3}, .ticksPerFrame = 16, .loops = true},
    [ANIM_BULLET] = {.frames = {18, 19}, .ticksPerFrame = 1, .loops = false},
    [ANIM_RED] = {.frames = {6, 7}, .ticksPerFrame = 16, .loops = true},
    [ANIM_GREEN_FLASH] = {.frames = {22, 23}, .ticksPerFrame = 16, .loops = true},
    [ANIM_RED_FLASH] = {.frames = {26, 27}, .ticksPerFrame = 16, .loops = true},
};

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
    SHIPKIND,
    ENEMYBULLETKIND,
};

void init(State *state) {
    state->activeCount = 0;
    memset(state->activeIds, NIL, MAX_THINGS * sizeof(u16));

    state->things.id[NIL] = NIL;
    state->things.kind[NIL] = NILKIND;

    for (int i = 1; i < MAX_THINGS - 1; ++i) {
        state->things.id[i] = i;
        state->things.kind[i] = NILKIND;
        state->things.nextSibId[i] = i + 1;

        memset(state->things.alarms[i], -1, sizeof(i16) * MAX_ALARMS); // is this how i do it?
    }

    state->things.id[MAX_THINGS - 1] = MAX_THINGS - 1;
    state->things.kind[MAX_THINGS - 1] = NILKIND;
    state->things.nextSibId[MAX_THINGS - 1] = NIL;

    state->nextEmptySlot = 1;

    memset(&state->kindHeads, NIL, sizeof(state->kindHeads));

    state->screenshake = 0.0f;
    state->camera = (Camera2D){
        .offset = GAME_CENTER,
        .rotation = 0.0f,
        .target = GAME_CENTER,
        .zoom = 1.0f
    };
}

u16 add(State* state, Thing thing) {
    if (state->nextEmptySlot == NIL) {
        printf("out of memory\n");
        return NIL;
    }

    u16 slot = state->nextEmptySlot;
    state->nextEmptySlot = state->things.nextSibId[slot];

    state->things.subX[slot] = thing.subX;
    state->things.subY[slot] = thing.subY;

    state->things.maskWidth[slot] = thing.maskWidth;
    state->things.maskHeight[slot] = thing.maskHeight;
    state->things.scaleX[slot] = thing.scaleX;
    state->things.scaleY[slot] = thing.scaleY;
    state->things.rotation[slot] = thing.rotation;
    state->things.kind[slot] = thing.kind;
    state->things.spriteId[slot] = thing.spriteId;
    state->things.health[slot] = thing.health;
    memcpy(state->things.alarms[slot], thing.alarms, sizeof(thing.alarms));

    state->things.id[slot] = slot;

    state->things.parentId[slot] = NIL;
    state->things.firstChildId[slot] = NIL;
    state->things.nextSibId[slot] = NIL;
    state->things.prevSibId[slot] = NIL;

    state->things.denseId[slot] = state->activeCount;
    state->activeIds[state->activeCount] = slot;
    state->activeCount++;

    kindLink(state, slot);

    return slot;
}

void rem(State *state, u16 id) {
    if (id <= NIL || id >= MAX_THINGS || state->things.kind[id] == NILKIND) {
        printf("nothing to do\n");
        return;
    }

    u16 deadDenseId = state->things.denseId[id];
    if (deadDenseId < state->activeCount - 1) {
        u16 lastEntityId = state->activeIds[state->activeCount - 1];
        state->activeIds[deadDenseId] = lastEntityId;
        state->things.denseId[lastEntityId] = deadDenseId;
    }

    state->activeCount--;
    state->things.denseId[id] = 0;
    kindUnlink(state, id);

    memset(state->things.alarms[id], -1, sizeof(state->things.alarms[id]));
    state->things.kind[id] = NILKIND;
    state->things.nextSibId[id] = state->nextEmptySlot;
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

void drawAnim(State *state, u16 id, const Animation *anim) {
    i16 currentTick = ANIMATION_TICK(state, id);

    int totalAnimationTicks = anim->ticksPerFrame * MAX_FRAMES;

    if (currentTick >= totalAnimationTicks) {
        if (anim->loops) {
            currentTick = currentTick % totalAnimationTicks;
            ANIMATION_TICK(state, id) = currentTick;
        } else {
            currentTick = totalAnimationTicks - 1;
        }
    }

    i8 frameIndex = currentTick / anim->ticksPerFrame;

    i8 actualSpriteId = anim->frames[frameIndex];

    drawSprite(state->spritesheet, actualSpriteId, state->things.subX[id], state->things.subY[id], state->things.scaleX[id], state->things.scaleY[id], state->things.rotation[id]);
}

void drawThing(State *state, u16 id) {
    drawSprite(state->spritesheet, state->things.spriteId[id], state->things.subX[id], state->things.subY[id], state->things.scaleX[id], state->things.scaleY[id], state->things.rotation[id]);
}

void kindLink(State *state, u16 id) {
    if (state->things.kind[id] == NILKIND)
        return;
    Kind k = state->things.kind[id];

    u16 head = state->kindHeads[k];

    if (head == NIL) {
        state->kindHeads[k] = id;
        state->things.nextSibId[id] = id;
        state->things.prevSibId[id] = id;
    } else {
        u16 tail = state->things.prevSibId[head];
        state->things.nextSibId[tail] = id;
        state->things.prevSibId[id] = tail;

        state->things.nextSibId[id] = head;
        state->things.prevSibId[head] = id;
    }
}

void kindUnlink(State *state, u16 id) {
    if (state->things.kind[id] == NILKIND)
        return;
    Kind k = state->things.kind[id];

    u16 next = state->things.nextSibId[id];
    u16 prev = state->things.prevSibId[id];

    if (next == id) {
        state->kindHeads[k] = NIL;
    } else {
        state->things.nextSibId[prev] = next;
        state->things.prevSibId[next] = prev;

        if (state->kindHeads[k] == id) {
            state->kindHeads[k] = next;
        }
    }
}

bool checkAABB(Things *things, u16 t1, u16 t2) {
    i32 s1x = things->scaleX[t1] < 0 ? -things->scaleX[t1] : things->scaleX[t1];
    i32 s1y = things->scaleY[t1] < 0 ? -things->scaleY[t1] : things->scaleY[t1];
    i32 s2x = things->scaleX[t2] < 0 ? -things->scaleX[t2] : things->scaleX[t2];
    i32 s2y = things->scaleY[t2] < 0 ? -things->scaleY[t2] : things->scaleY[t2];

    i16 hx1 = (i16)((things->maskWidth[t1]  * s1x) << 3);
    i16 hy1 = (i16)((things->maskHeight[t1] * s1y) << 3);
    i16 hx2 = (i16)((things->maskWidth[t2]  * s2x) << 3);
    i16 hy2 = (i16)((things->maskHeight[t2] * s2y) << 3);

    i16 dx = things->subX[t1] - things->subX[t2];
    i16 dy = things->subY[t1] - things->subY[t2];
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    return (dx < (hx1 + hx2)) && (dy < (hy1 + hy2));
}

static void drawThingMask(Things *things, u16 id, Color color) {
    float px = TO_FLOAT_16(things->subX[id]);
    float py = TO_FLOAT_16(things->subY[id]);
    float sx = fabsf(TO_FLOAT_8(things->scaleX[id]));
    float sy = fabsf(TO_FLOAT_8(things->scaleY[id]));

    float hx = (float)things->maskWidth[id] * sx * 0.5f;
    float hy = (float)things->maskHeight[id] * sy * 0.5f;

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
            if (state->things.maskWidth[current] != 0 && state->things.maskHeight[current] != 0) {
                drawThingMask(&state->things, current, kind_colors[k]);
            }

            current = state->things.nextSibId[current];
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
        u16 next1 = state->things.nextSibId[currentThing1];

        u16 currentThing2 = head2;
        if (currentThing2 != NIL) {
            do {
                u16 next2 = state->things.nextSibId[currentThing2];

                if (checkAABB(&state->things, currentThing1, currentThing2)) {
                    if (currentThing1 == head1)
                        head1 = state->things.nextSibId[currentThing1];
                    if (currentThing2 == head2)
                        head2 = state->things.nextSibId[currentThing2];

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
            currentThing1 = state->things.nextSibId[currentThing1];
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
