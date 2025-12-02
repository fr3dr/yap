#include "shapes.h"

void DrawCircle(SDL_Renderer *renderer, float x, float y, float radius) {
    int segmentCount = 36;
    int pointCount = segmentCount+1;
    float angleAdd = SDL_PI_F*2/segmentCount;
    float angle = 0;
    SDL_FPoint line_points[pointCount];
    SDL_FPoint point;

    for (int i = 0; i < pointCount; i++) {
        point.x = x + radius * SDL_cosf(angle);
        point.y = y + radius * SDL_sinf(angle);
        line_points[i] = point;
        angle += angleAdd;
    }

    SDL_RenderLines(renderer, line_points, pointCount);
}

void DrawFillCircle(SDL_Renderer *renderer, float x, float y, float radius) {
    int segmentAmmount = 36;
    int vertexAmmount = segmentAmmount * 3;
    float angleAdd = SDL_PI_F*2/segmentAmmount;
    float angle = 0;

    SDL_FColor color;
    SDL_GetRenderDrawColorFloat(renderer, &color.r, &color.g, &color.b, &color.a);

    SDL_Vertex vertices[vertexAmmount];
    SDL_zeroa(vertices);
    for (int i = 0; i < vertexAmmount; i++) {
        vertices[i].color = color;
        if (i % 3 == 0) {
            vertices[i].position.x = x;
            vertices[i].position.y = y;
            SDL_RenderPoint(renderer, vertices[i].position.x, vertices[i].position.y);
            continue;
        }
        vertices[i].position.x = x + radius * SDL_cosf(angle);
        vertices[i].position.y = y + radius * SDL_sinf(angle);
        if ((i-1) % 3 == 0) {
            angle += angleAdd;
        }
    }

    SDL_RenderGeometry(renderer, NULL, vertices, vertexAmmount, NULL, 0);
}

void DrawLineThickness(SDL_Renderer *renderer, float x1, float y1, float x2, float y2, float thickness) {
    SDL_Vertex vertices[4];

    float radius = thickness / 2;
    float angle = SDL_atan2(y2 - y1, x2 - x1) - SDL_PI_F / 2;
    vertices[0].position.x = x1 + radius * SDL_cosf(angle);
    vertices[0].position.y = y1 + radius * SDL_sinf(angle);
    vertices[1].position.x = x1 - radius * SDL_cosf(angle);
    vertices[1].position.y = y1 - radius * SDL_sinf(angle);

    vertices[2].position.x = x2 + radius * SDL_cosf(angle);
    vertices[2].position.y = y2 + radius * SDL_sinf(angle);
    vertices[3].position.x = x2 - radius * SDL_cosf(angle);
    vertices[3].position.y = y2 - radius * SDL_sinf(angle);

    int indicies[6] = {0, 1, 2, 1, 2, 3};

    SDL_FColor color;
    SDL_GetRenderDrawColorFloat(renderer, &color.r, &color.g, &color.b, &color.a);
    vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = color;

    DrawFillCircle(renderer, x1, y1, radius);
    DrawFillCircle(renderer, x2, y2, radius);
    SDL_RenderGeometry(renderer, NULL, vertices, 4, indicies, 6);
}

