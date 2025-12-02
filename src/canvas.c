#include "canvas.h"

Canvas CreateCanvas(SDL_Renderer *renderer, SDL_FRect rect, SDL_Color bg_color) {
    Canvas canvas;

    canvas.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, rect.w, rect.h);
    if (!canvas.texture) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Error creating canvas texture: %s\n", SDL_GetError());
    }
    SDL_SetTextureBlendMode(canvas.texture, SDL_BLENDMODE_BLEND);

    canvas.active_layer = 1;
    canvas.layer_count = 0;
    canvas.bg_color = bg_color;
    canvas.rect = rect;
    canvas.initial_width = rect.w;
    canvas.initial_height = rect.h;
    canvas.scale = 1.0;

    SDL_zeroa(canvas.layers);
    NewCanvasLayer(&canvas);
    NewCanvasLayer(&canvas);

    SDL_FillSurfaceRect(canvas.layers[0].surface, NULL, SDL_MapSurfaceRGB(canvas.layers[0].surface, bg_color.r, bg_color.g, bg_color.b));
    canvas.update = true;

    return canvas;
}

void DestroyCanvas(Canvas *canvas) {
    for (int i = 0; i < canvas->layer_count; i++) {
        RemoveCanvasLayer(canvas, i);
    }
}

void DrawOnCanvas(Canvas *canvas, Brush *brush, int x1, int y1, int x2, int y2) {
    CanvasLayer *layer = &canvas->layers[canvas->active_layer];
    if (!layer->visible) {
        return;
    }
    SDL_SetRenderDrawColor(layer->renderer, canvas->draw_color.r, canvas->draw_color.g, canvas->draw_color.b, canvas->draw_color.a);
    BrushDrawStroke(brush, layer->renderer, (x1 - canvas->rect.x) / canvas->scale, (y1 - canvas->rect.y) / canvas->scale, (x2 - canvas->rect.x) / canvas->scale, (y2 - canvas->rect.y) / canvas->scale);
    canvas->update = true;
}

void EraseFromCanvas(Canvas *canvas, Brush *brush, int x1, int y1, int x2, int y2) {
    CanvasLayer *layer = &canvas->layers[canvas->active_layer];
    if (!layer->visible) {
        return;
    }
    SDL_SetRenderDrawColor(layer->renderer, 0, 0, 0, 0);
    BrushDrawStroke(brush, layer->renderer, (x1 - canvas->rect.x) / canvas->scale, (y1 - canvas->rect.y) / canvas->scale, (x2 - canvas->rect.x) / canvas->scale, (y2 - canvas->rect.y) / canvas->scale);
    canvas->update = true;
}

void CanvasUseFillTool(Canvas *canvas, int ax, int ay) {
    int x, y;
    CanvasGlobalToRelativePosition(canvas, ax, ay, &x, &y);

    Uint32 target_color = CanvasGetPixelUint32(canvas, x, y);
    Uint32 replacement_color = (canvas->draw_color.r) | (canvas->draw_color.g << 8) | (canvas->draw_color.b << 16) | (canvas->draw_color.a << 24);
    if (target_color == replacement_color) {
        return;
    }
    SDL_Surface *surface = canvas->layers[canvas->active_layer].surface;

    canvas->update = true;

   const int stack_size = 1024;
    struct {
        int x1;
        int x2;
        int y;
        int dy;
    } spans[stack_size];
    SDL_zeroa(spans);

    spans[0].x1 = x;
    spans[0].x2 = x;
    spans[0].y = y;
    spans[0].dy = 1;

    int count = 1;
    int max_count = 0;

    while (count > 0) {
        if (count >= max_count) {
            max_count = count;
            // SDL_Log("max count: %d", max_count);
        }
        if (count > stack_size) {
            SDL_Log("fill stack overflow uh oh");
            break;
        }

        int x1 = spans[count - 1].x1;
        int x2 = spans[count - 1].x2;
        int y = spans[count - 1].y;
        int dy = spans[count - 1].dy;
        count--;

        int x = x1;
        if (CanvasGetPixelUint32(canvas, x, y) == target_color) {
            while (x > 0 && CanvasGetPixelUint32(canvas, x - 1, y) == target_color) {
                CanvasSetPixel(canvas, x - 1, y, canvas->draw_color.r, canvas->draw_color.g, canvas->draw_color.b, canvas->draw_color.a);
                x--;
            }
            if (x < x1) {
                spans[count].x1 = x;
                spans[count].x2 = x1 - 1;
                spans[count].y = y - dy;
                spans[count].dy = -dy;
                count++;
            }
        }
        while (x1 <= x2) {
            while (x1 < surface->w && CanvasGetPixelUint32(canvas, x1, y) == target_color) {
                CanvasSetPixel(canvas, x1, y, canvas->draw_color.r, canvas->draw_color.g, canvas->draw_color.b, canvas->draw_color.a);
                x1++;
            }
            if (x1 > x && y > 0 && y < surface->h - 1) {
                spans[count].x1 = x;
                spans[count].x2 = x1 - 1;
                spans[count].y = y + dy;
                spans[count].dy = dy;
                count++;
            }
            if (x1 - 1 > x2 && y > 0 && y < surface->h - 1) {
                spans[count].x1 = x2 + 1;
                spans[count].x2 = x1 - 1;
                spans[count].y = y - dy;
                spans[count].dy = -dy;
                count++;
            }
            x1++;
            while (x1 < x2 && CanvasGetPixelUint32(canvas, x1, y) != target_color && x1 < surface->w) {
                x1++;
            }
            x = x1;
        }
    }
}

void RenderCanvas(SDL_Renderer *renderer, Canvas *canvas) {
    if (!canvas->update) {
        SDL_RenderTexture(renderer, canvas->texture, NULL, &canvas->rect);
        return;
    }

    SDL_Surface *surface;
    SDL_LockTextureToSurface(canvas->texture, NULL, &surface);

    // draw checkered background
    int square_size = 16;
    for (int x = 0; x <= canvas->texture->w / square_size + 1; x++) {
        for (int y = 0; y <= canvas->texture->h / square_size + 1; y++) {
            SDL_Rect rect = {x * square_size, y * square_size, square_size, square_size};
            if ((y + x) % 2 == 0) {
                SDL_FillSurfaceRect(surface, &rect, SDL_MapSurfaceRGB(surface, 200, 200, 200));
            } else {
                SDL_FillSurfaceRect(surface, &rect, SDL_MapSurfaceRGB(surface, 130, 130, 130));
            }
        }
    }

    // draw layers
    for (int i = 0; i < canvas->layer_count; i++) {
        if (canvas->layers[i].visible) {
            SDL_RenderPresent(canvas->layers[i].renderer);
            SDL_BlitSurface(canvas->layers[i].surface, NULL, surface, NULL);
        }
    }
    SDL_UnlockTexture(canvas->texture);

    SDL_RenderTexture(renderer, canvas->texture, NULL, &canvas->rect);
    canvas->update = false;
}

inline void MoveCanvas(Canvas *canvas, float x, float y) {
    canvas->rect.x = x;
    canvas->rect.y = y;
}

void ScaleCanvas(Canvas *canvas, float scale, float mouse_x, float mouse_y) {
    if (canvas->scale < 2.0 && scale >= 2.0) {
        SDL_SetTextureScaleMode(canvas->texture, SDL_SCALEMODE_NEAREST);
    } else if (canvas->scale >= 2.0 && scale < 2.0) {
        SDL_SetTextureScaleMode(canvas->texture, SDL_SCALEMODE_LINEAR);
    }
    float scale_clamp = SDL_clamp(scale, 0.1, 20.0);
    float rel_x = mouse_x - canvas->rect.x;
    float rel_y = mouse_y - canvas->rect.y;
    float norm_x = rel_x / canvas->rect.w;
    float norm_y = rel_y / canvas->rect.h;
    float delta_w = canvas->rect.w - canvas->initial_width * scale_clamp;
    float delta_h = canvas->rect.h - canvas->initial_height * scale_clamp;
    canvas->rect.x += delta_w * norm_x;
    canvas->rect.y += delta_h * norm_y;
    canvas->scale = scale_clamp;
    canvas->rect.w = canvas->initial_width * canvas->scale;
    canvas->rect.h = canvas->initial_height * canvas->scale;
    SDL_Log("scale: %f", canvas->scale);
}

void ClearCanvas(Canvas *canvas) {
    SDL_FillSurfaceRect(canvas->layers[canvas->active_layer].surface, NULL, 0x00000000);
    canvas->update = true;
}

void NewCanvasLayer(Canvas *canvas) {
    canvas->layers[canvas->layer_count].surface = SDL_CreateSurface(canvas->initial_width, canvas->initial_height, SDL_PIXELFORMAT_RGBA32);
    if (!canvas->layers[canvas->layer_count].surface) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Error creating canvas layer surface: %s\n", SDL_GetError());
    }

    canvas->layers[canvas->layer_count].renderer = SDL_CreateSoftwareRenderer(canvas->layers[canvas->layer_count].surface);
    if (!canvas->layers[canvas->layer_count].renderer) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Error creating canvas layer renderer: %s\n", SDL_GetError());
    }

    canvas->layers[canvas->layer_count].visible = true;

    canvas->layer_count++;
}

void RemoveCanvasLayer(Canvas *canvas, int layer_index) {
    SDL_DestroySurface(canvas->layers[layer_index].surface);
    SDL_DestroyRenderer(canvas->layers[layer_index].renderer);
    canvas->layers[layer_index].surface = NULL;
    canvas->layers[layer_index].renderer= NULL;
    for (int i = layer_index; i < canvas->layer_count - 1; i++) {
        canvas->layers[i].surface = canvas->layers[i + 1].surface;
        canvas->layers[i].renderer = canvas->layers[i + 1].renderer;
        canvas->layers[i + 1].surface = NULL;
        canvas->layers[i + 1].renderer= NULL;
    }

    canvas->layer_count--;
    if (layer_index >= canvas->layer_count) {
        canvas->active_layer = canvas->layer_count - 1;
    }

    canvas->update = true;
}

void CanvasLayerUp(Canvas *canvas, int layer_index) {
    if (layer_index < 1) {
        return;
    }

    CanvasLayer copy = canvas->layers[layer_index];
    canvas->layers[layer_index] = canvas->layers[layer_index - 1];
    canvas->layers[layer_index - 1] = copy;
    canvas->active_layer--;

    canvas->update = true;
}

void CanvasLayerDown(Canvas *canvas, int layer_index) {
    if (1 + layer_index >= canvas->layer_count) {
        return;
    }

    CanvasLayer copy = canvas->layers[layer_index];
    canvas->layers[layer_index] = canvas->layers[layer_index + 1];
    canvas->layers[layer_index + 1] = copy;
    canvas->active_layer++;

    canvas->update = true;
}

void CanvasWriteImage(Canvas *canvas, const char *filename) {
    SDL_Log("saving image...");
    SDL_Surface *surface = SDL_CreateSurface(canvas->texture->w, canvas->texture->h, SDL_PIXELFORMAT_RGBA32);
    for (int i = 0; i < canvas->layer_count; i++) {
        SDL_BlitSurface(canvas->layers[i].surface, NULL, surface, NULL);
    }
    stbi_write_png(filename, surface->w, surface->h, 4, surface->pixels, surface->pitch);
    SDL_Log("saved!");
}

void CanvasLoadImage(Canvas *canvas, const char *filename) {
    int x, y, n;
    int comp = 4;
    stbi_uc *data = stbi_load(filename, &x, &y, &n, comp);
    if (data == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to load image file: %s", filename);
        return;
    }

    SDL_Log("loaded image w: %d\nloaded image h: %d\nloaded image n: %d", x, y, n);

    NewCanvasLayer(canvas);
    SDL_memcpy(canvas->layers[canvas->layer_count - 1].surface->pixels, data, comp * x * y);

    canvas->update = true;

    stbi_image_free(data);
}

Color CanvasGetPixel(Canvas *canvas, int x, int y) {
    SDL_Surface *surface = canvas->layers[canvas->active_layer].surface;
    Uint32 *pixel = (Uint32*)((Uint8*)surface->pixels + x * SDL_BYTESPERPIXEL(surface->format) + y * surface->pitch);
    return ColorFromRGB(*pixel, *pixel >> 8, *pixel >> 16);
}

void CanvasGlobalToRelativePosition(Canvas *canvas, int global_x, int global_y, int *rel_x, int *rel_y) {
    *rel_x = (global_x - canvas->rect.x) / canvas->scale;
    *rel_y = (global_y - canvas->rect.y) / canvas->scale;
}

Uint32 CanvasGetPixelUint32(Canvas *canvas, int x, int y) {
    SDL_Surface *surface = canvas->layers[canvas->active_layer].surface;
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
        SDL_Log("get overflow");
        return -1;
    }
    return *(Uint32*)((Uint8*)surface->pixels + x * SDL_BYTESPERPIXEL(surface->format) + y * surface->pitch);
}

void CanvasSetPixel(Canvas *canvas, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_Surface *surface = canvas->layers[canvas->active_layer].surface;
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
        SDL_Log("set overflow");
        return;
    }
    Uint32 *pixel = (Uint32*)((Uint8*)surface->pixels + x * SDL_BYTESPERPIXEL(surface->format) + y * surface->pitch);
    *pixel = 0x00000000;
    *pixel |= r;
    *pixel |= (Uint32)(g << 8);
    *pixel |= (Uint32)(b << 16);
    *pixel |= (Uint32)(a << 24);
}
