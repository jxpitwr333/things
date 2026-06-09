#include "utils.h"
#include <stdlib.h>

Vector2 vec2add(Vector2 v1, Vector2 v2) {
	return (Vector2){v1.x + v2.x, v1.y + v2.y};
}

int irandom_range(int min, int max) {
	return (rand() % (max - min + 1)) + min;
}