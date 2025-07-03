#include "menu.h"
#include <GL/gl.h>

// estado interno
static MenuState  menu_state = MENU_MAIN;
static unsigned   menu_w = 800, menu_h = 600;
static MenuStartCallback start_cb = nullptr;

// itens da loja
struct ShopItem { std::string name; int cost; };
static std::vector<ShopItem> shop_items = {
    //{"+1 Vida", 10},
    {"+10% Velocidade de disparo", 10},
    {"+1 disparo", 10}
};

static void drawBitmapText(const char* text, int x, int y) {
    glRasterPos2i(x, y);
    for (const char* c = text; *c; ++c)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}

void menu_init() {
    glutMouseFunc(menu_mouse);
}

void menu_set_size(unsigned w, unsigned h) {
    menu_w = w; menu_h = h;
}

void menu_set_start_callback(MenuStartCallback cb) {
    start_cb = cb;
}

bool menu_active() {
    return menu_state != MENU_NONE;
}

void menu_display() {
    // desliga iluminação (e mantêm o depth-test/desenho 2D como já está)
    glDisable(GL_LIGHTING);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, menu_w, 0, menu_h);
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();

    // 1) Main Menu
    if (menu_state == MENU_MAIN) {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        // ortho 2D
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        gluOrtho2D(0, menu_w, 0, menu_h);
        glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();

        int bw=140, bh=50;          // button width e button height
        int bx = menu_w/2 - bw/2;
        int by = menu_h/2 + 30;

        // botão JOGAR
        glColor3f(0.2f,0.2f,0.8f);
        glBegin(GL_QUADS);
          glVertex2i(bx, by);
          glVertex2i(bx+bw, by);
          glVertex2i(bx+bw, by+bh);
          glVertex2i(bx, by+bh);
        glEnd();
        glColor3f(1,1,1);
        // String por cima do botão
        glDisable(GL_DEPTH_TEST);
        glColor3f(1,1,1);
        drawBitmapText("JOGAR", bx+40, by+15);
        glEnable(GL_DEPTH_TEST);

        // botão LOJA
        int by2 = by - (bh + 20);
        glColor3f(0.8f,0.4f,0.2f);
        glBegin(GL_QUADS);
          glVertex2i(bx, by2);
          glVertex2i(bx+bw, by2);
          glVertex2i(bx+bw, by2+bh);
          glVertex2i(bx, by2+bh);
        glEnd();
        glColor3f(1,1,1);
        // String por cima do botão
        glDisable(GL_DEPTH_TEST);
        glColor3f(1,1,1);
        drawBitmapText("LOJA", bx+45, by2+15);
        glEnable(GL_DEPTH_TEST);

        // restaura
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW);  glPopMatrix();
        glutSwapBuffers();
        return;
    }

    // 2) Loja
    if (menu_state == MENU_SHOP) {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        gluOrtho2D(0, menu_w, 0, menu_h);
        glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();

        // título e pontuação (o main mantém a variável score)
        std::string header = "LOJA - escore atual: " + std::to_string(score);
        drawBitmapText(header.c_str(), 20, menu_h-40);

        // itens
        const int btnW=250, btnH=40, x0=20, y0=menu_h-100, gap=15; // btnW = button width e btnH = button Height
        for(int i=0;i<(int)shop_items.size();++i) {
            int bx=x0, by=y0 - i*(btnH+gap);
            glColor3f(0.3f,0.3f,0.3f);
            glBegin(GL_QUADS);
              glVertex2i(bx, by);
              glVertex2i(bx+btnW+70, by);
              glVertex2i(bx+btnW+50, by+btnH);
              glVertex2i(bx, by+btnH);
            glEnd();
            glColor3f(1,1,1);
            // String por cima do botão
            glDisable(GL_DEPTH_TEST);
            glColor3f(1,1,1);
            drawBitmapText((std::to_string(shop_items[i].cost)+" - "+shop_items[i].name).c_str(),bx+10, by+15);
            glEnable(GL_DEPTH_TEST); 
        }
        // botão Voltar
        int bx=menu_w-120, by=20;
        glColor3f(0.2f,0.8f,0.2f);
        glBegin(GL_QUADS);
          glVertex2i(bx,     by);
          glVertex2i(bx+100, by);
          glVertex2i(bx+100, by+40);
          glVertex2i(bx,     by+40);
        glEnd();
        glColor3f(1,1,1);
        // String por cima do botão
        glDisable(GL_DEPTH_TEST);
        glColor3f(1,1,1);
        drawBitmapText("VOLTAR", bx+15, by+12);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW);  glPopMatrix();
        glutSwapBuffers();
    }

    // restaura matrizes
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();

    // religar a iluminação para o resto do pipeline
    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}

void menu_mouse(int button,int state,int x,int y) {
    if (button!=GLUT_LEFT_BUTTON || state!=GLUT_DOWN) return;
    int wy = menu_h - y;

    // MAIN
    if (menu_state==MENU_MAIN) {
        int bw=140, bh=50;
        int bx=menu_w/2 - bw/2, by=menu_h/2 + 30;
        // JOGAR
        if (x>=bx && x<=bx+bw && wy>=by && wy<=by+bh) {
            menu_state = MENU_NONE;
            if (start_cb) start_cb();
            return;
        }
        // LOJA
        int by2 = by - (bh+20);
        if (x>=bx && x<=bx+bw && wy>=by2 && wy<=by2+bh) {
            menu_state = MENU_SHOP;
            return;
        }
    }

    // SHOP
    if (menu_state==MENU_SHOP) {
        // Voltar
        int bx=menu_w-120, by=20;
        if (x>=bx && x<=bx+100 && wy>=by && wy<=by+40) {
            menu_state = MENU_MAIN;
            return;
        }
        // compra de itens
        const int btnW=250, btnH=40, x0=20, y0=menu_h-100, gap=15;

        for(int i=0;i<(int)shop_items.size();++i) {
            int bx=x0, by=y0 - i*(btnH+gap);

            if (x>=bx && x<=bx+btnW && wy>=by && wy<=by+btnH) {
                if (score >= shop_items[i].cost) {
                    score -= shop_items[i].cost;
                    if (i==1) {  // “+10% Velocidade de disparo”
                        sphere_speed_multiplier *= 1.1f;
                    }
                    else if (i==2) {  // “+1 disparo”
                        shotNum += 1;
                    }
                }
                return;
            }
        }
    }
}

void menu_show() {
    menu_state = MENU_MAIN;
}