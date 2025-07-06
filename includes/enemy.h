#ifndef ENEMY_H
#define ENEMY_H

#include "glm.h"

// Estrutura para os dados da nave inimiga
struct EnemyShip {
    GLMmodel* model = nullptr;
    float posx = 0.0f, posy = 0.0f, posz = 0.0f;
    float rotx = 0.0f, roty = 0.0f, rotz = 0.0f;
    float scale = 1.0f;
    float speed = 0.0f;
    bool active = false; // Se o inimigo está ativo no jogo
};

// --- Protótipos das Funções ---

// Inicializa uma nave inimiga, carregando seu modelo
void enemy_init(EnemyShip& enemy, const char* model_path);

// Libera a memória do modelo
void enemy_cleanup(EnemyShip& enemy);

// Desenha a nave inimiga na tela
void enemy_draw(const EnemyShip& enemy);

// Atualiza a lógica da nave
void enemy_update(EnemyShip& enemy);

#endif