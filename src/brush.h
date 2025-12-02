#ifndef BRUSH_H
#define BRUSH_H

#include <SDL3/SDL.h>
#include "colors.h"
#include "shapes.h"

#define BRUSH_SIZE_MIN 1
#define BRUSH_SIZE_MAX 128

typedef struct Brush {
    int size;
    float variance;
} Brush;

void BrushDrawStroke(Brush *brush, SDL_Renderer *renderer, int x1, int y1, int x2, int y2);

#endif // !BRUSH_H
