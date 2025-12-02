#include "brush.h"

void BrushDrawStroke(Brush *brush, SDL_Renderer *renderer, int x1, int y1, int x2, int y2) {
    int dx = SDL_abs(x2 - x1);
    int dy = SDL_abs(y2 - y1);
    int delta = dx + dy;
    float size = brush->size + delta * brush->variance;

    DrawLineThickness(renderer, x1, y1, x2, y2, size);
}
