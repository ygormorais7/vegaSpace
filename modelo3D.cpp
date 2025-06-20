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

/* Inclui o arquivo header com a textura */
#include "glm.h"
#include "glut_text.h"
#include <cstdio>
#include <cmath>
#include <map>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif



/*
 * Declaracao de constantes e variaveis
 */
#define ESC 27 // ASCII para a tecla ESC
#define SPACE 32 // ASCII para a tecla de ESPACO
#define MAX_FPS 70 // Maximo de Frames Por Segundo (FPS) desejado
#define FPS 120 // FPS desejado atualmente
#define KEYFRAME_RATE 3 // taxa de repeticoes do keyframe da animacao nos quadros da video
#define POSY 0.0  // Posição inicial Y da nave
#define CAMZ 5.0    //posicao da camera no eixo z


int fps_desejado = FPS/2; // variavel para alterar os frames por segundo desejado
int fps = 0; //contador de frames por segundo
float reshape_ratio = 1.0; // Manter proporcional a projecao
GLuint width = 640, height = 480; // Largura e altura da janela

// Variaveis de animacao removidas ou adaptadas
GLMmodel* starship_model = NULL; // Ponteiro para o modelo da nave

int faces = 0, vertices = 0; //Numero de faces e vertices do objeto 3D

float graus = 15.0, deslocamento = 0.2; //incrementos do angulo de graus e do deslocamento
float scale = 0.5; // Fator de escala. Ajuste conforme o tamanho do seu modelo.
float rotx = 0.0, roty = 0.0, rotz = 0.0; // angulo de graus do modelo 3D no eixo x, y e z
float posx = 0.0, posy = POSY, posz = 0.0; // deslocamento do modelo 3D no eixo x, y e z
float R = 1.0, G = 1.0, B = 1.0; //Cores do desenho controladas pelo glColor (inicial: cor branca)
float camx = 0.0, camy = 0.0, camz = CAMZ; // posicao da camera no eixo x, y e z
float cam_rotx = 0.0, cam_roty = 0.0; // angulo de graus da camera no eixo x e y
float cam_up = 1.0; //direcao do vetor para cima da camera

bool use_perspective = true; // Determina se usa projecao perspectiva ou ortografica
bool use_gouraud = true; // Determina o uso de Gouraud ou Flat shading
bool use_texture = true; // Determina o uso de texturas
bool use_color_material = false; // Determina o uso da glColor como parametro do material
bool use_depth_test = true; // Determina o uso ou não do Z-buffering
bool use_light = true; // Determina se liga ou desliga a luz
bool render_solid = true; // Determina se renderiza o objeto solido ou em wireframe

GLint apply_texture = GL_DECAL; //Determina como a textura sera aplicada no objeto
std::string apply_texture_name = "Decal"; //Nome da fucao que sera usada para aplicar a textura no objeto

/*
 * Definicao dos parametros do modelo de iluminacao
 */
GLfloat light_pos[] = {-2.0, 2.0, 2.0, 0.0};
GLfloat light_Ka[] = {0.4, 0.4, 0.4, 0.0};
GLfloat light_Kd[] = {1.0, 1.0, 1.0, 0.0};
GLfloat light_Ks[] = {1.0, 1.0, 1.0, 0.0};

/*
 * Definicao dos parametros do material para o modelo de iluminacao
 * (Estes podem ser sobrescritos pelo arquivo .mtl do modelo)
 */
GLfloat material_Ka[] = {0.33, 0.22, 0.03, 1.00};
GLfloat material_Kd[] = {0.78, 0.57, 0.11, 1.00};
GLfloat material_Ks[] = {0.99, 0.94, 0.81, 1.00};
GLfloat material_Ke[] = {0.00, 0.00, 0.00, 0.00};
GLfloat material_Se = 28;

/*
 * Declaracoes antecipadas (forward) das funcoes (assinaturas)
 */
void init_glut(const char *window_name, int argc, char** argv);
void reshape(int w, int h);
void display(void);
void drawModel(GLuint mode); // Função de desenho genérica
void computeFPS(); // Apenas computa FPS, não controla animação
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void keyboard_special(int key, int x, int y);
void menu(int value);

/*
 * Funcao principal
 */
int main(int argc, char** argv){
    // inicia o GLUT e alguns parametros do OpenGL
    init_glut("Computacao Grafica: Visualizador de Nave Espacial", argc, argv);

    // funcao de controle do GLUT
    glutMainLoop();

    return EXIT_SUCCESS;
}

/*
 * Funcao para inicializacao do GLUT e de alguns parametros do OpenGL
 */
void init_glut(const char *nome_janela, int argc, char** argv){
    // inicia o GLUT
    glutInit(&argc,argv);

    // Inicia o display usando RGB, double-buffering e Z-buffering
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width,height);
    glutInitWindowPosition(100,100);
    glutCreateWindow(nome_janela);
    
    // Define as funcoes de callback
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(keyboard_special);
    glutTimerFunc(1000/fps_desejado, timer, 0); //(mseg, timer, value)

    // Define o menu
    glutCreateMenu(menu);
    glutAddMenuEntry("Desenhar a azul", 1);
    glutAddMenuEntry("Desenhar a verde", 2);
    glutAddMenuEntry("Desenhar a vermelho", 3);
    glutAddMenuEntry("Sair", 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    /*
     * Ativacao dos parametros do modelo de iluminacao para a Luz 0
     */
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_Ka);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_Kd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_Ks);

    /*
     * Ativacao dos parametros do material para uso do modelo de iluminacao
     */
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_Ka);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_Kd);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_Ks);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material_Ke);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material_Se);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    /*
     * Configurando o OpenGL para o uso de Texturas
     */
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, apply_texture);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    /*
     * Carregando o Modelo 3D
     */
    printf("Loading 3D Model...");
    starship_model = glmReadOBJ((char*)"./Starship_Collection.obj");
    if (!starship_model) {
        printf("Error: Failed to load model.\n");
        exit(1);
    }

    // Opcional: Unitize o modelo para garantir que ele se encaixe bem na cena.
    glmUnitize(starship_model);
    glmFacetNormals(starship_model);
    glmVertexNormals(starship_model, 90.0);

    faces = starship_model->numtriangles;
    vertices = starship_model->numvertices;
    printf(" done.\n");

    // Disable color material by default to ensure model materials are used
    glDisable(GL_COLOR_MATERIAL);
    use_color_material = false;
    
    // Set texture mode to MODULATE to better preserve material colors
    apply_texture = GL_MODULATE;
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, apply_texture);
}

/*
 * Processa o reshape da janela
 */
void reshape(int w, int h){
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    reshape_ratio = (float)w/(float)h;
    width = w;
    height = h;

    if(use_perspective){
        gluPerspective(30, reshape_ratio, 1.0, 15.0);
    }else{
        float ratioh = 1.34, ratiow = reshape_ratio * ratioh;
        glOrtho (-ratiow, ratiow, -ratioh, ratioh, 1.0 , 15.0);
    }
    
    glMatrixMode (GL_MODELVIEW);
}

/*
 * Funcao para controlar o display
 */
void display(void){
    computeFPS();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    gluLookAt(camx, camy, camz, 0.0, 0.0, 0.0, 0.0, cam_up, 0.0);

    if(use_texture) glEnable(GL_TEXTURE_2D);
    else glDisable(GL_TEXTURE_2D);
    
    if(use_light) glEnable(GL_LIGHTING);
    else glDisable(GL_LIGHTING);
    
    GLuint mode;
    if(use_gouraud) mode = (GLM_SMOOTH | GLM_MATERIAL);
    else mode = (GLM_FLAT | GLM_MATERIAL);

    glColor3f(R, G, B);
    
    glPushMatrix();
        glScaled(scale, scale, scale);
        glTranslated(posx, posy, posz);
        glRotated(rotx, 1.0f, 0.0f, 0.0f);
        glRotated(roty, 0.0f, 1.0f, 0.0f);
        glRotated(rotz, 0.0f, 0.0f, 1.0f);
        drawModel(mode);
    glPopMatrix();
    
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(0.0, 0.0, 0.0);
    std::string cor = (R && G && B)?"White":(B)?"Blue":(G)?"Green":"Red";
    std::string luz = (use_light)?"On":"Off";
    std::string dep = (use_depth_test)?"On":"Off";
    std::string sha = (use_gouraud)?"Smooth":"Flat";
    std::string mat = (use_color_material)?"On":"Off";
    std::string tex = (use_texture)?"On":"Off";
    std::string ren = (render_solid)?"Solid":"Wired";
    std::string pro = (use_perspective)?"Persp":"Ortho";
    float x = -reshape_ratio-0.45;
    draw_text_stroke(x , 1.25, "(ESC)EXIT (</>)FPS: " + to_string(fps) + " FACE: " + to_string(faces) + " VERT: " + to_string(vertices));
    draw_text_stroke(x , 1.15,"(R)RESET (WASDQE)MOVE (Arrows)ROTATE (F1)PROJ: " + pro );
    draw_text_stroke(x , 1.05, "(F2)RENDER: " + ren + " (F3)C.MAT: " + mat + " (F4)COLOR: " +cor+ " (F5)SHADE: " + sha);
    draw_text_stroke(x , 0.95, "(F6)DEPTH: " + dep + " (F7)LIGHT: "+ luz + " (F8)TEX: " + tex + " (F9)APPLY: " + apply_texture_name);
    
    glutSwapBuffers();
}

/*
 * Função que apenas computa FPS
 */
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

// Função de desenho do modelo
void drawModel(GLuint mode){
    if (starship_model) {
        glmDraw(starship_model, mode | GLM_TEXTURE);
    }
}

/*
 * Funcao utilizada para a animacao com temporizador
 */
void timer(int value){
    glutTimerFunc(1000/(fps_desejado), timer, 0);
    glutPostRedisplay(); // Manda redesenhar a tela em cada frame
}

/*
 * Controle das teclas comuns
 */
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
            posx = 0.0; posy = POSY; posz = 0.0;
            rotx = 0.0; roty = 0.0; rotz = 0.0;
        break;

        // Controle de FPS
        case ',': case '<': if(fps_desejado > 1) fps_desejado -= 1; break;
        case '.': case '>': if(fps_desejado*2 < MAX_FPS) fps_desejado += 1; break;
        case ESC: exit(EXIT_SUCCESS); break;
    }
}

/*
 * Controle das teclas especiais (Cursores para Rotação)
 */
void keyboard_special(int key, int x, int y){
    switch(key){
        case GLUT_KEY_F1: 
            use_perspective = !use_perspective;
            reshape(width, height);
        break;
        case GLUT_KEY_F2:
            if (render_solid) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            render_solid = !render_solid;
        break;
        case GLUT_KEY_F3: 
            if (use_color_material){
                glDisable(GL_COLOR_MATERIAL);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_Ka);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_Kd);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_Ks);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material_Ke);
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material_Se);
            } else {
                glEnable(GL_COLOR_MATERIAL);
            }
            use_color_material = !use_color_material;
        break;
        case GLUT_KEY_F4: 
            if (R && G && B) { R = 0.0; G = 0.0; B = 1.0;}
            else if(B){ R = 0.0; G = 1.0; B = 0.0;}
            else if (G){ R = 1.0; G = 0.0; B = 0.0;}
            else { R = 1.0; G = 1.0; B = 1.0;}
        break;
        case GLUT_KEY_F5: use_gouraud = !use_gouraud; break;
        case GLUT_KEY_F6: 
            if (use_depth_test) glDisable(GL_DEPTH_TEST);
            else glEnable(GL_DEPTH_TEST);
            use_depth_test = !use_depth_test;
        break;
        case GLUT_KEY_F7: use_light = !use_light; break;
        case GLUT_KEY_F8: use_texture = !use_texture; break;
        case GLUT_KEY_F9:
            switch (apply_texture) {
                case GL_ADD:
                    apply_texture = GL_DECAL;
                    apply_texture_name = "Decal";
                break;
                case GL_MODULATE:
                    apply_texture = GL_BLEND;
                    apply_texture_name = "Blend";
                break;
                case GL_BLEND:
                    apply_texture = GL_ADD;
                    apply_texture_name = "Add";
                break;
                case GL_DECAL:
                    apply_texture = GL_MODULATE;
                    apply_texture_name = "Modulate";
                break;
            }
        // Rotação do modelo
        case GLUT_KEY_RIGHT: roty += graus; break;
        case GLUT_KEY_LEFT:  roty -= graus; break;
        case GLUT_KEY_UP:    rotx -= graus; break;
        case GLUT_KEY_DOWN:  rotx += graus; break;

    }
}

/*
 * Controle do menu pop-up
 */
void menu(int value){
    switch(value){
        case 1: R = 0.0; G = 0.0; B = 1.0; break;//Muda a cor para azul
        case 2: R = 0.0; G = 1.0; B = 0.0; break;//Muda a cor para verde
        case 3: R = 1.0; G = 0.0; B = 0.0; break;//Muda a cor para vermelho
        case 0: exit(EXIT_SUCCESS); break; //Sai do programa
    }
}
