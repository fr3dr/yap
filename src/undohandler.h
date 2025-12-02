#ifndef UNDOHANDLER_H
#define UNDOHANDLER_H

#include <SDL3/SDL.h>
#include "canvas.h"

typedef enum UndoType {
    TYPE_DRAW,
    TYPE_LAYER_ADD,
    TYPE_LAYER_REMOVE
} UndoType;

typedef struct UndoState {
    SDL_Surface *surface;
    SDL_Surface *target_surface;
    // SDL_Rect area;
    // Uint32 *delta;
    // SDL_Surface *current;
    // SDL_Surface *previous;
} UndoState;

typedef struct UndoHandler {
    SDL_Renderer *renderer;
    Canvas *canvas;
    UndoState buffer[128];
    SDL_Surface *old_surface;
    int len;
    int pos;
} UndoHandler;

UndoHandler* CreateUndoHandler(SDL_Renderer *renderer, Canvas *canvas);
void FreeUndoHandler(UndoHandler *uh);

void AddUndoState(UndoHandler *uh);

void DoUndo(UndoHandler *uh);
void DoRedo(UndoHandler *uh);

void ClearUndoHandler(UndoHandler *uh);

#endif // !UNDOHANDLER_H
