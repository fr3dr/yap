#include "colors.h"

void HSVToRGB(Color *color, double hue, double saturation, double value) {
    double rgbRange = value * saturation;
    double maxRGB = value;
    double minRGB = value - rgbRange;
    double hPrime = hue / 60.0;
    double slope_up = SDL_fmod(hPrime, 1.0);
    double slope_down = 1.0 - SDL_fmod(hPrime, 1.0);

    if (hPrime >= 0 && hPrime < 1) {
        color->r = maxRGB * 255;
        color->g = (slope_up * rgbRange + minRGB) * 255;
        color->b = minRGB * 255;
    } else if (hPrime >= 1 && hPrime < 2) {
        color->r = (slope_down * rgbRange + minRGB) * 255;
        color->g = maxRGB * 255;
        color->b = minRGB * 255;
    } else if (hPrime >= 2 && hPrime < 3) {
        color->r = minRGB * 255;
        color->g = maxRGB * 255;
        color->b = (slope_up * rgbRange + minRGB) * 255;
    } else if (hPrime >= 3 && hPrime < 4) {
        color->r = minRGB * 255;
        color->g = (slope_down * rgbRange + minRGB) * 255;
        color->b = maxRGB * 255;
    } else if (hPrime >= 4 && hPrime < 5) {
        color->r = (slope_up * rgbRange + minRGB) * 255;
        color->g = minRGB * 255;
        color->b = maxRGB * 255;
    } else if (hPrime >= 5 && hPrime < 6) {
        color->r = maxRGB * 255;
        color->g = minRGB * 255;
        color->b = (slope_down * rgbRange + minRGB) * 255;
    }
}

void RGBToHSV(Color *color, Uint8 r, Uint8 g, Uint8 b) {
    double rd = r / 255.0;
    double gd = g / 255.0;
    double bd = b / 255.0;

    double max = SDL_max(rd, SDL_max(gd, bd));
    double min = SDL_min(rd, SDL_min(gd, bd));
    double range = max - min;

    if (range == 0) {
        color->h = 0;
    } else if (max == rd) {
        color->h = 60 * ((gd - bd) / range);
        if (color->h < 0) {
            color->h += 360.0;
        }
    } else if (max == gd) {
        color->h = 60 * ((bd - rd) / range + 2);
    } else if (max == bd) {
        color->h = 60 * ((rd - gd) / range + 4);
    }

    color->s = max == 0 ? 0 : range / max;
    color->v = max;
}

Color ColorFromRGB(unsigned char r, unsigned char g, unsigned char b) {
    Color color;
    color.a = 255;
    color.r = r;
    color.g = g;
    color.b = b;
    RGBToHSV(&color, r, g, b);
    return color;
}

Color ColorFromHSV(double h, double s, double v) {
    Color color;
    color.a = 255;
    color.h = h;
    color.s = s;
    color.v = v;
    HSVToRGB(&color, h, s, v);
    return color;
}

void SetColorRGB(Color *color, int r, int g, int b) {
    color->r = r <= -1 ? color->r : r;
    color->g = g <= -1 ? color->g : g;
    color->b = b <= -1 ? color->b : b;
    RGBToHSV(color, color->r, color->g, color->b);
}

void SetColorHSV(Color *color, double h, double s, double v) {
    color->h = h <= -1 ? color->h : h;
    color->s = s <= -1 ? color->s : s;
    color->v = v <= -1 ? color->v : v;
    HSVToRGB(color, color->h, color->s, color->v);
}

bool CompareColors(Color const *color1, Color const *color2) {
    return (color1->a == color2->a && color1->r == color2->r && color1->g == color2->g && color1->b == color2->b) ? true : false;
}

// void SetColorR(Color *color, unsigned char r) {
//     color->r = r;
//     ColorHSV hsv = RGBToHSV(color->r, color->g, color->b);
//     color->h = hsv.h;
//     color->h = hsv.s;
//     color->h = hsv.v;
// }
// void SetColorG(Color *color, unsigned char g) {
//     color->g = g;
//     ColorHSV hsv = RGBToHSV(color->r, color->g, color->b);
//     color->h = hsv.h;
//     color->h = hsv.s;
//     color->h = hsv.v;
// }
// void SetColorB(Color *color, unsigned char b) {
//     color->b = b;
//     ColorHSV hsv = RGBToHSV(color->r, color->g, color->b);
//     color->h = hsv.h;
//     color->h = hsv.s;
//     color->h = hsv.v;
// }
//
// void SetColorH(Color *color, double h) {
//     color->h = h;
//     SDL_Color rgb = HSVToRGB(color->h, color->s, color->v);
//     color->r = rgb.r;
//     color->g = rgb.g;
//     color->b = rgb.b;
// }
// void SetColorS(Color *color, double s) {
//     color->s = s;
//     SDL_Color rgb = HSVToRGB(color->h, color->s, color->v);
//     color->r = rgb.r;
//     color->g = rgb.g;
//     color->b = rgb.b;
// }
// void SetColorV(Color *color, double v) {
//     color->v = v;
//     SDL_Color rgb = HSVToRGB(color->h, color->s, color->v);
//     color->r = rgb.r;
//     color->g = rgb.g;
//     color->b = rgb.b;
// }
