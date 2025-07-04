#ifndef SPHERE_H
#define SPHERE_H

#include <GL/glut.h>
#include "starship.h"

struct Esfera {
    bool   active   = false;
    float  x, y, z;               // Posição X,Y,Z da esfera
    float  dirx, diry, dirz;      // Direção X,Y,Z do movimento
    float  speed = 0.07f;      // Velocidade da esfera
};

// Inicializa apenas o estado inativo
void sphere_init(Esfera& e, float speed = 0.07f);

// Dispara a esfera a partir da nave
void sphere_fire(Esfera& e, const Ship& ship, float start_dist = 0.4f);

// Atualiza a posição; desativa se sair de limites
void sphere_update(Esfera& e, float limit = 20.0f);

// Desenha a esfera (somente se e.active == true)
void sphere_draw(const Esfera& e);

#endif // SPHERE_H