#ifndef ASTEROID_H
#define ASTEROID_H

#include "glm.h"

struct Asteroid {
    GLMmodel* model = nullptr;
    float posx = 0.0f, posy = 0.0f, posz = 0.0f;
    float rotx = 0.0f, roty = 0.0f, rotz = 0.0f;
};

// Inicializa o asteroide (carrega modelo e define posição/rotação)
void asteroid_init(Asteroid& a, const char* model_path,
                   float px, float py, float pz,
                   float rx = 0, float ry = 0, float rz = 0);

// Desenha o asteroide
void asteroid_draw(const Asteroid& a);

// Libera recursos
void asteroid_cleanup(Asteroid& a);

#endif