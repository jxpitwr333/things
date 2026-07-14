#include "platform_sleep.h"
#include "things.h"
#include "game.h"
#include <math.h>
#include <stdio.h>

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

const WaveTemplate WAVE_POOL[] = {
    { .type = FORMATION_V,          .threatCost = 25, .alienType = ALIEN_GREEN },
    { .type = FORMATION_THREE_WALL, .threatCost = 15, .alienType = ALIEN_GREEN },
	{ .type = FORMATION_THREE_WALL, .threatCost = 35, .alienType = ALIEN_RED },
};

const size_t WAVE_POOL_COUNT = sizeof(WAVE_POOL)/sizeof(WAVE_POOL[0]);

static i8 getAlienHealth(AlienType type) {
	switch (type) {
		case ALIEN_GREEN: return 2;
		case ALIEN_RED: return 3;
		default: return 1;
	}
}

static i8 getAlienAnim(AlienType type) {
	switch (type) {
		case ALIEN_GREEN: return ANIM_GREEN;
		case ALIEN_RED: return ANIM_RED;
		default: return 1;
	}
}

static i8 getAlienHitAnim(AlienType type) {
	switch (type) {
		case ALIEN_GREEN: return ANIM_GREEN_FLASH;
		case ALIEN_RED: return ANIM_RED_FLASH;
		default: return 1;
	}
}

void particleUpdate(State *state, u16 id) {
    // alarm[1] is used for lifetime.
    if (PARTICLE_LIFETIME(state, id) == 0) {
        rem(state, id);
        return;
    }

    // for particles, the parentId field is repurposed for the particle type
    // indicator
    Particle template = PARTICLES[PARTICLE_TYPE(state, id)];
    if (template.shrink != 0) {
        state->things.scaleX[id] -= template.shrink;
        state->things.scaleY[id] -= template.shrink;
    }

    if (template.speed != 0) {
        state->things.subX[id] += (COSTABLE[state->things.rotation[id]] * template.speed) >> 7;
        state->things.subY[id] += (SINTABLE[state->things.rotation[id]] * template.speed) >> 7;
    }
}

void particleDraw(State *state, u16 id) {
    Particle template = PARTICLES[PARTICLE_TYPE(state, id)];

    switch (PARTICLE_TYPE(state, id)) {
        case PARTICLE_EXHAUST: {
            const i32 *palette = template.colorPalette;
            float percentage = (float)PARTICLE_LIFETIME(state, id) / (float)template.lifetime;
            int idx = clamp((int)floorf(percentage * template.colorCount), 0, template.colorCount - 1);

            DrawEllipse((int)TO_FLOAT_16(state->things.subX[id]), (int)TO_FLOAT_16(state->things.subY[id]),
                    TO_FLOAT_8(state->things.scaleX[id]), TO_FLOAT_8(state->things.scaleY[id]), hex2Color(palette[idx]));
            break;
        }

        case PARTICLE_EXPLOSION:
            DrawEllipse((int)TO_FLOAT_16(state->things.subX[id]), (int)TO_FLOAT_16(state->things.subY[id]),
                    TO_FLOAT_8(state->things.scaleX[id]), TO_FLOAT_8(state->things.scaleY[id]), hex2Color(ALIEN_COLORS[ALIEN_COLOR(state, id)]));
            break;
    }
}

void createExplosion(State *state, u16 alienId) {
    const int EXPLOSION_PELLETS = 8;
    const float ANGLE_STEP = 360.0f / EXPLOSION_PELLETS;

    for (int i = 0; i < EXPLOSION_PELLETS; ++i) {
        Particle template = PARTICLES[PARTICLE_EXPLOSION];
        float angle = (float)i * ANGLE_STEP;
        u16 p = add(state, (Thing){
                .scaleX = template.scale,
                .scaleY = template.scale,
                .rotation = DEG2BRAD(angle),
                .subX = state->things.subX[alienId],
                .subY = state->things.subY[alienId],
                .kind = PARTICLEKIND,
                .alarms = {[1] = template.lifetime}
                });
        PARTICLE_TYPE(state, p) = PARTICLE_EXPLOSION;
        ALIEN_COLOR(state, p) = ALIEN_COLOR(state, alienId);
    }
}

void alienUpdate(State *state, u16 id) {
    if (ALIEN_ROTATION_TIMER(state, id) <= 0) { ALIEN_ROTATION_TIMER(state, id) = 255; } 
    if (ALIEN_HITFLASH_TIMER(state, id) <= 0) { ANIMATION_ID(state, id) = getAlienAnim(ALIEN_COLOR(state, id)); }
    u8 wave_idx = (u8)(ALIEN_ROTATION_TIMER(state, id) * ALIEN_ROTATION_SPD);
    state->things.rotation[id] = (SINTABLE[wave_idx] * ALIEN_ROTATION_AMPLITUDE) >> 7;
    state->things.subY[id] += TO_FIXED_16(0.25);
}

void bulletUpdate(State *state, u16 id) {
    state->things.subX[id] += (COSTABLE[state->things.rotation[id]] * BULLET_SPD) >> 7;
    state->things.subY[id] += (SINTABLE[state->things.rotation[id]] * BULLET_SPD) >> 7;

    if (state->things.subY[id] <= 0) {
        rem(state, id);
        return;
    }
}

void shipUpdate(State *state, u16 id) {
    int moveX = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
    int moveY = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);

    if (IsKeyDown(KEY_SPACE) && WEAPON_COOLDOWN(state, id) == 0) {
        addScreenshake(state, 4);
        WEAPON_COOLDOWN(state, id) = 7;

        add(state, (Thing){
                .kind = BULLETKIND,
                .subX = state->things.subX[id],
                .subY = state->things.subY[id],
                .rotation = DEG2BRAD(270),
                .scaleX = TO_FIXED_8(1),
                .scaleY = TO_FIXED_8(1),
                .maskWidth = 8,
                .maskHeight = 8,
                });
    }

    if (moveX != 0) {
        state->things.spriteId[id] = 1;
        state->things.scaleX[id] = TO_FIXED_8(moveX);
        state->things.maskWidth[id] = 4;
    } else {
        state->things.spriteId[id] = 0;
        state->things.scaleX[id] = TO_FIXED_8(1);
        state->things.maskWidth[id] = 8;
    }

    if (moveX != 0 || moveY != 0) {
        u16 p = add(state, (Thing){
			.kind = PARTICLEKIND,
			.subX = state->things.subX[id] + TO_FIXED_16(randomRange(-2, 2)),
			.subY = state->things.subY[id] + TO_FIXED_16(randomRange(-2, 2)),
			.scaleX = PARTICLES[PARTICLE_EXHAUST].scale,
			.scaleY = PARTICLES[PARTICLE_EXHAUST].scale,
			.alarms = {[1] = PARTICLES[PARTICLE_EXHAUST].lifetime}
		});
		PARTICLE_TYPE(state, p) = PARTICLE_EXHAUST;
    }

    state->things.subX[id] += TO_FIXED_16(SHIP_SPD * moveX);
    state->things.subY[id] += TO_FIXED_16(SHIP_SPD * moveY);

    state->things.subX[id] =
        TO_FIXED_16(fclamp(TO_FLOAT_16(state->things.subX[id]), (float)HALF_TILE_SIZE,
                    (float)(GAME_WIDTH - HALF_TILE_SIZE)));
    state->things.subY[id] =
        TO_FIXED_16(fclamp(TO_FLOAT_16(state->things.subY[id]), (float)HALF_TILE_SIZE,
                    (float)(GAME_HEIGHT - HALF_TILE_SIZE)));
}

void spawnerUpdate(State *state, Director* director) {
    if (director->phaseTimer > 0) {
        director->phaseTimer--;
        
        if (director->budget < director->maxBudget && (director->phaseTimer % 30 == 0)) {
            director->budget += 5; 
        }
    } else {
        director->phase = (director->phase + 1) % PHASE_COUNT;

        switch (director->phase) {
            case PHASE_BUILDUP:
                director->maxBudget = 40;
                director->phaseTimer = SECONDS(15);
                break;
            case PHASE_CLIMAX:
                director->maxBudget = 90;
                director->phaseTimer = SECONDS(15);
                break;
            case PHASE_RECOVERY:
                director->maxBudget = 0;
                director->phaseTimer = SECONDS(15);
                director->budget = 0;
                break;
            default:
                break;
        }
    }

    if (director->timer > 0) {
        director->timer--;
        return;
    }

    if (director->budget <= 0) {
        director->timer = SECONDS(1);
        return;
    }

    i8 choice = randomRange(0, WAVE_POOL_COUNT);
    WaveTemplate wave = WAVE_POOL[choice];

    if (wave.threatCost <= director->budget) {
        director->budget -= wave.threatCost;
        
        director->timer = SECONDS(randomRange(2, 4));

        Formation chosenFormation = FORMATIONS[wave.type];
        i8 baseTile = randomRange(chosenFormation.min_tile, chosenFormation.max_tile + 1);

        for (i8 i = 0; i < chosenFormation.count; ++i) {
            Vector2_i8 offset = chosenFormation.offsets[i];
            i16 tileX = baseTile + offset.x;
            i16 posY = -TILE_SIZE + (offset.y * TILE_SIZE);
            
            u16 a = add(state, (Thing){
                .kind = ALIENKIND,
                .subX = TO_FIXED_16(tileX * TILE_SIZE + HALF_TILE_SIZE),
                .subY = TO_FIXED_16(posY),
                .maskWidth = TILE_SIZE,
                .maskHeight = TILE_SIZE,
                .scaleX = TO_FIXED_8(1),
                .scaleY = TO_FIXED_8(1),
                .health = getAlienHealth(wave.alienType),
            });
            ALIEN_COLOR(state, a) = wave.alienType;
			ANIMATION_ID(state, a) = getAlienAnim(wave.alienType);
        }
    } else {
        director->timer = 15;
    }
}

void onBulletHitAlien(State *state, u16 bulletId, u16 alienId) {
	printf("entered onBulletHitAlien callback\n");
    rem(state, bulletId);
	state->things.health[alienId] -= 1;
	ANIMATION_ID(state, alienId) = getAlienHitAnim(ALIEN_COLOR(state, alienId));
	ALIEN_HITFLASH_TIMER(state, alienId) = 3;
	if (state->things.health[alienId] <= 0) {
		sys_sleep(2);
		createExplosion(state, alienId);
		addScreenshake(state, 4);
		rem(state, alienId);
	}
}
