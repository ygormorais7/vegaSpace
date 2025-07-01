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
#include <algorithm>
#include <random>
#include <cmath>



// --- Constantes ---
#define ESC 27
#define MAX_FPS 70

// --- Variáveis de FPS ---
static int fps_desejado = 30;
static int fps = 0;

// --- Modelos ---
static Ship ship;
//static Asteroid asteroid;
static Esfera sphere;

// --- Janela e aspect ---
static float reshape_ratio = 1.0;
static unsigned int width  = 800;
static unsigned int height = 600;

// --- Lista de Esferas, Asteroides e Pause ---
static std::vector<Esfera> spheres;         // Lista de esferas
static std::vector<Asteroid> asteroids;     // Lista de asteroides
size_t numAtivas = 0;                       // Número de esferas ativas
bool is_paused = false;                     // Determina se a animacao esta tocando ou em pausa

static int score = 0;
static bool is_gameover = false;
static const float SPHERE_RADIUS = 0.02;
static const float SHIP_RADIUS = 0.5;
static const int N_ASTEROIDS = 10;

// ------------ Declaracoes antecipadas (forward) das funcoes (assinaturas) ------------
static void init_glut(int argc, char** argv);
static void reshape(int w, int h);
static void display();
static void keyboard(unsigned char key, int x, int y);
static void keyboard_special(int key, int x, int y);
static void timer(int);
static void computeFPS();
static void spawn_asteroids();
static void reset_game();



int main(int argc, char** argv) {
    init_glut(argc, argv);

    // Inicializa a nave
    ship_init(ship, "./modelos/naves/nave1.obj");
    // Inicializa a esfera
    sphere_init(sphere);
    // Inicializa os asteroides
    spawn_asteroids();

    // Evitar vazamento de memória limpando-a
    atexit([](){
        ship_cleanup(ship);
        for (auto& a : asteroids)
            asteroid_cleanup(a);
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

    // desenha a nave
    ship_draw(ship);
    // desenha e move os asteroides
    for (auto& a : asteroids){
        if (!is_paused) asteroid_update(a, 0.1);
        asteroid_draw(a);
    }

    // atualiza, desenha e remove esferas inativas
    for (auto it = spheres.begin(); it != spheres.end(); ) {
        if (!is_paused) sphere_update(*it);
        if (!it->active) {
            it = spheres.erase(it); // Apaga as esferas que passaram do limite
        } else {
            sphere_draw(*it);
            ++it;
        }
    }

    // colisão Esfera vs Asteroide
    if (!is_paused) {
        for (auto& a : asteroids) {
            for (auto it = spheres.begin(); it != spheres.end(); ++it) {
                // 1) calcula vetor diferença entre centros
                float dx = a.posx - it->x;
                float dy = a.posy - it->y;
                float dz = a.posz - it->z;
                float dist2 = dx*dx + dy*dy + dz*dz;

                // 2) soma dos raios das bounding‐spheres
                //    SPHERE_RADIUS é o raio fixo da bala;
                //    a.scale ~ raio do asteroide
                float R = SPHERE_RADIUS + (a.scale); // assume bound≈scale*1

                // 3) se distância² < (raio_total)², houve colisão
                if (dist2 < R*R) {
                    // colisão: reposiciona asteroide, remove esfera e pontua
                    it->active = false;         // “destrói” a esfera
                    a.posz = -20.0f;            // reposiciona o asteroide lá atrás
                    score += 10;                // incrementa a pontuação
                }
            }
        }
    }

    // colisão Asteroide vs Nave
    if (!is_paused) {
        for (auto& a : asteroids) {
            // 1) calcula o vetor diferença entre os centros
            float dx = a.posx - ship.posx;
            float dy = a.posy - ship.posy;
            float dz = a.posz - ship.posz;
            float dist2 = dx*dx + dy*dy + dz*dz;

            // 2) soma dos raios das bounding spheres (nave + asteroide)
            float R = SHIP_RADIUS + (a.scale);

            // 3) se distância² < (raio_total)², houve colisão
            if (dist2 < R*R) {
                is_paused   = true;     // pausa o jogo
                is_gameover = true;     // sinaliza Game Over
                break;
            }
        }
    }

    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glColor3f(1.0, 1.0, 1.0);
    // desenha FPS
    draw_text_stroke(-reshape_ratio - 0.30, 1.15, "FPS: "+std::to_string(fps), 0.0005);
    // exibe pontuação
    draw_text_stroke(-reshape_ratio, 1.15, "Score: " + std::to_string(score), 0.0005);

    // tela de Game Over
    if (is_gameover) {
        draw_text_stroke(-0.5, 0.0, "GAME OVER", 0.002);
        draw_text_stroke(-0.7, -0.2, "Pressione 'J' para jogar novamente", 0.0005);
    }
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

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

// Cria/posiciona n asteroides
static void spawn_asteroids() {
    asteroids.clear();
    std::random_device rd;
    std::mt19937       rng(rd());
    std::uniform_real_distribution<float> distXZ(0.7f, 1.3f);
    std::uniform_real_distribution<float> distY (0.9f, 1.2f);

    for (int i = 0; i < N_ASTEROIDS; ++i) {
        float rXZ = distXZ(rng), rY = distY(rng);
        float x = rXZ * (-5.0f + i * 2.5f);
        float y = rY  * ( i * 1.0f);
        float s = rXZ * (0.5f + 0.1f * i);

        Asteroid a;
        asteroid_init(a, "./modelos/asteroides/asteroid1/asteroidBase.obj", x, y, -20.0, s, 0.0, 45.0 * i, 0.0);
        asteroids.push_back(a);
    }
}

static void reset_game() {
    score = 0;
    is_paused = false;
    is_gameover = false;
    ship_reset(ship);
    spheres.clear();
}

static void timer(int) {
    glutTimerFunc(1000/fps_desejado, timer, 0);
    glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y) {
    if (is_gameover && (key=='j' || key=='J')) {
        reset_game();
        return;
    }
    
    switch (key) {
        case ESC: exit(EXIT_SUCCESS);break;

        case ',': case '<': if(fps_desejado > 1) fps_desejado--; break;

        case '.': case '>': if(fps_desejado*2 < MAX_FPS) fps_desejado += 1; break;

        default:
            if (!is_paused){
                ship_handle_key(ship, key);
            }
            break;

        case 'f': case 'F':
            if (!is_paused && spheres.size() < 5) {
                Esfera e;
                sphere_init(e);
                sphere_fire(e, ship);
                spheres.push_back(e);
            }
            break;
    }
}

// Teclas especiais (F1-F12, setas)
static void keyboard_special(int key, int x, int y) {
    if (!is_paused) ship_handle_special_key(ship, key);
}