#include "../includes/starship.h"
#include <GL/glut.h> // Necessário para as constantes das teclas especiais (GLUT_KEY_*)
#include <cmath>     // Para fmod

// Constantes de movimento e rotação, agora locais a este módulo.
static const float ROTATION_DEGREES = 15.0;
static const float MOVE_DISPLACEMENT = 0.2;

// Função auxiliar interna para atualizar a direção com base na rotação.
static void ship_update_direction(Ship& ship){
    // 1) normaliza em [0,360)
    float current = fmod(ship.roty, 360.0);
    if (current < 0) current += 360.0;

    // 2) arredonda ao inteiro mais próximo e converte em [0,359]
    int angle = (int)(current + 0.5f) % 360;

    // 3) mapeia direção em intervalos:
    //    BACKWARD: [330°,360) ∪ [0°,30)
    //    LEFT:     [30°,150)
    //    FORWARD:  [150°,210)
    //    RIGHT:    [210°,330)
    if (angle >= 330 || angle < 30) {
        ship.direction = DIR_BACKWARD;
    }
    else if (angle >= 30 && angle < 150) {
        ship.direction = DIR_LEFT;
    }
    else if (angle >= 150 && angle < 210) {
        ship.direction = DIR_FORWARD;
    }
    else if (angle >= 210 && angle < 330) {
        ship.direction = DIR_RIGHT;
    }
    // caso esteja entre 30–150,150–210,210–330 ou em outro intervalo,
    // a direção permanece a última definida se não for exatamente um dos casos acima
}

void ship_init(Ship& ship, const char* model_path){
    ship.model = glmReadOBJ((char*)model_path, true);

    if (!ship.model) {
        fprintf(stderr, "Erro: Falha ao carregar o modelo do jogador de %s\n", model_path);
        exit(1);
    }

    glmUnitize(ship.model);
    glmFacetNormals(ship.model);
    glmVertexNormals(ship.model, 90.0);

    ship_reset(ship);
}

void ship_draw(const Ship& ship){
    if (!ship.model) return;

    glPushMatrix();                                         // Salva o estado atual
    glTranslatef(ship.posx, ship.posy, ship.posz);
    glRotatef(ship.rotx, 1.0, 0.0, 0.0);
    glRotatef(ship.roty, 0.0, 1.0, 0.0);
    glRotatef(ship.rotz, 0.0, 0.0, 1.0);
    glmDraw(ship.model, GLM_SMOOTH | GLM_MATERIAL);         // Desenha o modelo usando as funções da biblioteca glm
                                                            // GLM_SMOOTH usa normais de vértice para sombreamento suave.
                                                            // GLM_MATERIAL aplica as propriedades de material lidas do arquivo .mtl.
    glPopMatrix();                                          // Restaura o estado anterior
}

void ship_handle_key(Ship& ship, unsigned char key){
    switch (key) {
        case 'a': case 'A': ship.posx -= MOVE_DISPLACEMENT; break;
        case 'd': case 'D': ship.posx += MOVE_DISPLACEMENT; break;
        case 's': case 'S': ship.posy -= MOVE_DISPLACEMENT; break;
        case 'w': case 'W': ship.posy += MOVE_DISPLACEMENT; break;
        case 'r': case 'R': ship_reset(ship); break;
    }
}

void ship_handle_special_key(Ship& ship, int key){
    switch (key) {
        case GLUT_KEY_RIGHT: ship.roty += ROTATION_DEGREES; break;
        case GLUT_KEY_LEFT:  ship.roty -= ROTATION_DEGREES; break;
        case GLUT_KEY_UP:    ship.rotx -= ROTATION_DEGREES; break;
        case GLUT_KEY_DOWN:  ship.rotx += ROTATION_DEGREES; break;
    }
    ship_update_direction(ship);
}

void ship_reset(Ship& ship){
    ship.posx = 0.0;
    ship.posy = 0.0;
    ship.posz = -1.0;
    ship.rotx = 0.0;
    ship.roty = 0.0;
    ship.rotz = 0.0;
    ship.direction = DIR_BACKWARD;
}

void ship_cleanup(Ship& ship){
    if (ship.model) {
        glmDelete(ship.model);
        ship.model = nullptr;
    }
}