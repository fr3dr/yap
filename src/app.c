#include "app.h"

void AppInit(App *app) {
    if (!SDL_Init(SDL_FLAGS)) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Error initializing SDL: %s\n", SDL_GetError());
        return;
    }
    if (!SDL_CreateWindowAndRenderer("yap", WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS, &app->window, &app->renderer)) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Error creating window/renderer: %s\n", SDL_GetError());
        return;
    }
    if (!SDL_SetRenderVSync(app->renderer, 1)) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Error enabling VSync: %s\n", SDL_GetError());
        return;
    }

    app->window_rect = (SDL_FRect){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

    app->cursor = SDL_GetDefaultCursor();

    app->is_running = true;

    app->brush.size = 5;
    app->brush.variance = 1;
    app->canvas = CreateCanvas(app->renderer, (SDL_FRect){0, 0, 1024, 1024}, (SDL_Color){0, 0, 0, 255});
    app->canvas.draw_color = ColorFromRGB(255, 255, 255);

    app->draw_brush_preview = true;

    app->window_bg_color = (SDL_Color){50, 50, 50, 255};

    app->undo_handler = CreateUndoHandler(app->renderer, &app->canvas);

    // get display mode
    const SDL_DisplayMode *display = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(app->window));
    if (!display) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Error getting display mode: %s\n", SDL_GetError());
        return;
    }

    app->ui_rect = (SDL_FRect){0, 0, 500, app->window_rect.h};

    app->color_picker = (SDL_FRect){20, 20, 360, 360};
    app->color_picker_texture = SDL_CreateTexture(app->renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, app->color_picker.w, app->color_picker.h);
    if (!app->color_picker_texture) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Error creating color picker texture: %s\n", SDL_GetError());
        return;
    }
    app->hue_slider = (SDL_FRect){app->color_picker.x + app->color_picker.w, app->color_picker.y, 30, app->color_picker.h};

    app->color_palette = (SDL_FRect){app->color_picker.x, app->color_picker.y + app->color_picker.h + 20, app->color_picker.w, 100};
    app->pallette_len = 0;
    SDL_zeroa(app->palette_swatches);

    // define layer list ui
    app->layer_list_rect = (SDL_FRect){20, 500, 200, 500};
    app->button_layer_add = (SDL_FRect){app->layer_list_rect.x, app->layer_list_rect.y, 15, 15};
    app->button_layer_remove = (SDL_FRect){app->button_layer_add.x + app->button_layer_add.w + 5, app->layer_list_rect.y, 15, 15};
    app->button_layer_up = (SDL_FRect){app->button_layer_remove.x + app->button_layer_remove.w + 5, app->layer_list_rect.y, 15, 15};
    app->button_layer_down = (SDL_FRect){app->button_layer_up.x + app->button_layer_up.w + 5, app->layer_list_rect.y, 15, 15};

    UpdateColorPicker(app);
}

void AppUpdate(App *app) {
    // SDL_Log("r: %d\ng: %d\nb: %d\na: %d", app->canvas.draw_color_rgb.r, app->canvas.draw_color_rgb.g, app->canvas.draw_color_rgb.b, app->canvas.draw_color_rgb.a);
    // SDL_Log("h: %f\ns: %f\nv: %f", app->canvas.draw_color_hsv.h, app->canvas.draw_color_hsv.s, app->canvas.draw_color_hsv.v);
    // SDL_Log("undo_pos: %d, undo_len: %d", app->undo_handler->pos, app->undo_handler->len);
    // SDL_Log("input state: %d", app->input_state);
    while (SDL_PollEvent(&app->event)) {
        switch (app->event.type) {
            case SDL_EVENT_QUIT:
                app->is_running = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                if (app->input_state == INPUT_DRAWING) {
                    DrawOnCanvas(&app->canvas, &app->brush, app->event.motion.x, app->event.motion.y, app->mouse_pos.x, app->mouse_pos.y);
                } else if (app->input_state == INPUT_ERASING) {
                    EraseFromCanvas(&app->canvas, &app->brush, app->event.motion.x, app->event.motion.y, app->mouse_pos.x, app->mouse_pos.y);
                } else if (app->input_state == INPUT_COLOR_PICKER) {
                    SetColorHSV(&app->canvas.draw_color, -1, SDL_clamp((app->event.motion.x - app->color_picker.x) / app->color_picker.w, 0.0, 1.0), 1.0 - SDL_clamp((app->event.motion.y - app->color_picker.y) / app->color_picker.h, 0.0, 1.0));
                } else if (app->input_state == INPUT_COLOR_VALUE_SLIDER) {
                    SetColorHSV(&app->canvas.draw_color, SDL_clamp((app->event.motion.y - app->hue_slider.y) / app->hue_slider.h * 359.0, 0.0, 359.0), -1, -1);
                    UpdateColorPicker(app);
                } else if (app->input_state == INPUT_MOVE_CANVAS) {
                    MoveCanvas(&app->canvas, app->canvas.rect.x + app->event.motion.xrel, app->canvas.rect.y + app->event.motion.yrel);
                }
                app->mouse_pos.x = app->event.motion.x;
                app->mouse_pos.y = app->event.motion.y;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                app->mouse_pos.x = app->event.button.x;
                app->mouse_pos.y = app->event.button.y;
                switch (app->event.button.button) {
                    case SDL_BUTTON_LEFT:
                        if ((!app->draw_ui || (app->draw_ui && !SDL_PointInRectFloat(&app->mouse_pos, &app->ui_rect))) && app->input_state == INPUT_NONE && SDL_PointInRectFloat(&app->mouse_pos, &app->canvas.rect)) {
                            app->input_state = INPUT_DRAWING;
                            DrawOnCanvas(&app->canvas, &app->brush, app->event.button.x, app->event.button.y, app->event.button.x, app->event.button.y);
                        } else if (app->draw_ui) {
                            if (SDL_PointInRectFloat(&app->mouse_pos, &app->color_picker)) {
                                app->input_state = INPUT_COLOR_PICKER;
                                SetColorHSV(&app->canvas.draw_color, -1, (app->event.button.x - app->color_picker.x) / app->color_picker.w, 1.0 - ((app->event.button.y - app->color_picker.y) / app->color_picker.h));
                            }
                            if (SDL_PointInRectFloat(&app->mouse_pos, &app->hue_slider)) {
                                app->input_state = INPUT_COLOR_VALUE_SLIDER;
                                SetColorHSV(&app->canvas.draw_color, (app->event.motion.y - app->hue_slider.y) / app->hue_slider.h * 359.0, -1, -1);
                                UpdateColorPicker(app);
                            }
                            for (int i = 0; i < app->pallette_len; i++) {
                                if (SDL_PointInRectFloat(&app->mouse_pos, &app->palette_swatches[i].rect)) {
                                    app->canvas.draw_color = app->palette_swatches[i].color;
                                    UpdateColorPicker(app);
                                    break;
                                }
                            }
                            for (int i = 0; i < app->canvas.layer_count; i++) {
                                SDL_FRect rect = {app->layer_list_rect.x, 20 + app->layer_list_rect.y + (5 + app->canvas.layers[i].surface->h / 20.0) * i, app->layer_list_rect.w, app->canvas.layers[i].surface->h / 20.0};
                                SDL_FRect vis_rect = {rect.x + rect.w - 15, 20 + app->layer_list_rect.y + (5 + app->canvas.layers[i].surface->h / 20.0) * i, 15, 15};
                                if (SDL_PointInRectFloat(&app->mouse_pos, &vis_rect)) {
                                    app->canvas.layers[i].visible = !app->canvas.layers[i].visible;
                                    app->canvas.update = true;
                                    break;
                                }
                                if (SDL_PointInRectFloat(&app->mouse_pos, &rect)) {
                                    app->canvas.active_layer = i;
                                    break;
                                }
                            }
                            if (SDL_PointInRectFloat(&app->mouse_pos, &app->button_layer_add)) {
                                NewCanvasLayer(&app->canvas);
                            }
                            if (SDL_PointInRectFloat(&app->mouse_pos, &app->button_layer_remove)) {
                                RemoveCanvasLayer(&app->canvas, app->canvas.active_layer);
                            }
                            if (SDL_PointInRectFloat(&app->mouse_pos, &app->button_layer_up)) {
                                CanvasLayerUp(&app->canvas, app->canvas.active_layer);
                            }
                            if (SDL_PointInRectFloat(&app->mouse_pos, &app->button_layer_down)) {
                                CanvasLayerDown(&app->canvas, app->canvas.active_layer);
                            }
                        }
                        break;
                    case SDL_BUTTON_RIGHT:
                        if (!app->draw_ui && app->input_state == INPUT_NONE) {
                            app->input_state = INPUT_ERASING;
                            EraseFromCanvas(&app->canvas, &app->brush, app->event.button.x, app->event.button.y, app->event.button.x, app->event.button.y);
                        }
                        break;
                    case SDL_BUTTON_MIDDLE:
                        if (SDL_PointInRectFloat(&app->mouse_pos, &app->canvas.rect)) {
                            CanvasUseFillTool(&app->canvas, app->mouse_pos.x, app->mouse_pos.y);
                        }
                        break;
                    default:
                        break;
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (app->input_state == INPUT_DRAWING || app->input_state == INPUT_ERASING) {
                    AddUndoState(app->undo_handler);
                }
                app->input_state = INPUT_NONE;
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                if (app->input_state == INPUT_NONE && app->brush.size <= BRUSH_SIZE_MAX && app->brush.size >= BRUSH_SIZE_MIN) {
                    app->brush.size += app->event.wheel.y * 3;
                    app->brush.size = SDL_clamp(app->brush.size, BRUSH_SIZE_MIN, BRUSH_SIZE_MAX);
                    SDL_Log("brush size: %d", app->brush.size);
                }
                if (app->input_state == INPUT_SCALE_CANVAS) {
                    ScaleCanvas(&app->canvas, app->canvas.scale + app->event.wheel.y * app->canvas.scale / 10, app->event.wheel.mouse_x, app->event.wheel.mouse_y);
                }
                break;
            case SDL_EVENT_KEY_DOWN:
                switch (app->event.key.scancode) {
                    case SDL_SCANCODE_LCTRL:
                        app->input_state = INPUT_SCALE_CANVAS;
                        app->draw_brush_preview = false;
                        SetAppCursor(app, SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE));
                        break;
                    case SDL_SCANCODE_SPACE:
                        app->input_state = INPUT_MOVE_CANVAS;
                        app->draw_brush_preview = false;
                        SetAppCursor(app, SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE));
                        break;
                    case SDL_SCANCODE_B:
                        app->draw_ui = !app->draw_ui;
                        if (!app->draw_ui) {
                            // add color to color palette
                            bool exists = false;
                            for (int i = 0; i < app->pallette_len; i++) {
                                if (CompareColors(&app->palette_swatches[i].color, &app->canvas.draw_color)) {
                                    exists = true;
                                    break;
                                }
                            }
                            if (!exists) {
                                app->palette_swatches[app->pallette_len].color = app->canvas.draw_color;
                                app->palette_swatches[app->pallette_len].rect = (SDL_FRect){app->color_palette.x + app->pallette_len * 30, app->color_palette.y, 30, 30};
                                app->pallette_len++;
                            }
                        }
                        break;
                    case SDL_SCANCODE_C:
                        ClearCanvas(&app->canvas);
                        AddUndoState(app->undo_handler);
                        break;
                    case SDL_SCANCODE_L:
                        if (app->event.key.mod & SDL_KMOD_CTRL) {
                            CanvasLoadImage(&app->canvas, "save.png");
                            AddUndoState(app->undo_handler);
                        }
                        break;
                    case SDL_SCANCODE_S:
                        if (app->event.key.mod & SDL_KMOD_CTRL) {
                            CanvasWriteImage(&app->canvas, "save.png");
                        }
                        break;
                    case SDL_SCANCODE_Z:
                        if (app->event.key.mod & SDL_KMOD_CTRL && app->event.key.mod & SDL_KMOD_SHIFT) {
                            DoRedo(app->undo_handler);
                        } else if (app->event.key.mod & SDL_KMOD_CTRL) {
                            DoUndo(app->undo_handler);
                        }
                        break;
                    default:
                        break;
                }
                break;
            case SDL_EVENT_KEY_UP:
                if ((app->input_state == INPUT_SCALE_CANVAS && app->event.key.scancode == SDL_SCANCODE_LCTRL) ||
                    (app->input_state == INPUT_MOVE_CANVAS && app->event.key.scancode == SDL_SCANCODE_SPACE)) {
                    app->input_state = INPUT_NONE;
                    app->draw_brush_preview = true;
                    SetAppCursor(app, SDL_GetDefaultCursor());
                }
                break;
            case SDL_EVENT_WINDOW_MOUSE_ENTER:
                app->draw_brush_preview = true;
                break;
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                app->draw_brush_preview = false;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                app->input_state = INPUT_NONE;
                int w, h;
                SDL_GetWindowSizeInPixels(app->window, &w, &h);
                app->window_rect.w = w;
                app->window_rect.h = h;
                break;
            default:
                break;
        }
    }

    // render everything
    SDL_SetRenderTarget(app->renderer, NULL);
    SDL_SetRenderDrawColor(app->renderer, app->window_bg_color.r, app->window_bg_color.g, app->window_bg_color.b, app->window_bg_color.a);
    SDL_RenderClear(app->renderer);
    RenderCanvas(app->renderer, &app->canvas);

    SDL_SetRenderDrawColor(app->renderer, app->canvas.draw_color.r, app->canvas.draw_color.g, app->canvas.draw_color.b, 255);
    DrawCircle(app->renderer, app->mouse_pos.x, app->mouse_pos.y, app->canvas.scale * app->brush.size / 2.0);

    if (app->draw_ui) {
        DrawUI(app);
    }
    SDL_RenderPresent(app->renderer);
}

void AppQuit(App *app) {
    FreeUndoHandler(app->undo_handler);
    DestroyCanvas(&app->canvas);

    if (app->cursor) {
        SDL_DestroyCursor(app->cursor);
    }

    if (app->color_picker_texture) {
        SDL_DestroyTexture(app->color_picker_texture);
    }
    if (app->renderer) {
        SDL_DestroyRenderer(app->renderer);
    }
    if (app->window) {
        SDL_DestroyWindow(app->window);
    }

    SDL_Quit();
}

void SetAppCursor(App *app, SDL_Cursor *cursor) {
    SDL_DestroyCursor(app->cursor);
    app->cursor = cursor;
    SDL_SetCursor(app->cursor);
}

void UpdateColorPicker(App *app) {
    Color orig = ColorFromHSV(app->canvas.draw_color.h, 1.0, 1.0);
    SDL_Color buf;
    SDL_Surface *surface;
    SDL_LockTextureToSurface(app->color_picker_texture, NULL, &surface);
    for (int x = 0; x < app->color_picker.w; x++) { // saturation
        buf.r = orig.r - (orig.r - 255) * ((app->color_picker.w - x) / app->color_picker.w);
        buf.g = orig.g - (orig.g - 255) * ((app->color_picker.w - x) / app->color_picker.w);
        buf.b = orig.b - (orig.b - 255) * ((app->color_picker.w - x) / app->color_picker.w);
        for (int y = 0; y < app->color_picker.h; y++) { // value
            Uint32 c = 0xff000000;
            c |= (Uint32)(buf.r * ((app->color_picker.h - y) / app->color_picker.h));
            c |= (Uint32)(buf.g * ((app->color_picker.h - y) / app->color_picker.h)) << 8;
            c |= (Uint32)(buf.b * ((app->color_picker.h - y) / app->color_picker.h)) << 16;

            *(Uint32*)((Uint8*)surface->pixels + x * SDL_BYTESPERPIXEL(surface->format) + y * surface->pitch) = c;
        }
    }
    SDL_UnlockTexture(app->color_picker_texture);
}

void DrawUI(App *app) {
    SDL_SetRenderDrawColor(app->renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(app->renderer, &app->ui_rect);

    // display color values
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
    SDL_RenderDebugTextFormat(app->renderer, app->hue_slider.x + app->hue_slider.w + 20, app->hue_slider.y + 20, "h: %.0f", app->canvas.draw_color.h);
    SDL_RenderDebugTextFormat(app->renderer, app->hue_slider.x + app->hue_slider.w + 20, app->hue_slider.y + 40, "s: %.0f", app->canvas.draw_color.s * 100.0);
    SDL_RenderDebugTextFormat(app->renderer, app->hue_slider.x + app->hue_slider.w + 20, app->hue_slider.y + 60, "v: %.0f", app->canvas.draw_color.v * 100.0);
    SDL_RenderDebugTextFormat(app->renderer, app->hue_slider.x + app->hue_slider.w + 20, app->hue_slider.y + 100, "r: %d", app->canvas.draw_color.r);
    SDL_RenderDebugTextFormat(app->renderer, app->hue_slider.x + app->hue_slider.w + 20, app->hue_slider.y + 120, "g: %d", app->canvas.draw_color.g);
    SDL_RenderDebugTextFormat(app->renderer, app->hue_slider.x + app->hue_slider.w + 20, app->hue_slider.y + 140, "b: %d", app->canvas.draw_color.b);
    SDL_RenderDebugTextFormat(app->renderer, app->hue_slider.x + app->hue_slider.w + 20, app->hue_slider.y + 160, "a: %d", app->canvas.draw_color.a);

    // render color picker
    SDL_RenderTexture(app->renderer, app->color_picker_texture, NULL, &app->color_picker);

    // draw hue slider
    Color color;
    for (int y = 0; y < app->hue_slider.h; y++) {
        double hue = ((double)y / app->hue_slider.h) * 359.0;
        color = ColorFromHSV(hue, 1.0, 1.0);
        SDL_SetRenderDrawColor(app->renderer, color.r, color.g, color.b, 255);
        SDL_RenderLine(app->renderer, app->hue_slider.x, y + app->hue_slider.y, app->hue_slider.x + app->hue_slider.w - 1, y + app->hue_slider.y);
    }

    // draw selected hue line
    SDL_FRect rect = {app->hue_slider.x, app->hue_slider.y + (app->canvas.draw_color.h / (app->hue_slider.h)) * (app->hue_slider.h - 1.0), app->hue_slider.w, 2};
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(app->renderer, &rect);

    // draw palette
    for (int i = 0; i < app->pallette_len; i++) {
        SDL_SetRenderDrawColor(app->renderer, app->palette_swatches[i].color.r, app->palette_swatches[i].color.g, app->palette_swatches[i].color.b, app->palette_swatches[i].color.a);
        SDL_RenderFillRect(app->renderer, &app->palette_swatches[i].rect);
        // draw selected outline
        if (CompareColors(&app->palette_swatches[i].color, &app->canvas.draw_color)) {
            if (app->palette_swatches[i].color.v < 0.5) {
                SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
            }
            SDL_RenderRect(app->renderer, &app->palette_swatches[i].rect);
        }
    }

    // draw picker dot
    int radius = 5;
    SDL_SetRenderDrawColor(app->renderer, app->canvas.draw_color.r, app->canvas.draw_color.g, app->canvas.draw_color.b, 255);
    DrawFillCircle(app->renderer, app->color_picker.x + app->canvas.draw_color.s * app->color_picker.w, app->color_picker.y + (1.0 - app->canvas.draw_color.v) * app->color_picker.h, radius);
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
    DrawCircle(app->renderer, app->color_picker.x + app->canvas.draw_color.s * app->color_picker.w, app->color_picker.y + (1.0 - app->canvas.draw_color.v) * app->color_picker.h, radius);

    // draw layer list
    SDL_SetRenderDrawColor(app->renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(app->renderer, &app->button_layer_add);
    SDL_RenderFillRect(app->renderer, &app->button_layer_remove);
    SDL_RenderFillRect(app->renderer, &app->button_layer_up);
    SDL_RenderFillRect(app->renderer, &app->button_layer_down);
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(app->renderer, app->button_layer_add.x + 4, app->button_layer_add.y + 4, "+");
    SDL_RenderDebugText(app->renderer, app->button_layer_remove.x + 4, app->button_layer_remove.y + 4, "-");
    SDL_RenderDebugText(app->renderer, app->button_layer_up.x + 4, app->button_layer_up.y + 4, "<");
    SDL_RenderDebugText(app->renderer, app->button_layer_down.x + 4, app->button_layer_down.y + 4, ">");

    SDL_Texture *texture;
    for (int i = 0; i < app->canvas.layer_count; i++) {
        SDL_FRect rect = {app->layer_list_rect.x, 20 + app->layer_list_rect.y + (5 + app->canvas.layers[i].surface->h / 20.0) * i, app->layer_list_rect.w, app->canvas.layers[i].surface->h / 20.0};
        SDL_FRect texture_rect = {app->layer_list_rect.x, 20 + app->layer_list_rect.y + (5 + app->canvas.layers[i].surface->h / 20.0) * i, app->canvas.layers[i].surface->w / 20.0, app->canvas.layers[i].surface->h / 20.0};
        SDL_FRect vis_rect = {rect.x + rect.w - 15, 20 + app->layer_list_rect.y + (5 + app->canvas.layers[i].surface->h / 20.0) * i, 15, 15};

        if (i == app->canvas.active_layer) {
            SDL_SetRenderDrawColor(app->renderer, 130, 130, 130, 255);
        } else {
            SDL_SetRenderDrawColor(app->renderer, 100, 100, 100, 255);
        }
        SDL_RenderFillRect(app->renderer, &rect);

        texture = SDL_CreateTextureFromSurface(app->renderer, app->canvas.layers[i].surface);
        SDL_RenderTexture(app->renderer, texture, NULL, &texture_rect);
        SDL_DestroyTexture(texture);

        SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
        SDL_RenderDebugTextFormat(app->renderer, texture_rect.x + texture_rect.w + 20, texture_rect.y + texture_rect.h / 2, "layer %d", i);
        if (app->canvas.layers[i].visible) {
            SDL_SetRenderDrawColor(app->renderer, 0, 200, 0, 255);
        } else {
            SDL_SetRenderDrawColor(app->renderer, 200, 0, 0, 255);
        }
        SDL_RenderFillRect(app->renderer, &vis_rect);
    }
}
