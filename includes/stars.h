#ifndef STARS_H
#define STARS_H

#ifdef __APPLE__
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

#include <vector>

// Estrutura para representar uma única estrela
struct Star {
    float x, y, z;
    float brightness;
};

// Funções para gerenciar o fundo de estrelas
void init_stars(int numberOfStars, float radius);
void draw_stars();
void cleanup_stars();

#endif // STARS_H