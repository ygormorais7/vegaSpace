/*
 * Computacao Grafica
 * Projeto Asteroids
 * Autores: Carol e Ygor
 */

/* Inclui os headers do OpenGL, GLU, e GLUT */
#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include "glut_text.h"
#include "starship.h"
#include "asteroid.h"
#include "sphere.h"
#include <cmath>



// --- Constantes ---
#define ESC 27
#define MAX_FPS 70

// --- Variáveis de FPS ---
static int fps_desejado = 30;
static int fps = 0;

// --- Modelos ---
static Ship ship;
static Asteroid asteroid;
static Esfera sphere;

// --- Janela e aspect ---
static float reshape_ratio = 1.0;
static unsigned int width  = 800;
static unsigned int height = 600;

static std::vector<Esfera> spheres;  // Lista de esferas
size_t numAtivas = 0; // Número de esferas ativas
bool is_paused = false; // Determina se a animacao esta tocando ou em pausa

// ------------ Declaracoes antecipadas (forward) das funcoes (assinaturas) ------------
static void init_glut(int argc, char** argv);
static void reshape(int w, int h);
static void display();
static void keyboard(unsigned char key, int x, int y);
static void keyboard_special(int key, int x, int y);
static void timer(int);
static void computeFPS();
static void drawSphere(float x, float y, float z);



int main(int argc, char** argv) {
    init_glut(argc, argv);

    // inicializa a nave
    ship_init(ship, "./modelos/naves/nave1.obj");
    // carrega asteroide
    asteroid_init(asteroid, "./modelos/asteroides/asteroid1/asteroidBase.obj", 2.0f, 1.0f, -10.0f);
    // Inicia a esfera
    esfera_init(sphere);

    // Evitar vazamento de memória limpando-a
    atexit([](){
        ship_cleanup(ship);
        asteroid_cleanup(asteroid);
    });

    glutMainLoop();
    return 0;
}

static void init_glut(int argc, char** argv) {
    glutInit(&argc, argv);

    // Inicia o display usando RGB, double-buffering e Z-buffering
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Vega Space");

    // Define as funcoes de callback
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboard_special);
    glutTimerFunc(1000/fps_desejado, timer, 0);

    // Configurações de iluminação do OpenGL
    glEnable(GL_LIGHTING);          // Sistema de iluminação
    glEnable(GL_LIGHT0);            // Luz branca
    glEnable(GL_DEPTH_TEST);        // Teste de profundidade, diferencia objetos na frente de outros
    glEnable(GL_COLOR_MATERIAL);
    // Define uma cor de fundo
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

// Mostra tudo que há na tela
static void display() {
    computeFPS();

    // Limpa o buffer de cor e de profundidade
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Reseta a matriz de transformações
    glLoadIdentity();

    // Configura a câmera (view matrix)
    gluLookAt(0.0, 0.0, 3.0,    // Posição da câmera: (0, 0, 3)
              0.0, 0.0, 0.0,    // Olhando para a origem: (0, 0, 0)
              0.0, 1.0, 0.0);   // Vetor "up" (para cima): (0, 1, 0)

    // Define a posição da fonte de luz
    GLfloat light_pos[] = {2.0, 2.0, 5.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    ship_draw(ship);
    asteroid_draw(asteroid);

    // atualiza, desenha e remove esferas inativas
    for (auto it = spheres.begin(); it != spheres.end(); ) {
        esfera_update(*it);
        if (!it->active) {
            it = spheres.erase(it); // Apaga as esferas que passaram do limite
        } else {
            esfera_draw(*it);
            ++it;
        }
    }

    // desenha FPS
    float x = -reshape_ratio - 0.30f;
    draw_text_stroke(x, 1.15f, "FPS: "+std::to_string(fps), 0.0005f);

    // Troca os buffers (double-buffering) para exibir o que foi desenhado
    glutSwapBuffers();
}

// Ajusta o tamanho dos objetos e da janela
static void reshape(int w, int h) {
    width = w; height = h;
    reshape_ratio = float(w)/float(h);
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, reshape_ratio, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Função para computar FPS
static void computeFPS() {
    static int frames = 0;
    static int next = 0;
    frames++;
    int t = glutGet(GLUT_ELAPSED_TIME);
    if (t < next) return;
    fps = frames;
    frames = 0;
    next = t + 1000;
}

// Desenhar a esfera do tiro
static void drawSphere(float x, float y, float z) {
    GLfloat amb[] = {0.0f,0.1f,0.3f,1.0f},
            dif[] = {0.0f,0.3f,1.0f,1.0f},
            spec[] = {0.4f,0.6f,1.0f,1.0f};
    glMaterialfv(GL_FRONT,GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT,GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT,GL_SPECULAR,spec);
    glMaterialf (GL_FRONT,GL_SHININESS,64.0f);
    glPushMatrix();
      glTranslatef(x,y,z);
      glutSolidSphere(0.02,20,16);
    glPopMatrix();
}

static void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case ESC: exit(EXIT_SUCCESS);break;

        case ',': case '<': if(fps_desejado > 1) fps_desejado -= 1; break;

        case '.': case '>': if(fps_desejado*2 < MAX_FPS) fps_desejado += 1; break;

        default: ship_handle_key(ship, key); break;

        case 'f': case 'F':
            numAtivas = spheres.size();
            if (!is_paused && numAtivas < 5) {
                Esfera e;
                esfera_init(e);
                esfera_fire(e, ship);
                spheres.push_back(e);
            }
    }
}

// Teclas especiais (F1-F12, setas)
static void keyboard_special(int key, int x, int y) {
    ship_handle_special_key(ship, key);
}

static void timer(int) {
    glutTimerFunc(1000/fps_desejado, timer, 0);
    glutPostRedisplay();
}