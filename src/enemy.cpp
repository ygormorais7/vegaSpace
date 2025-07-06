#include "../includes/enemy.h"
#include <iostream>

// Carrega o modelo 3D e define valores padrão para o inimigo
void enemy_init(EnemyShip& enemy, const char* model_path) {
    enemy.model = glmReadOBJ((char*)model_path, true);
    if (!enemy.model) {
        std::cerr << "Erro ao carregar modelo do inimigo: " << model_path << std::endl;
        exit(1); // Aborta se o modelo não puder ser carregado
    }

    glmUnitize(enemy.model);
    glmFacetNormals(enemy.model);
    glmVertexNormals(enemy.model, 90.0);

    enemy.scale = 0.8f;  // Um pouco maior que a nave do jogador
    enemy.active = true; // Ativa o inimigo
    enemy.speed = 0.03f;
}

// Libera a memória do modelo quando não for mais necessário
void enemy_cleanup(EnemyShip& enemy) {
    if (enemy.model) {
        glmDelete(enemy.model);
        enemy.model = nullptr;
    }
}

// Desenha a nave inimiga na posição e rotação corretas
void enemy_draw(const EnemyShip& enemy) {
    if (!enemy.active || !enemy.model) return;

    glPushMatrix();
        glTranslatef(enemy.posx, enemy.posy, enemy.posz);
        glRotatef(enemy.rotx, 1.0f, 0.0f, 0.0f);
        glRotatef(enemy.roty, 0.0f, 1.0f, 0.0f);
        glRotatef(enemy.rotz, 0.0f, 0.0f, 1.0f);
        glScalef(enemy.scale, enemy.scale, enemy.scale);

        // Desenha o modelo
        glmDraw(enemy.model, GLM_SMOOTH | GLM_MATERIAL);
    glPopMatrix();
}

// Atualiza o estado do inimigo
void enemy_update(EnemyShip& enemy) {
    if (!enemy.active) return;

    // Move a nave inimiga para frente
    enemy.posz += enemy.speed;

    // Se a nave inimiga passar da câmera, desativa
    if (enemy.posz > 3.0f) {
        enemy.active = false;
    }
}