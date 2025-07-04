#include "../includes/asteroid.h"
#include <random>
#include <cstdio>
#include <cstdlib>

void asteroid_init(Asteroid& a, const char* model_path, float px, float py, float pz, float scale, float rx, float ry, float rz){
    a.model = glmReadOBJ((char*)model_path, true);
    if (!a.model) {
        fprintf(stderr, "Erro: não pôde carregar %s\n", model_path);
        exit(1);
    }

    glmUnitize(a.model);
    glmFacetNormals(a.model);
    glmVertexNormals(a.model, 90.0);
    a.posx = px; a.posy = py; a.posz = pz;
    a.rotx = rx; a.roty = ry; a.rotz = rz;
    a.scale = scale;
}

void asteroid_draw(const Asteroid& a){
    if (!a.model) return;
    glPushMatrix();
      glTranslatef(a.posx, a.posy, a.posz);
      glRotatef(a.rotx, 1,0,0);
      glRotatef(a.roty, 0,1,0);
      glRotatef(a.rotz, 0,0,1);
      glScalef(a.scale, a.scale, a.scale);
      glmDraw(a.model, GLM_SMOOTH);
    glPopMatrix();
}

static std::mt19937                    rng{ std::random_device{}() };
static std::uniform_real_distribution<float> distX(-10.0, 10.0);
static std::uniform_real_distribution<float> distY(-5.0, 5.0);
static std::uniform_real_distribution<float> distSpeed(0.9, 1.5);

void asteroid_respawn(Asteroid& a, float back_z) {
    a.posz = back_z;
    a.posx = distX(rng);
    a.posy = distY(rng);
}

void asteroid_update(Asteroid& a, float speed){
    a.posz += distSpeed(rng)*(speed);

    // quando passar de z = 1, “sumir” e reaparecer em z = –15
    if (a.posz > 1.0) {
        asteroid_respawn(a);
    }
}

void asteroid_cleanup(Asteroid& a){
    if (a.model) {
        glmDelete(a.model);
        a.model = nullptr;
    }
}