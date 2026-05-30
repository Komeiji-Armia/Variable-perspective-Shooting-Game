#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <mmsystem.h>
#include "GL/glut.h"
#include "GL/gl.h"
#include "GL/glu.h"
#include "myShape.h"
#include "sound.h"
#define KEY_ESC 27

/* =========================================================
   Game constants
   ========================================================= */
#define ENEMY_LIMIT   15      /* max simultaneous enemies   */
#define BULLET_MOD    2       /* mod N for bullet type      */
#define ENEMY_RADIUS  0.5f    /* enemy hit-sphere radius Core   */
#define PLAYER_RADIUS 0.2f    /* player hit-sphere radius Core  */
#define SCORE_PER_KILL 100

/* =========================================================
   Camera / player state
   ========================================================= */
float distance = 20.0f;
float m = 0.0f, n = -3.0f;          /* player X, Y            */
float py = 0.0f, size = 0.0f;       /* player bullet Y, active*/

float dist  = 0.0f;
float theta = 0.0f;

float diffuse[]  = { 0.0f, 1.0f, 0.0f, 1.0f };
float specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
float ambient[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
float shininess  = 128.0f;

/* [Fix1] key state: [0-255] normal, [256-511] special (+256) */
int keyState[512] = {0};
int shiftState    = 0;

/* =========================================================
   Game state
   ========================================================= */
int score        = 0;
int live         = 3;
int special      = 2;
int flush        = 0;
int gameOver     = 0;
int totalSpawned = 0;   /* total enemy spawns (for mod N)  */
int frameCount   = 0;   /* frame counter for spawn timing  */
int visualmode   = 0;         /* 2D or 3D*/
int extendstate  = 0;
#define SPAWN_INTERVAL 120  /* spawn attempt every N frames    */

/* =========================================================
   Enemy definition
   ========================================================= */
#define EB_MAX 10   /* max enemy bullets on screen at once */

typedef struct {
    float x, y;         /* position                        */
    int   active;       /* 1 = alive                       */
} Enemy;

typedef struct {
    float x, y;         /* position                        */
    float vx, vy;       /* velocity                        */
    int   active;
} EnemyBullet;

Enemy       enemies[ENEMY_LIMIT];
EnemyBullet eBullets[EB_MAX];

int enemyCount = 0;     /* current live enemy count        */

/* =========================================================
   Utilities
   ========================================================= */
float frand(float lo, float hi)
{
    return lo + (hi - lo) * ((float)rand() / (float)RAND_MAX);
}

int activeEnemyCount(void)
{
    int i, c = 0;
    for (i = 0; i < ENEMY_LIMIT; i++)
        if (enemies[i].active) c++;
    return c;
}

/* =========================================================
   Spawn an enemy bullet from enemy (ex, ey)
   type 0 : straight down
   type 1 : aimed at player
   ========================================================= */
void spawnEnemyBullet(float ex, float ey, int type)
{
    int i;
    float speed = 0.007f;
    for (i = 0; i < EB_MAX; i++) {
        if (!eBullets[i].active) {
            eBullets[i].x = ex;
            eBullets[i].y = ey;
            if (type == 0) {
                eBullets[i].vx = 0.0f;
                eBullets[i].vy = -speed;
            } else {
                /* aimed shot: direction toward player */
                float dx = m - ex;
                float dy = n - ey;
                float len = sqrtf(dx*dx + dy*dy);
                speed = 0.002f;
                if (len < 0.001f) len = 0.001f;
                eBullets[i].vx = speed * dx / len;
                eBullets[i].vy = speed * dy / len;
            }
            eBullets[i].active = 1;
            break;
        }
    }
}

/* =========================================================
   Spawn one enemy at random X, off-screen top
   ========================================================= */
void spawnEnemy(void)
{
    int i;
    if (activeEnemyCount() >= ENEMY_LIMIT) return;
    for (i = 0; i < ENEMY_LIMIT; i++) {
        if (!enemies[i].active) {
            enemies[i].x      = frand(-4.0f, 4.0f);
            enemies[i].y      = 8.0f;   /* off-screen top */
            enemies[i].active = 1;
            enemyCount++;
            totalSpawned++;
            break;
        }
    }
}

void ClearEnemy(int x){

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    flush = 120;

    for (int i = 0; i < EB_MAX; i++) {

        eBullets[i].active = 0;

    }

    /* ---- hit: enemy body vs player ---- */
    for (int i = 0; i < ENEMY_LIMIT; i++) {
        if (!enemies[i].active) continue;

            enemies[i].active = 0;
            enemyCount--;
            
            if(x == 0){
                score += SCORE_PER_KILL;
            }

    }
    
    glClearColor(0.3f, 0.3f, 1.0f, 1.0f);

    if(x == -1){
        soundHitPlayer(); 
        live--;
        special = 2;
        if (live <= 0) gameOver = 1;
    }

    else{
        soundSpecial();
    }


}



/* =========================================================
   2-D distance helper
   ========================================================= */
float dist2(float ax, float ay, float bx, float by)
{
    float dx = ax - bx, dy = ay - by;
    return sqrtf(dx*dx + dy*dy);
}

/* =========================================================
   Draw 2-D text (screen-space overlay)
   ========================================================= */
void drawString(float x, float y, const char *str)
{
    int i;
    glRasterPos2f(x, y);
    for (i = 0; str[i]; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
}

/* =========================================================
   display()
   ========================================================= */
void display(void)
{
    int i;
    char buf[64];

    if(flush > 0 ){
        flush--;

        if(flush % 10 >= 0 && flush % 10 < 5){
            glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
        }

        else{
            glClearColor(0.3f, 0.3f, 1.0f, 1.0f);
        }
        
    }

    else{
        glClearColor(0.3f, 0.3f, 1.0f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glPushMatrix();

      if(visualmode == 0){ //2D
        gluLookAt(0.0, 0.0, distance,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0);
      }

      else{ //3D
        gluLookAt(m, n-12.0, 4.0,  m, n+4.0, 0.0,  0.0, 0.0, 1.0);
      }

      /* ---- player hit-sphere (wire) ---- */
      glPushMatrix();
        glTranslatef(m, n, 0.3f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glutWireSphere(PLAYER_RADIUS, 32, 32);
      glPopMatrix();

      /* ---- rocket body ---- */
      glPushMatrix();
        glEnable(GL_DEPTH_TEST);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR,  specular);
        glMaterialfv(GL_FRONT, GL_AMBIENT,   ambient);
        glMaterialf (GL_FRONT, GL_SHININESS, shininess);
        glEnable(GL_LIGHTING);

        glTranslatef(m, n, -0.6f);
        glRotatef(theta, 0.0f+(0*m), 3.0f+(100*m), 0.0f);
        glScalef(0.5f, 0.5f, 0.5f);

        

        glPushMatrix();
          glTranslatef(0.0f, 2.0f, 0.0f);
          glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
          glutSolidCone(1.0, 2.0, 12, 3);
        glPopMatrix();
        glPushMatrix();
          glTranslatef(0.0f, 1.0f, 0.0f);
          mySolidCylinder(1.0, 2.0, 12);
        glPopMatrix();
        glPushMatrix();
          glTranslatef(0.0f, -1.0f, 0.0f);
          mySolidCylinder(1.0, 2.0, 12);
        glPopMatrix();

        glEnable(GL_NORMALIZE);
        glPushMatrix();
          GLfloat red[] = { 1.0f, 0.1f, 0.1f, 1.0f };
          glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
          glColor3f(1.0f, 0.0f, 0.0f);
          glTranslatef(0.0f, -2.5f, 0.0f);
          glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
          glScalef(0.5f, 0.5f, 0.5f);
          glutSolidCone(1.0, 2.0, 12, 3);
          glColor3f(0.0f, 0.0f, 1.0f);
        glPopMatrix();
        glPushMatrix(); /* right wing */
          glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
          glTranslatef(1.5f, -1.0f, 0.0f);
          glRotatef(180.0f, 0.0f, 0.0f, 0.0f);
          glScalef(1.0f, 2.0f, 0.5f);
          glutSolidCube(1.0);
        glPopMatrix();
        glPushMatrix(); /* left wing */
          glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
          glTranslatef(-1.5f, -1.0f, 0.0f);
          glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
          glScalef(1.0f, 2.0f, 0.5f);
          glutSolidCube(1.0);
        glPopMatrix();
        
        glDisable(GL_LIGHTING);

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
      glPopMatrix();

      /* ---- player bullet ---- */
      if (size > 0) {
        glPushMatrix();
          glTranslatef(m, n + py, 0.0f);
          glColor3f(1.0f, 0.0f, 0.0f);
          glutWireSphere(size, 10, 10);
        glPopMatrix();
      }

      /* ---- enemies ---- */
      glEnable(GL_DEPTH_TEST);
      for (i = 0; i < ENEMY_LIMIT; i++) {
        if (!enemies[i].active) continue;
        glPushMatrix();
          glColor3f(1.0f, 0.2f, 0.2f);
          glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
          glEnable(GL_LIGHTING);
          glTranslatef(enemies[i].x, enemies[i].y, 0.0f);
          glutSolidSphere(ENEMY_RADIUS, 16, 16);
          glDisable(GL_LIGHTING);
        glPopMatrix();
      }

      /* ---- enemy bullets ---- */
      glColor3f(1.0f, 1.0f, 0.0f);
      for (i = 0; i < EB_MAX; i++) {
        if (!eBullets[i].active) continue;
        glPushMatrix();
          glTranslatef(eBullets[i].x, eBullets[i].y, 0.0f);
          glutWireSphere(0.15f, 8, 8);
        glPopMatrix();
      }
      glDisable(GL_DEPTH_TEST);

    glPopMatrix(); /* end 3-D scene */

    /* ---- 2-D overlay (score / gameover) ---- */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
      glLoadIdentity();
      gluOrtho2D(0.0, 500.0, 0.0, 500.0);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
        glLoadIdentity();
        glColor3f(1.0f, 1.0f, 0.0f);
        sprintf(buf, "SCORE: %d", score);
        drawString(10.0f, 475.0f, buf);
        sprintf(buf, "LIVES: %d", live);
        drawString(390.0f, 475.0f, buf);
        sprintf(buf, "SPECIAL: %d", special);
        drawString(390.0f, 450.0f, buf);
        sprintf(buf, "SCORE: %d", score);

        if (gameOver == 1) {
            glColor3f(1.0f, 0.0f, 0.0f);
            drawString(180.0f, 270.0f, "GAME OVER");
            drawString(175.0f, 250.0f, buf);
            drawString(150.0f, 220.0f, "Press R key...(restart)");
        }

        else if(gameOver == 2){
            glColor3f(0.0f, 1.0f, 0.0f);
            drawString(175.0f, 270.0f, "YOU WIN!");
            //drawString(175.0f, 250.0f, buf);
            drawString(150.0f, 220.0f, "Press R key...(restart)");

        }
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

/* =========================================================
   idle()
   ========================================================= */
void idle(void)
{
    int i, j;

    if (gameOver) {
        glutPostRedisplay();
        return;
    }

    /* ---- player movement inside frame---- */
    if (keyState[GLUT_KEY_LEFT  + 256] && m > -5.03f){
        m -= 0.005f;
    }

    if (keyState[GLUT_KEY_RIGHT + 256] && m < 5.03f){
        m += 0.005f;
    }

    if (keyState[GLUT_KEY_UP    + 256] && n < 5.03f) {
        n += 0.005f;
    }

    if (keyState[GLUT_KEY_DOWN  + 256] && n > -5.03f) {
        n -= 0.005f;
    }

    /* ---- player bullet ---- */
    if (keyState[(int)'z'] && size <= 0.0f) {
        py   = 0.0f;
        size = 0.2f;
        //soundShoot();
    }
    if (size > 0.0f) {
        py += 0.15f;
        
        if (py > 20.0f) { size = 0.0f; py = 0.0f; }
    }

    /* ---- enemy spawn ---- */
    frameCount++;
    if (frameCount >= SPAWN_INTERVAL) {
        frameCount = 0;
        spawnEnemy();
    }

    /* ---- enemy movement + their shooting ---- */
    for (i = 0; i < ENEMY_LIMIT; i++) {
        if (!enemies[i].active) continue;
        enemies[i].y -= 0.006f;  /* descend */

        /* shoot every ~60 frames (staggered by index) */
        if ((frameCount + i * 30) % 60 == 0) {
            int btype = (totalSpawned % BULLET_MOD == 0) ? 0 : 1;
            spawnEnemyBullet(enemies[i].x, enemies[i].y, btype);
        }

        /* despawn if off screen bottom */
        if (enemies[i].y < -5.7f) {
            enemies[i].active = 0;
            enemyCount--;
        }
    }

    /* ---- enemy bullet movement ---- */
    for (i = 0; i < EB_MAX; i++) {
        if (!eBullets[i].active) continue;
        eBullets[i].x += eBullets[i].vx;
        eBullets[i].y += eBullets[i].vy;
        if (eBullets[i].y < -10.0f || eBullets[i].y > 12.0f ||
            eBullets[i].x < -8.0f  || eBullets[i].x >  8.0f) {
            eBullets[i].active = 0;
        }
    }

    /* ---- hit: player bullet vs enemies ---- */
    if (size > 0.0f) {
        float bx = m, by = n + py;
        for (i = 0; i < ENEMY_LIMIT; i++) {
            if (!enemies[i].active) continue;
            if (dist2(bx, by, enemies[i].x, enemies[i].y)
                    < ENEMY_RADIUS + size) {
                enemies[i].active = 0;
                enemyCount--;
                score += SCORE_PER_KILL;
                size = 0.0f; py = 0.0f;  /* consume bullet */
                break;
            }
        }
    }

    /* ---- hit: enemy bullets vs player ---- */
    for (i = 0; i < EB_MAX; i++) {
        if (!eBullets[i].active) continue;
        if (dist2(eBullets[i].x, eBullets[i].y, m, n)
                < PLAYER_RADIUS + 0.15f) {
            ClearEnemy(-1);
            break;
        }
    }

    /* ---- hit: enemy body vs player ---- */
    for (i = 0; i < ENEMY_LIMIT; i++) {
        if (!enemies[i].active) continue;
        if (dist2(enemies[i].x, enemies[i].y, m, n)
                < PLAYER_RADIUS + ENEMY_RADIUS) {
            ClearEnemy(-1);
            break;
        }
    }

    if(score % 10000 == 0 && score != 0){
        
        if(extendstate == 0){
            extendstate = 1;
            soundExtend();
            live++;
        }
        

    }

    else if(score % 10000 != 0){
        extendstate = 0;
    }

    if(score >= 50000){
        gameOver = 2;
    }

    glutPostRedisplay();
}

/* =========================================================
   Callbacks
   ========================================================= */
void myReshape(int width, int height)
{
    glutPostRedisplay();
}

void myMotion(int x, int y)   { glutPostRedisplay(); }

void myMouse(int button, int state, int x, int y)
{
    glutPostRedisplay();
}

void mySkey(int key, int x, int y)
{
    if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) shiftState = 1;
    else                                         shiftState = 0;
    if (key < 256) keyState[key + 256] = 1;
    glutPostRedisplay();
}

void mySkeyUp(int key, int x, int y)
{
    if (!(glutGetModifiers() & GLUT_ACTIVE_SHIFT)) shiftState = 0;
    if (key < 256) keyState[key + 256] = 0;
    glutPostRedisplay();
}

void myKbd(unsigned char key, int x, int y)
{
    if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) shiftState = 1;
    else                                         shiftState = 0;

    switch (key) {
    case 'z':
        keyState[(int)key] = 1;
        
        break;
    
    case 'x':
        if(special > 0){
            ClearEnemy(0);
            special--;
        }
        break;
        
    case 'v':
        visualmode++;
        if (visualmode > 1) visualmode = 0;
        break;

    case 'r':   /* restart */
        if (gameOver == 1 || gameOver == 2) {
            int i;
            gameOver = score = totalSpawned = frameCount = enemyCount = 0;
            live = 3;
            m = n = py = size = 0.0f;
            for (i = 0; i < ENEMY_LIMIT; i++) enemies[i].active = 0;
            for (i = 0; i < EB_MAX;      i++) eBullets[i].active = 0;
        }
        break;
    
    case '0':
        if(score >= 10000){
            score -= 10000;
            soundExtend();
            live++;
        }
        break;
    
    case '9':
        if(score >= 5000){
            score -= 5000;
            special++;
        }
        break;

    case 'p':
        score += 5000;
        break;
    case KEY_ESC:
        exit(0);
    }
    glutPostRedisplay();
}

void myKbdUp(unsigned char key, int x, int y)
{
    keyState[(int)key] = 0;
    if (!(glutGetModifiers() & GLUT_ACTIVE_SHIFT)) shiftState = 0;
}

/* =========================================================
   Init / main
   ========================================================= */
void myInit(char *progname)
{
    int i;
    int width = 700, height = 700;
    float aspect = (float)width / (float)height;

    glutInitWindowPosition(0, 0);
    glutInitWindowSize(width, height);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow(progname);
    glClearColor(0.3f, 0.3f, 1.0f, 1.0f);  /* dark blue background */

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, aspect, 1.0, 50.0);
    glMatrixMode(GL_MODELVIEW);

    /* init enemy / bullet arrays */
    for (i = 0; i < ENEMY_LIMIT; i++) enemies[i].active  = 0;
    for (i = 0; i < EB_MAX;      i++) eBullets[i].active = 0;

    /* ---- lighting setup (reuse global material arrays) ---- */
    {
        GLfloat light_pos[] = { 5.0f, 10.0f, 10.0f, 1.0f }; /* positional light */
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
        glLightfv(GL_LIGHT0, GL_AMBIENT,  ambient);
        glEnable(GL_LIGHT0);
    }

    soundInit();

    srand(42);
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    myInit(argv[0]);
    glutDisplayFunc(display);
    glutKeyboardFunc(myKbd);
    glutKeyboardUpFunc(myKbdUp);
    glutSpecialFunc(mySkey);
    glutSpecialUpFunc(mySkeyUp);
    glutMouseFunc(myMouse);
    glutMotionFunc(myMotion);
    glutReshapeFunc(myReshape);
    glutIdleFunc(idle);
    glutMainLoop();
    atexit(soundClose);
    return 0;
}