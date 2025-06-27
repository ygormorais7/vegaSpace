#ifndef STARSHIP_H
#define STARSHIP_H

#include "glm.h"

// Enumeração para a direção do nave, movida para cá por ser um estado do nave.
enum ShipDirection {
    DIR_BACKWARD,
    DIR_FORWARD,
    DIR_LEFT,
    DIR_RIGHT
};

// Estrutura que agrupa todos os dados do nave.
struct Ship {
    GLMmodel* model = nullptr;
    float posx = 0.0, posy = 0.0, posz = 0.0;
    float rotx = 0.0, roty = 0.0, rotz = 0.0;
    ShipDirection direction = DIR_BACKWARD;
};

// --- Protótipos das Funções ---

// Inicializa a nave, carregando o modelo e definindo valores iniciais.
void ship_init(Ship& ship, const char* model_path);

// Desenha a nave na tela.
void ship_draw(const Ship& ship);

// Processa as teclas de movimento (W, A, S, D, etc.).
void ship_handle_key(Ship& ship, unsigned char key);

// Processa as teclas de rotação (setas).
void ship_handle_special_key(Ship& ship, int key);

// Reseta a posição e rotação do nave.
void ship_reset(Ship& ship);

// Libera a memória alocada para o modelo da nave.
void ship_cleanup(Ship& ship);

#endif