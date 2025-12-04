#ifndef APP_H
#define APP_H

#define SDL_DISABLE_OLD_NAMES
#include <SDL3/SDL.h>

#include "canvas.h"
#include "colors.h"
#include "shapes.h"
#include "undohandler.h"

#define SDL_FLAGS SDL_INIT_VIDEO
#define WINDOW_FLAGS SDL_WINDOW_RESIZABLE

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define PALETTE_SIZE 128

typedef enum InputState {
    INPUT_NONE,
    INPUT_DRAWING,
    INPUT_ERASING,
    INPUT_COLOR_PICKER,
    INPUT_COLOR_VALUE_SLIDER,
    INPUT_MOVE_CANVAS,
    INPUT_SCALE_CANVAS
} InputState;

typedef struct App {
    bool is_running;

    SDL_FRect window_rect;
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Cursor *cursor;

    SDL_Color window_bg_color;

    Canvas canvas;
    Brush brush;

    InputState input_state;
    SDL_Event event;
    SDL_FPoint mouse_pos;

    bool draw_brush_preview;
    bool draw_ui;

    UndoHandler* undo_handler;

    // ui things
    SDL_FRect ui_rect;

    SDL_FRect color_picker;
    SDL_Texture *color_picker_texture;
    SDL_FRect hue_slider;
    SDL_FRect color_palette;
    ColorSwatch palette_swatches[PALETTE_SIZE];
    int pallette_len;

    SDL_FRect layer_list_rect;
    SDL_FRect button_layer_add;
    SDL_FRect button_layer_remove;
    SDL_FRect button_layer_up;
    SDL_FRect button_layer_down;
} App;

void AppInit(App *app);
void AppUpdate(App *app);
void AppQuit(App *app);

void SetAppCursor(App *app, SDL_Cursor *cursor);

void DrawUI(App *app);

void UpdateColorPicker(App *app);

#endif // !APP_H
