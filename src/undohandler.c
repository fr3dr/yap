#include "undohandler.h"

UndoHandler* CreateUndoHandler(SDL_Renderer *renderer, Canvas *canvas) {
    UndoHandler *uh = SDL_malloc(sizeof(UndoHandler));

    // uh->renderer = renderer;
    uh->canvas = canvas;

    // SDL_SetRenderTarget(renderer, canvas->layers[canvas->active_layer]);
    // uh->old_surface = SDL_RenderReadPixels(renderer, NULL);

    // SDL_zeroa(uh->buffer);
    // uh->len = 1;
    // uh->pos = 0;
    // // first element of buffer is an empty canvas
    // SDL_SetRenderTarget(renderer, canvas->layers[canvas->active_layer]);
    // uh->buffer[0].surface = SDL_RenderReadPixels(renderer, NULL);

    return uh;
}

void FreeUndoHandler(UndoHandler *uh) {
    for (size_t i = 0; i < SDL_arraysize(uh->buffer); i++) {
        SDL_DestroySurface(uh->buffer[i].surface);
        uh->buffer[i].surface = NULL;
    }
    SDL_free(uh);
    uh = NULL;
}

void AddUndoState(UndoHandler *uh) {
    // if (uh->pos < uh->len - 1) {
    //     SDL_Log("test");
    //     for (int i = uh->pos + 1; i < uh->len; i++) {
    //         SDL_DestroySurface(uh->buffer[i].surface);
    //         uh->buffer[i].surface = NULL;
    //     }
    //     // SDL_SetRenderTarget(uh->renderer, uh->canvas->layers[canvas->active_layer]);
    //     // SDL_Surface *temp = SDL_RenderReadPixels(uh->renderer, NULL);
    //     // SDL_UpdateTexture(uh->old_texture, NULL, temp->pixels, temp->pitch);
    //     // SDL_DestroySurface(temp);
    // }

    // uh->pos++;
    // uh->len = uh->pos + 1;

    // SDL_SetRenderTarget(uh->renderer, uh->canvas->layers[uh->canvas->active_layer]);
    // uh->buffer[uh->pos].surface = SDL_RenderReadPixels(uh->renderer, NULL);



    // SDL_SetRenderTarget(uh->renderer, uh->canvas->layers[uh->canvas->active_layer]);
    // SDL_Surface *temp = SDL_RenderReadPixels(uh->renderer, &(SDL_Rect){uh->canvas->rect.x, uh->canvas->rect.y, uh->canvas->rect.w, uh->canvas->rect.h});
    // SDL_UpdateTexture(uh->old_surface, NULL, temp->pixels, temp->pitch);
    // SDL_DestroySurface(temp);
}

void ClearUndoHandler(UndoHandler *uh) {
    // for (int i = 1; i < uh->len; i++) {
    //     SDL_DestroySurface(uh->buffer[i].surface);
    //     uh->buffer[i].surface = NULL;
    // }
    // uh->len = 1;
    // uh->pos = 0;
}

void DoUndo(UndoHandler *uh) {
    // if (uh->pos > 0) {
    //     uh->pos--;
    //     SDL_UpdateTexture(uh->canvas->layers[uh->canvas->active_layer], NULL, uh->buffer[uh->pos].surface->pixels, uh->buffer[uh->pos].surface->pitch);

        // uh->len++;
        // uh->buffer[uh->len-1] = *&uh->buffer[uh->pos];
    // }
}

void DoRedo(UndoHandler *uh) {
    // if (uh->pos < uh->len-1) {
    //     uh->pos++;
    //     SDL_UpdateTexture(uh->canvas->layers[uh->canvas->active_layer], NULL, uh->buffer[uh->pos].surface->pixels, uh->buffer[uh->pos].surface->pitch);

        // uh->len--;
        // SDL_DestroySurface(uh->buffer[uh->len].previous);
        // SDL_DestroySurface(uh->buffer[uh->len].current);
        // uh->buffer[uh->len].previous = NULL;
        // uh->buffer[uh->len].current = NULL;
    // }
}
