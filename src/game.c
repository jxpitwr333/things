#include "platform_sleep.h"
#include "game.h"
#include "things.h"
#include <math.h>

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

const i32 ALIEN_COLORS[] = {
    [ALIEN_GREEN] = GREEN_HEX,
    [ALIEN_ORANGE] = ORANGE_HEX,
    [ALIEN_RED] = RED_HEX
};

const i32 PLACEHOLDER_PALETTE[] = {
    WHITE_HEX
};

const i32 EXHAUST_PALETTE[MAX_COLORS] = {
    GREY_HEX,
    ORANGE_HEX,
    YELLOW_HEX,
    WHITE_HEX,
};

const Particle PARTICLES[] = {
    [PARTICLE_EXHAUST] =
    {
        .colorCount = 4,
        .colorPalette = EXHAUST_PALETTE,
        .lifetime = 16,
        .scale = TO_FIXED_8(2.5),
        .shrink = TO_FIXED_8(0.1),
        .speed = 0,
    },
    [PARTICLE_EXPLOSION] =
    {
        .speed = TO_FIXED_16(2.5),
        .shrink = TO_FIXED_8(0.1),
        .scale = TO_FIXED_8(2.5),
        .colorCount = 1,
        .colorPalette = PLACEHOLDER_PALETTE,
        .lifetime = 16,
    },
};

void particleUpdate(State *state, Thing *t) {
    // alarm[1] is used for lifetime.
    if (t->alarms[1] == 0) {
        rem(state, t->id);
        return;
    }

    // for particles, the parentId field is repurposed for the particle type
    // indicator
    Particle template = PARTICLES[t->parentId];
    if (template.shrink != 0) {
        t->scaleX -= template.shrink;
        t->scaleY -= template.shrink;
    }

    if (template.speed != 0) {
        t->subX += (COSTABLE[t->rotation] * template.speed) >> 7;
        t->subY += (SINTABLE[t->rotation] * template.speed) >> 7;
    }
}

void particleDraw(Thing *t) {
    Particle template = PARTICLES[t->parentId];

    switch (t->parentId) {
        case PARTICLE_EXHAUST: {
            const i32 *palette = template.colorPalette;
            float percentage = (float)t->alarms[1] / (float)template.lifetime;
            int idx = clamp((int)floorf(percentage * template.colorCount), 0, template.colorCount - 1);

            DrawEllipse((int)TO_FLOAT_16(t->subX), (int)TO_FLOAT_16(t->subY),
                    TO_FLOAT_8(t->scaleX), TO_FLOAT_8(t->scaleY), hex2Color(palette[idx]));
            break;
        }

        case PARTICLE_EXPLOSION:
            DrawEllipse((int)TO_FLOAT_16(t->subX), (int)TO_FLOAT_16(t->subY),
                    TO_FLOAT_8(t->scaleX), TO_FLOAT_8(t->scaleY), hex2Color(ALIEN_COLORS[t->firstChildId]));
            break;
    }
}

void createExplosion(State *state, Thing *t) {
    const int EXPLOSION_PELLETS = 8;
    const float ANGLE_STEP = 360.0f / EXPLOSION_PELLETS;

    for (int i = 0; i < EXPLOSION_PELLETS; ++i) {
        Particle template = PARTICLES[PARTICLE_EXPLOSION];
        float angle = (float)i * ANGLE_STEP;
        Thing* p = get(state->things, add(state, (Thing){
                .scaleX = template.scale,
                .scaleY = template.scale,
                .rotation = DEG2BRAD(angle),
                .subX = t->subX,
                .subY = t->subY,
                .kind = PARTICLEKIND,
                .alarms = {[1] = template.lifetime}
                }));
        p->parentId = PARTICLE_EXPLOSION;
        p->firstChildId = t->firstChildId; // DETERMINES ALIEN COLOR
    }
}

void alienUpdate(Thing *t) {
    if (t->alarms[2] <= 0)
        t->alarms[2] = 255;
    u8 wave_idx = (u8)(t->alarms[2] * ALIEN_ROTATION_SPD);
    t->rotation = (SINTABLE[wave_idx] * ALIEN_ROTATION_AMPLITUDE) >> 7;
    t->subY += TO_FIXED_16(0.25);
}

void bulletUpdate(State *state, Thing *t) {
    t->subX += (COSTABLE[t->rotation] * BULLET_SPD) >> 7;
    t->subY += (SINTABLE[t->rotation] * BULLET_SPD) >> 7;

    if (t->subY <= 0) {
        rem(state, t->id);
        return;
    }
}

void shipUpdate(State *state, u16 id) {
    Thing *ship = get(state->things, id);

    int moveX = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
    int moveY = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);

    if (IsKeyDown(KEY_SPACE) && ship->alarms[1] == 0) {
        addScreenshake(state, 4);
        ship->alarms[1] = 7;

        add(state, (Thing){
                .kind = BULLETKIND,
                .subX = ship->subX,
                .subY = ship->subY,
                .rotation = DEG2BRAD(270),
                .scaleX = TO_FIXED_8(1),
                .scaleY = TO_FIXED_8(1),
                .mask = {.width = 8, .height = 8},
                });
    }

    if (moveX != 0) {
        ship->spriteId = 1;
        ship->scaleX = TO_FIXED_8(moveX);
        ship->mask.width = 4;
    } else {
        ship->spriteId = 0;
        ship->scaleX = TO_FIXED_8(1);
        ship->mask.width = 8;
    }

    if (moveX != 0 || moveY != 0) {
        add(state, (Thing){.kind = PARTICLEKIND,
                .parentId = PARTICLE_EXHAUST,
                .subX = ship->subX + TO_FIXED_16(randomRange(-2, 2)),
                .subY = ship->subY + TO_FIXED_16(randomRange(-2, 2)),
                .scaleX = PARTICLES[PARTICLE_EXHAUST].scale,
                .scaleY = PARTICLES[PARTICLE_EXHAUST].scale,
                .alarms = {[1] = PARTICLES[PARTICLE_EXHAUST].lifetime}});
    }

    ship->subX += TO_FIXED_16(SHIP_SPD * moveX);
    ship->subY += TO_FIXED_16(SHIP_SPD * moveY);

    ship->subX =
        TO_FIXED_16(fclamp(TO_FLOAT_16(ship->subX), (float)HALF_TILE_SIZE,
                    (float)(GAME_WIDTH - HALF_TILE_SIZE)));
    ship->subY =
        TO_FIXED_16(fclamp(TO_FLOAT_16(ship->subY), (float)HALF_TILE_SIZE,
                    (float)(GAME_HEIGHT - HALF_TILE_SIZE)));
}

void spawnerUpdate(State *state) {
    state->spawnerCounter++;

    if (state->spawnerCounter >= SECONDS(3)) {
        state->spawnerCounter = 0;

        Formation chosenFormation = FORMATIONS[randomRange(0, FORMATION_COUNT)];
        i8 baseTile =
            randomRange(chosenFormation.min_tile, chosenFormation.max_tile + 1);

        for (i8 i = 0; i < chosenFormation.count; ++i) {
            Vector2_i8 offset = chosenFormation.offsets[i];
            i16 tileX = baseTile + offset.x;
            i16 posY = -TILE_SIZE + (offset.y * TILE_SIZE);
            get(state->things, add(state, (Thing){
                    .kind = ALIENKIND,
                    .subX = TO_FIXED_16(tileX * TILE_SIZE + HALF_TILE_SIZE),
                    .subY = TO_FIXED_16(posY),
                    .mask = {.width = 6, .height = 6},
                    .scaleX = TO_FIXED_8(1),
                    .scaleY = TO_FIXED_8(1),
                    }))->firstChildId = ALIEN_GREEN; // firstChild is used to store the type of alien.
        }
    }
}

void onBulletHitAlien(State *state, u16 bulletId, u16 alienId) {
    createExplosion(state, get(state->things, alienId));
    addScreenshake(state, 4);
    sys_sleep(2);
    rem(state, bulletId);
    rem(state, alienId);
}
