#include "../includes/stars.h"
#include <vector>
#include <random>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Variáveis globais para as estrelas ---
static std::vector<Star> stars;
static bool stars_initialized = false;

// Motor de aleatorização
static std::mt19937 rng{ std::random_device{}() };

void init_stars(int numberOfStars, float radius) {
    if (stars_initialized) {
        cleanup_stars();
    }

    stars.reserve(numberOfStars);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int i = 0; i < numberOfStars; ++i) {
        // Gera pontos uniformemente distribuídos em uma esfera usando coordenadas esféricas
        float u = dist(rng); // para longitude
        float v = dist(rng); // para latitude

        float theta = 2.0f * M_PI * u;       // Longitude (0 a 2*PI)
        float phi = acos(2.0f * v - 1.0f); // Latitude (0 a PI)

        Star s;
        s.x = radius * sin(phi) * cos(theta);
        s.y = radius * sin(phi) * sin(theta);
        s.z = radius * cos(phi);

        // Define um brilho aleatório para variar a aparência
        s.brightness = 0.5f + dist(rng) * 0.5f;

        stars.push_back(s);
    }
    stars_initialized = true;
}

void draw_stars() {
    if (!stars_initialized) return;

    // Desabilitar iluminação para as estrelas terem a cor exata que definimos
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glPointSize(1.5f); // Tamanho dos pontos
    glBegin(GL_POINTS);
    for (const auto& star : stars) {
        // Define a cor da estrela baseada no seu brilho
        glColor3f(star.brightness, star.brightness, star.brightness);
        glVertex3f(star.x, star.y, star.z);
    }
    glEnd();

    // Restaura o estado anterior do OpenGL
    glPopAttrib();
}

void cleanup_stars() {
    stars.clear();
    stars.shrink_to_fit(); // Libera a memória do vector
    stars_initialized = false;
}