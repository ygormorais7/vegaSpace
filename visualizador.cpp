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

#include "glm.h"
#include "glut_text.h"
#include <cmath>



// ------------ Declaracao de constantes e variaveis ------------
#define ESC 27 // ASCII para a tecla ESC
#define MAX_FPS 70 // Maximo de Frames Por Segundo (FPS) desejado
#define FPS 60 // FPS desejado atualmente


// ------------ Variáveis Globais ------------
float reshape_ratio = 1.0; // Manter proporcional a projecao
int fps_desejado = FPS/2; // variavel para alterar os frames por segundo desejado
int fps = 0; //contador de frames por segundo

int faces = 0, vertices = 0; //Numero de faces e vertices do objeto 3D

GLMmodel* model = NULL; // Ponteiro para a estrutura do nosso modelo.
GLuint width = 800, height = 600; // Largura e altura da janela
float posx = 0.0, posy = 0.0, posz = 0.0; // deslocamento do modelo 3D no eixo x, y e z
float graus = 15.0, deslocamento = 0.2; //incrementos do angulo de graus e do deslocamento
float rotx = 0.0, roty = 0.0, rotz = 0.0; // angulo de graus do modelo 3D no eixo x, y e z

bool is_paused = false; // Determina se a animacao esta tocando ou em pausa

//Enumeracoes com as direcoes do Objeto 3D em relacao a camera para realizar movimentacoes
enum direcao_acam{frente, tras, esquerda, direita};
int direcao = tras;

bool is_shooting = false;
bool esfera_ativa = false;           // Indica se a esfera está visível e em movimento
float esfera_x = 0.0;                // Posição X da esfera
float esfera_y = 0.0;                // Posição Y da esfera
float esfera_z = 0.0;                // Posição Z da esfera
float esfera_dir_x = 0.0;            // Direção X do movimento
float esfera_dir_y = 0.0;            // Direção Y do movimento
float esfera_dir_z = 0.0;            // Direção Z do movimento
float esfera_velocidade = 0.07;      // Velocidade de movimento da esfera


// ------------ Declaracoes antecipadas (forward) das funcoes (assinaturas) ------------
void init_glut(const char *window_name, int argc, char** argv);
void reshape(int w, int h);
void display(void);
void keyboard(unsigned char key, int x, int y);
void keyboard_special(int key, int x, int y);
void timer(int value);
void computeFPS(); // Apenas computa FPS, não controla animação
void drawSphere(float x, float y, float z);



// ------------ Funções ------------
int main(int argc, char** argv){
    init_glut("Vega Space", argc, argv);
    glutMainLoop();

    return EXIT_SUCCESS;
}

void init_glut(const char *nome_janela, int argc, char** argv){
    // inicia o GLUT
    glutInit(&argc,argv);

    // Inicia o display usando RGB, double-buffering e Z-buffering
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width,height);
    glutInitWindowPosition(100,100);
    glutCreateWindow(nome_janela);

    // Define as funcoes de callback
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboard_special);
    glutTimerFunc(1000/fps_desejado, timer, 0); //(mseg, timer, value)



    // Carrega o modelo OBJ
    model = glmReadOBJ((char*)"./modelos/naves/nave1.obj", true);
    if (!model) {
        fprintf(stderr, "Erro: Não foi possível carregar o modelo.\n");
        exit(1);
    }

    // Move o modelo para a origem e escala para caber em um cubo de -1 a 1. (Simplifica o posicionamento da camera)
    glmUnitize(model);
    // Gera normais de face e de vértice para que a iluminação funcione corretamente.
    glmFacetNormals(model);
    glmVertexNormals(model, 90.0);

    faces = model->numtriangles;
    vertices = model->numvertices;



    // Configurações de iluminação do OpenGL
    glEnable(GL_LIGHTING); // Sistema de iluminação
    glEnable(GL_LIGHT0); // Luz branca
    glEnable(GL_DEPTH_TEST); // Teste de profundidade, diferencia objetos na frente de outros
    glEnable(GL_COLOR_MATERIAL); // ?

    // Define uma cor de fundo
    glClearColor(0.1, 0.1, 0.1, 1.0);
}

// Mostra tudo que há na tela
void display(void){
    computeFPS();

    // Limpa o buffer de cor e de profundidade
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reseta a matriz de transformações
    glLoadIdentity();

    // Configura a câmera (view matrix)
    gluLookAt(0.0, 0.0, 3.0,  // Posição da câmera: (0, 0, 3)
              0.0, 0.0, 0.0,  // Olhando para a origem: (0, 0, 0)
              0.0, 1.0, 0.0); // Vetor "up" (para cima): (0, 1, 0)

    // Define a posição da fonte de luz
    GLfloat light_pos[] = {2.0, 2.0, 5.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    
    // Aplica as transformações de posição e rotação
    glPushMatrix();                  // Salva o estado atual
    glTranslatef(posx, posy, posz);             // Aplica a translação
    glRotatef(rotx, 1.0, 0.0, 0.0);             // Rotação em X
    glRotatef(roty, 0.0, 1.0, 0.0);             // Rotação em Y
    glRotatef(rotz, 0.0, 0.0, 1.0);             // Rotação em Z
    glmDraw(model, GLM_SMOOTH | GLM_MATERIAL);  // Desenha o modelo usando as funções da biblioteca glm
                                                // GLM_SMOOTH usa normais de vértice para sombreamento suave.
                                                // GLM_MATERIAL aplica as propriedades de material lidas do arquivo .mtl.
    glPopMatrix();                   // Restaura o estado anterior


    if (is_shooting && !esfera_ativa){
        esfera_ativa = true;

        float distancia = 0.4;
        // Converter rotações para radianos
        float roty_rad = (roty * M_PI) / 180.0;
        float rotx_rad = (rotx * M_PI) / 180.0;

        // Salva a direção do movimento baseada na orientação 3D da nave
        // O vetor "para frente" (0,0,1) é rotacionado para corresponder à nave
        // esfera_dir_x = -cos(rotx_rad) * sin(roty_rad);
        // esfera_dir_y = sin(rotx_rad);
        // esfera_dir_z = -cos(rotx_rad) * cos(roty_rad);
        esfera_dir_x = -sin(roty_rad);
        esfera_dir_y = sin(rotx_rad) * cos(roty_rad);
        esfera_dir_z = -cos(roty_rad) * cos(rotx_rad);

        // Define a posição inicial da esfera à frente da nave
        esfera_x = posx + distancia * esfera_dir_x;
        esfera_y = posy + distancia * esfera_dir_y;
        esfera_z = posz + distancia * esfera_dir_z;
    }

    // Se a esfera estiver ativa, move e desenha
    if(esfera_ativa) {
        esfera_x += esfera_dir_x * esfera_velocidade;
        esfera_y += esfera_dir_y * esfera_velocidade;
        esfera_z += esfera_dir_z * esfera_velocidade;
        
        // Save complete OpenGL state
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        
        // Draw the sphere
        drawSphere(esfera_x, esfera_y, esfera_z);
        
        // Restore all OpenGL state
        glPopAttrib();
        
        // Verifica se a esfera saiu da área visível
        if(fabsf(esfera_x) > 20.0 || fabsf(esfera_z) > 20.0) {
            esfera_ativa = false;
            is_shooting = false;
        }
    }

    float x = -reshape_ratio-0.30;
    draw_text_stroke(x , 1.15, "FPS: " + to_string(fps), 0.0005);// + " FACE: " + to_string(faces) + " VERT: " + to_string(vertices));

    // Troca os buffers (double-buffering) para exibir o que foi desenhado
    glutSwapBuffers();
}

// Ajusta o tamanho dos objetos e da janela
void reshape(int w, int h){
    // Configura a projeção (projection matrix)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Configura a viewport para usar toda a janela
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    reshape_ratio = (float)w/(float)h;
    width = w;
    height = h;

    // Projeção em perspectiva: ângulo de 45 graus, aspecto da tela, near=0.1, far=100.0
    gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 0.1, 100.0);
    
    // Volta para a matriz de visão do modelo
    glMatrixMode(GL_MODELVIEW);
}

// Função para computar FPS
void computeFPS(){
    static GLuint frames = 0;
    static GLuint clock;
    static GLuint next_clock = 0;

    frames++;
    clock = glutGet(GLUT_ELAPSED_TIME);

    if (clock < next_clock ) return;
    fps = frames;
    if(next_clock != 0) frames = 0;
    next_clock = clock + 1000;
}

// Desenhar a esfera do tiro
void drawSphere(float x, float y, float z) {
    float radius = 0.02;  // Raio da esfera
    int slices = 20;      // Número de divisões ao redor da esfera
    int stacks = 16;      // Número de divisões do topo até a base
    
    // Use material properties instead of glColor
    GLfloat sphere_ambient[] = {0.0, 0.1, 0.3, 1.0};
    GLfloat sphere_diffuse[] = {0.0, 0.3, 1.0, 1.0};
    GLfloat sphere_specular[] = {0.4, 0.6, 1.0, 1.0};
    GLfloat sphere_shininess = 64.0;
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, sphere_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, sphere_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, sphere_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, sphere_shininess);
    
    glPushMatrix();
        glTranslatef(x, y, z);
        glutSolidSphere(radius, slices, stacks);
    glPopMatrix();
}

// Funcao utilizada para a animacao com temporizador
void timer(int value){
    glutTimerFunc(1000/(fps_desejado), timer, 0);
    glutPostRedisplay(); // Manda redesenhar a tela em cada frame
}

// Atualiza a direção da nave
void update_direcao(){
    if(roty ==  0.0 || roty == 360.0){
        direcao = tras;
        roty = 0.0;
        printf("tras\n"); fflush(stdout);
    }
    if(roty == 180.0 || roty == -180.0){
        direcao = frente;
        roty = 180.0;
        printf("frente\n"); fflush(stdout);
    }
    if(roty ==  90.0){
        direcao = esquerda;
        printf("esquerda\n"); fflush(stdout);
    } 
    if(roty == -90.0 || roty == 270.0){
        direcao = direita;
        printf("direita\n"); fflush(stdout);
    }
}

// Controle das teclas comuns
void keyboard(unsigned char key, int x, int y){
    switch(key){
        // Movimentação do modelo
        case 'w': case 'W': posz -= deslocamento; break; // Move para frente (eixo Z negativo)
        case 's': case 'S': posz += deslocamento; break; // Move para trás (eixo Z positivo)
        case 'a': case 'A': posx -= deslocamento; break; // Move para a esquerda (eixo X negativo)
        case 'd': case 'D': posx += deslocamento; break; // Move para a direita (eixo X positivo)
        case 'q': case 'Q': posy -= deslocamento; break; // Move para baixo (eixo Y negativo)
        case 'e': case 'E': posy += deslocamento; break; // Move para cima (eixo Y positivo)

        // Reseta a posição e rotação
        case 'r': case 'R': 
            posx = 0.0; posy = 0.0; posz = 0.0;
            rotx = 0.0; roty = 0.0; rotz = 0.0;
        break;

        // Controle de FPS
        case ',': case '<': if(fps_desejado > 1) fps_desejado -= 1; break;
        case '.': case '>': if(fps_desejado*2 < MAX_FPS) fps_desejado += 1; break;
        case ESC: exit(EXIT_SUCCESS); break;

        case 'f': case 'F': 
        if(!is_paused) {
            is_shooting = true;
            esfera_ativa = false;
        }
    }
}

// Teclas especiais (F1-F12, setas)
void keyboard_special(int key, int x, int y){
    switch(key){
        case GLUT_KEY_RIGHT:
            roty += graus; printf("roty = %f\n", roty); fflush(stdout);
            update_direcao();
            break;
        case GLUT_KEY_LEFT:
            roty -= graus; printf("roty = %f\n", roty); fflush(stdout);
            update_direcao();
            break;
        case GLUT_KEY_UP:
            rotx -= graus; printf("rotx = %f\n", rotx); fflush(stdout);
            update_direcao();
            break;
        case GLUT_KEY_DOWN:
            rotx += graus; printf("rotx = %f\n", rotx); fflush(stdout);
            update_direcao();
            break;
    }
}