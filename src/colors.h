#ifndef COLORS_H
#define COLORS_H

#include <SDL3/SDL.h>

typedef struct Color {
    unsigned char r; // READ ONLY red: 0 - 255
    unsigned char g; // READ ONLY green: 0 - 255
    unsigned char b; // READ ONLY blue: 0 - 255
    unsigned char a; // READ ONLY alpha: 0 - 255

    double h; // READ ONLY hue: 0.0 - 359.0
    double s; // READ ONLY saturation: 0.0 - 1.0
    double v; // READ ONLY value: 0.0 - 1.0
} Color;

typedef struct ColorSwatch {
    Color color;
    SDL_FRect rect;
} ColorSwatch;

Color ColorFromRGB(unsigned char r, unsigned char g, unsigned char b);
Color ColorFromHSV(double h, double s, double v);

void SetColorRGB(Color *color, int r, int g, int b);
void SetColorHSV(Color *color, double h, double s, double v);

void ColorToRGB(Color *color);
void ColorToHSV(Color *color);

// void SetColorR(Color *color, unsigned char r);
// void SetColorG(Color *color, unsigned char g);
// void SetColorB(Color *color, unsigned char b);
//
// void SetColorH(Color *color, double h);
// void SetColorS(Color *color, double s);
// void SetColorV(Color *color, double v);

bool CompareColors(Color const *color1, Color const *color2);

#endif // !COLORS_H
