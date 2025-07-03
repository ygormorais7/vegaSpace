#ifndef MENU_H
#define MENU_H

#include <GL/glut.h>
#include <string>
#include <vector>

extern int score;
extern float sphere_speed_multiplier;
extern int shotNum;

// callback para iniciar jogo
using MenuStartCallback = void(*)();

// define o estado interno do menu
enum MenuState {
    MENU_MAIN,
    MENU_SHOP,
    MENU_NONE
};

/// Inicializa o menu (registra mouse callback interno)
void menu_init();
/// Chama em reshape() para atualizar w/h
void menu_set_size(unsigned int w, unsigned int h);
/// Callback para “JOGAR”
void menu_set_start_callback(MenuStartCallback cb);
/// Retorna true se ainda estamos em alguma tela do menu (main ou loja)
bool menu_active();
/// Desenha a tela de menu atual (main ou loja)
void menu_display();
/// Mouse handler (é chamado pela GLUT)
void menu_mouse(int button,int state,int x,int y);
/// Exibe o menu principal (MAIN)
void menu_show();

#endif // MENU_H