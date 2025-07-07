/*
 * Computacao Grafica
 * Projeto Asteroids - VegaSpace
 * Autores: 
 *          Ana Carolline Santos Silva
 *          Ygor Francisco de Carvalho Morais
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

#include "../includes/glut_text.h"
#include "../includes/starship.h"
#include "../includes/asteroid.h"
#include "../includes/sphere.h"
#include "../includes/menu.h"
#include "../includes/stars.h"
#include "../includes/audio.h"
#include "../includes/enemy.h"
#include <algorithm>
#include <random>
#include <cmath>


// --- aleatorização --- 
static std::mt19937 rng{ std::random_device{}() };
static std::uniform_real_distribution<float> distXZ(0.7f, 1.3f);
static std::uniform_real_distribution<float> distY (0.9f, 1.2f);

// --- Constantes ---
#define ESC 27
#define MAX_FPS 70
#define NUM_STARS 1100      // Número de estrelas no fundo
#define STARS_RADIUS 100.0f // Distância das estrelas

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
static std::vector<EnemyShip> enemies; 
size_t numAtivas = 0;                       // Número de esferas ativas

bool is_paused = false;                     // Determina se a animacao esta tocando ou em pausa
int score = 0;
int shotNum = 3;
static bool is_gameover = false;
static bool enemy_spawned = false; 
float sphere_speed_multiplier = 1.0f;

static const float SPHERE_RADIUS = 0.02;
static const float SHIP_RADIUS = 0.3;
static const int N_ASTEROIDS = 10;

// nave inimiga
static const float ENEMY_RADIUS = 0.6;
static const int N_ENEMIES = 1;             // Quantidade de inimigos criados
static int last_enemy_fire_time = 0;

// ------------ Declaracoes antecipadas (forward) das funcoes (assinaturas) ------------
static void init_glut(int argc, char** argv);
static void reshape(int w, int h);
static void display();
static void keyboard(unsigned char key, int x, int y);
static void keyboard_special(int key, int x, int y);
static void timer(int);
static void computeFPS();
static void spawn_asteroids();
static void spawn_enemies();
static void reset_game();
static void start_or_resume();



int main(int argc, char** argv) {
    init_glut(argc, argv);

    // Inicializa e carrega os sons
    audio_init();
    audio_load_music("./assets/sounds/ambient.mp3"); // musica ambiente
    audio_load_sfx("./assets/sounds/laserShot.wav"); // som do tiro

    // Inicializa a nave
    ship_init(ship, "./modelos/naves/nave1.obj");
    // Inicializa a esfera
    sphere_init(sphere);
    // Inicializa os asteroides
    spawn_asteroids();
    // Inicializa as estrelas
    init_stars(NUM_STARS, STARS_RADIUS);

    // Evitar vazamento de memória limpando-a
    atexit([](){
        ship_cleanup(ship);
        for (auto& a : asteroids)
            asteroid_cleanup(a);
        for (auto& e : enemies)
            enemy_cleanup(e);
        cleanup_stars();
        audio_cleanup();
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

    // inicializa menu antes de registrar os outros callbacks
    menu_init();
    menu_set_size(width, height);
    menu_set_start_callback(start_or_resume);

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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

// Mostra tudo que há na tela
static void display() {
    computeFPS();

    if (menu_active()) {
        menu_display();
        audio_play_music_loop(); 
        return;
    }

    // reinstaura projeção em perspectiva
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, reshape_ratio, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Limpa o buffer de cor e de profundidade
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Reseta a matriz de transformações
    glLoadIdentity();

    // Configura a câmera (view matrix)
    gluLookAt(0.0, 0.0, 3.0,    // Posição da câmera: (0, 0, 3)
              0.0, 0.0, 0.0,    // Olhando para a origem: (0, 0, 0)
              0.0, 1.0, 0.0);   // Vetor "up" (para cima): (0, 1, 0)

    // Desenha as estrelas no background
    draw_stars();

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

    // Verifica se a pontuação foi atingida e se os inimigos ainda não foram criados
    if (!is_paused && !enemy_spawned && score >= 50) { // pontuação para aparecer os inimigos (eh pra ser 100)
        spawn_enemies();
        enemy_spawned = true; // Marca que já foram criados para não criar de novo
    }

    // Desenha e atualiza todos os inimigos ativos
    for (auto& e : enemies) {
        if (e.active) {
            if (!is_paused) enemy_update(e, ship);  // agora recebe a ship
            enemy_draw(e);
        }
    }

    // Enemy firing: a cada 3s dispara uma esfera na direção que olha  
    int now = glutGet(GLUT_ELAPSED_TIME);
    if (!is_paused && enemy_spawned && now - last_enemy_fire_time >= 3000) {
        for (auto& e : enemies) {
            if (!e.active) continue;
            Esfera be;
            sphere_init(be);
            sphere_fire_enemy(be, e, 0.6f);
            spheres.push_back(be);
        }
        last_enemy_fire_time = now;
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
                // ignora disparos da nave inimiga
                if (!it->active || it->is_enemy) continue;
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
                    asteroid_respawn(a);        // reposiciona o asteroide
                    score += 10;                // incrementa a pontuação
                }
            }
        }
    }

    // colisão Esfera do Jogador vs Inimigo
    if (!is_paused) {
        for (auto& enemy : enemies) {
            if (!enemy.active) continue;

            // verifique a colisão com cada esfera do jogador.
            for (auto it = spheres.begin(); it != spheres.end(); ++it) {
                if (!it->active) continue;

                float dx = enemy.posx - it->x;
                float dy = enemy.posy - it->y;
                float dz = enemy.posz - it->z;
                float dist2 = dx * dx + dy * dy + dz * dz;

                float R = ENEMY_RADIUS + SPHERE_RADIUS;

                if (dist2 < R * R) {
                    it->active = false;      // Desativa a esfera do jogador
                    enemy.active = false;    // “destrói” a nave inimiga
                    score += 50;             // Incrementa a pontuação
                    break;
                }
            }
        }
    }  

    // colisão Esfera (jogador) vs EnemyShip
    if (!is_paused) {
        for (auto& en : enemies) {
            if (!en.active) continue;
            for (auto& s : spheres) {
                if (!s.active || s.is_enemy) continue;
                float dx = en.posx - s.x;
                float dy = en.posy - s.y;
                float dz = en.posz - s.z;
                float dist2 = dx*dx + dy*dy + dz*dz;
                float R = SPHERE_RADIUS + ENEMY_RADIUS;
                if (dist2 < R*R) {
                    // “destrói” inimigo e projétil
                    en.active = false;
                    s.active  = false;
                    // opcional: dar pontos ao jogador
                    score += 20;
                    break;  // sai do loop de esferas para este inimigo
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

    // colisão Esfera inimiga vs Nave
    if (!is_paused) {
        for (auto& s : spheres) {
            if (!s.active || !s.is_enemy) continue;
            float dx = s.x - ship.posx;
            float dy = s.y - ship.posy;
            float dz = s.z - ship.posz;
            float dist2 = dx*dx + dy*dy + dz*dz;
            float R = SPHERE_RADIUS + SHIP_RADIUS;
            if (dist2 < R*R) {
               // Game Over ao ser atingido por tiro inimigo
               is_paused   = true;
            is_gameover = true;
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
    draw_text_stroke(-reshape_ratio, 1.15, "ESCORE: " + std::to_string(score), 0.0005);

    // tela de Game Over
    if (is_gameover) {
        draw_text_stroke(-0.78, 0.0, "GAME OVER", 0.002);
        draw_text_stroke(-0.6, -0.2, "Pressione 'J' para jogar novamente", 0.0005);

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

    // informa o menu do novo tamanho de janela
    menu_set_size(w, h);
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

// cria as naves inimigas
static void spawn_enemies() {
    enemies.clear(); // limpa inimigos antigos

    // Cria única nave inimiga parada
    EnemyShip new_enemy;
    enemy_init(new_enemy, "./modelos/naves/space_battleship_lowpoly.obj");
    new_enemy.posx   = 0.0f;
    new_enemy.posy   = 0.0f;
    new_enemy.posz   = -10.0f;
    new_enemy.roty   = 180.0f;
    new_enemy.speed  = 0.0f;      // sem movimento
    enemies.push_back(new_enemy);

    printf("1 nave inimiga criada!\n");
}

static void reset_game() {
    score = 0;
    is_paused = false;
    is_gameover = false;
    ship_reset(ship);
    spheres.clear();
}

static void start_or_resume() {
    if (is_gameover) {
        reset_game();      // zerar score e flags apenas se for Game Over
    } else {
        is_paused = false; // caso tenha vindo de “P”, só despausar
    }
    // Para a música ambiente quando o jogo começa
    audio_stop_music();
}

static void timer(int) {
    glutTimerFunc(1000/fps_desejado, timer, 0);
    glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y) {
    // Garante prioridade
    if (key=='p' || key=='P') {
        is_paused = true;
        menu_show();
        audio_play_music_loop(); // Toca a música do menu ao pausar
        return;
    }

    // Garante só resetar o jogo se is_gameover estiver ativo
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
            if (!is_paused) {
                // conta só esferas ativas que NÃO são de inimigo
                int playerShots = std::count_if(spheres.begin(), spheres.end(), [](const Esfera& b){ return b.active && !b.is_enemy; });
                if (playerShots < shotNum) {
                    Esfera e;
                    sphere_init(e);
                    sphere_fire(e, ship);
                    e.speed *= sphere_speed_multiplier;
                    spheres.push_back(e);
                    audio_play_sfx();
                }
            }
            break;
    }
}

// Teclas especiais (F1-F12, setas)
static void keyboard_special(int key, int x, int y) {
    if (!is_paused) ship_handle_special_key(ship, key);
}