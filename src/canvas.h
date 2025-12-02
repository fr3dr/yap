#ifndef CANVAS_H
#define CANVAS_H

#include <SDL3/SDL.h>
#include "stb_image.h"
#include "stb_image_write.h"
#include "brush.h"
#include "colors.h"

#define MAX_LAYERS 128

typedef struct CanvasLayer {
    SDL_Surface *surface;
    SDL_Renderer *renderer;
    bool visible;
    struct CanvasLayer *next;
    struct CanvasLayer *last;
} CanvasLayer;

typedef struct Canvas {
    SDL_FRect rect;
    float initial_width;
    float initial_height;
    float scale;

    bool update;
    SDL_Texture *texture;
    CanvasLayer layers[MAX_LAYERS];
    int layer_count;
    int active_layer;

    SDL_Color bg_color;
    Color draw_color;
} Canvas;

Canvas CreateCanvas(SDL_Renderer *renderer, SDL_FRect rect, SDL_Color bg_color);
void DestroyCanvas(Canvas *canvas);

void DrawOnCanvas(Canvas *canvas, Brush *brush, int x1, int y1, int x2, int y2);
void EraseFromCanvas(Canvas *canvas, Brush *brush, int x1, int y1, int x2, int y2);
void CanvasUseFillTool(Canvas *canvas, int x, int y);

void RenderCanvas(SDL_Renderer *renderer, Canvas *canvas);
void MoveCanvas(Canvas *canvas, float x, float y);
void ScaleCanvas(Canvas *canvas, float scale, float mouse_x, float mouse_y);
void ClearCanvas(Canvas *canvas);

void NewCanvasLayer(Canvas *canvas);
void RemoveCanvasLayer(Canvas *canvas, int layer_index);
void CanvasLayerUp(Canvas *canvas, int layer_index);
void CanvasLayerDown(Canvas *canvas, int layer_index);

void CanvasWriteImage(Canvas *canvas, const char *filename);
void CanvasLoadImage(Canvas *canvas, const char *filename);

void CanvasGlobalToRelativePosition(Canvas *canvas, int global_x, int global_y, int *rel_x, int *rel_y);
Color CanvasGetPixel(Canvas *canvas, int x, int y);
void CanvasSetPixel(Canvas *canvas, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
Uint32 CanvasGetPixelUint32(Canvas *canvas, int x, int y);

#endif /* end of include guard: CANVAS_H */
