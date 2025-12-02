#ifndef SHAPES_H
#define SHAPES_H

#include <SDL3/SDL.h>

void DrawCircle(SDL_Renderer *renderer, float x, float y, float radius);
void DrawFillCircle(SDL_Renderer *renderer, float x, float y, float radius);
void DrawLineThickness(SDL_Renderer *renderer, float x1, float y1, float x2, float y2, float thickness);

#endif // !SHAPES_H
