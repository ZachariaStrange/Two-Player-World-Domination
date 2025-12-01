// risk.cpp
#include <GL/glut.h>
#include <cstdio>
#include <string>
#include <cmath>
#include "game.h"
#include "stb_image.h"
#include <GL/glu.h>


Game game;

float camX = 0.0f;   // camera pan
float camY = 0.0f;
float camZoom = 1.0f; // 1 = default, >1 zoom in, <1 zoom out

int windowWidth = 800;
int windowHeight = 600;

// NEW: world map texture
GLuint worldTex = 0;
int worldTexW = 0, worldTexH = 0;

void drawWorldMap();

void screenToWorld(int sx, int sy, float &wx, float &wy) {
    // first: screen -> NDC (-1..1)
    float ndcX =  2.0f * ( (float)sx / (float)windowWidth  ) - 1.0f;
    float ndcY = -2.0f * ( (float)sy / (float)windowHeight ) + 1.0f;

    // then invert camera: world = (NDC / zoom) + cam offset
    wx = ndcX / camZoom + camX;
    wy = ndcY / camZoom + camY;
}

// point in convex quad test (simple)
bool pointInPoly(const Territory &t, float x, float y) {
    bool inside = false;
    int n = (int)t.polyX.size();
    for (int i = 0, j = n - 1; i < n; j = i++) {
        float xi = t.polyX[i], yi = t.polyY[i];
        float xj = t.polyX[j], yj = t.polyY[j];

        // Check if the edge (xj,yj)->(xi,yi) straddles the horizontal ray at y
        bool intersect = ((yi > y) != (yj > y)) &&
                         (x < (xj - xi) * (y - yi) / (yj - yi + 1e-6f) + xi);
        if (intersect)
            inside = !inside;
    }
    return inside;
}


// draw bitmap text at world coords
void drawText(float x, float y, const std::string &s, float r, float g, float b) {
    glColor3f(r,g,b);
    glRasterPos2f(x,y);
    for (char c : s){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}



// draw a single territory polygon
void drawTerritory(const Territory &t, bool highlight=false){
    // base color by owner
    float baseR = t.r;
    float baseG = t.g;
    float baseB = t.b;
    if (t.owner == 0) { // player1 red-ish
        baseR = 1.0f; baseG *=0.4f; baseB *=0.4f;
    } else if (t.owner == 1) { // player2 blue-ish
        baseR *=0.4f; baseG *=0.4f; baseB = 1.0f;
    } else { // neutral gray
        baseR = baseG = baseB = 0.5f;
    }

    if (highlight){
        baseR = std::min(baseR+0.3f,1.0f);
        baseG = std::min(baseG+0.3f,1.0f);
        baseB = std::min(baseB+0.3f,1.0f);
    }

    float finalR = baseR;
    float finalG = baseG;
    float finalB = baseB;

  
    if (t.capturing) {
        // flash toward white as animT -> 1
        float w = t.animT; // 0..1
        if (w > 1.0f) w = 1.0f;
        finalR = (1.0f - w)*finalR + w*1.0f;
        finalG = (1.0f - w)*finalG + w*1.0f;
        finalB = (1.0f - w)*finalB + w*1.0f;
    }

    if (t.reinforcing) {
        // flash toward green as reinfT -> 1
        float w = t.reinfT; // 0..1
        if (w > 1.0f) w = 1.0f;

        const float gR = 0.0f;
        const float gG = 1.0f;
        const float gB = 0.0f;

        finalR = (1.0f - w)*finalR + w*gR;
        finalG = (1.0f - w)*finalG + w*gG;
        finalB = (1.0f - w)*finalB + w*gB;
    }

    glColor4f(finalR, finalG, finalB, 0.65f);
    glBegin(GL_POLYGON);
    for (size_t i=0;i<t.polyX.size();i++){
        glVertex2f(t.polyX[i], t.polyY[i]);
    }
    glEnd();

    // outline
    glColor4f(0,0,0, 0.9f);
    glBegin(GL_LINE_LOOP);
    for (size_t i=0;i<t.polyX.size();i++){
        glVertex2f(t.polyX[i], t.polyY[i]);
    }
    glEnd();
}


// render
void displayCB(){
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Apply camera: translate, then scale
    glTranslatef(-camX, -camY, 0.0f);
    glScalef(camZoom, camZoom, 1.0f);

    
    // --- draw world map background ---
    drawWorldMap();

    // draw territories
    for (size_t i=0;i<game.terrs.size();i++){
        bool hl = false;
        if (game.phase == PHASE_ATTACK && (int)i == game.attackSel.fromTerr) hl = true;
        if (game.phase == PHASE_FORTIFY && (int)i == game.fortSel.fromTerr) hl = true;
        drawTerritory(game.terrs[i], hl);
    }

    // draw army counts
    for (size_t i=0;i<game.terrs.size();i++){
        char buf[64];
        std::snprintf(buf, 64, "%d", game.terrs[i].armies);
        drawText(game.terrs[i].labelX,
                 game.terrs[i].labelY,
                 buf,
                 0,0,0);
    }

    // UI text
    glLoadIdentity();

    // Build the non-player part of the status string
    std::string info;
    if (game.gameOver) {
        info = "GAME OVER! Winner: Player " + std::to_string(game.winner+1);
    } else {
        info = " | Phase: " + game.phaseName();

        if (game.phase == PHASE_REINFORCE) {
            info += " | Reinforce left: " + std::to_string(game.reinforcementsLeft);
        }
        if (game.phase == PHASE_ATTACK) {
            info += " | Click YOUR territory, then ENEMY";
        }
        if (game.phase == PHASE_FORTIFY) {
            info += " | Click YOUR source, then YOUR neighbor (once)";
        }

        info += " | ENTER=NextPhase";
    }

    // Draw “Player X” separately so we can color it
    std::string playerStr = "Player " + std::to_string(game.currentPlayer + 1);

    // Choose color
    float pr, pg, pb;
    if (game.currentPlayer == 0) { 
        pr = 1.0f; pg = 0.0f; pb = 0.0f;     // red
    } else {
        pr = 0.0f; pg = 0.4f; pb = 1.0f;     // blue
    }

    // Draw colored “Player X”
    drawText(-0.95f, -0.98f, playerStr, pr, pg, pb);

    // Draw the rest (white or black)
    drawText(-0.75f, -0.98f, info, 0, 0, 0);


    glutSwapBuffers();
}

void reshapeCB(int w, int h){
    windowWidth = w;
    windowHeight = h;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1,1,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// handle clicks based on phase
void handleClick(int terrIdx){
    if (game.gameOver) return;

    if (terrIdx < 0) return;

    switch(game.phase){
        case PHASE_REINFORCE:
            game.placeReinforcement(terrIdx);
            break;

        case PHASE_ATTACK:
            // if we haven't picked from
            if (game.attackSel.fromTerr < 0) {
                game.selectAttackFrom(terrIdx);
            } else {
                game.selectAttackTo(terrIdx);
            }
            break;

        case PHASE_FORTIFY:
            if (game.fortifyDone) {
                // can't fortify again, ignore clicks
                break;
            }
            if (game.fortSel.fromTerr < 0) {
                game.selectFortifyFrom(terrIdx);
            } else {
                game.selectFortifyTo(terrIdx);
            }
            break;
    }
}

// mouse callback
void mouseCB(int button, int state, int x, int y){
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
        float wx, wy;
        screenToWorld(x,y,wx,wy);

        // find which territory
        int clicked = -1;
        for (int i=0;i<(int)game.terrs.size();i++){
            if (pointInPoly(game.terrs[i], wx, wy)){
                clicked = i;
                break;
            }
        }
        handleClick(clicked);

        glutPostRedisplay();
    }
}

// keyboard callback
void keyCB(unsigned char key, int x, int y){
    if (key == 27) { // ESC
        std::exit(0);
    }
    if (key == '\r' || key == '\n') {
        // ENTER
        game.nextPhase();
    }
      const float panStep = 0.1f / camZoom;   // pan smaller when zoomed in
    const float zoomStep = 0.1f;

    switch (key) {
        case 'w': camY += panStep; break;
        case 's': camY -= panStep; break;
        case 'a': camX -= panStep; break;
        case 'd': camX += panStep; break;

        case '+':
        case '=': // for shiftless +
            camZoom *= (1.0f + zoomStep);
            break;
        case '-':
        case '_':
            camZoom /= (1.0f + zoomStep);
            if (camZoom < 0.2f) camZoom = 0.2f;
            break;
    }
    glutPostRedisplay();
}

void updateAnimation() {
    const float dt = 0.02f;

    bool anyAnimating = false;
    for (auto &t : game.terrs) {
        if (t.capturing) {
            anyAnimating = true;
            t.animT += dt;
            if (t.animT >= 1.0f) {
                t.animT = 0.0f;
                t.capturing = false;
            }
        }

        if (t.reinforcing) {
            anyAnimating = true;
            t.reinfT += dt;
            if (t.reinfT >= 1.0f) {
                t.reinfT = 0.0f;
                t.reinforcing = false;
            }
        }
    }

    if (anyAnimating) {
        glutPostRedisplay();
    }
}

void idleCB() {
    updateAnimation();
}

bool loadWorldTexture(const char* filename) {
    int nChannels;
    unsigned char* data = stbi_load(filename, &worldTexW, &worldTexH, &nChannels, 3);
    if (!data) {
        std::fprintf(stderr, "Failed to load image %s\n", filename);
        return false;
    }

    glGenTextures(1, &worldTex);
    glBindTexture(GL_TEXTURE_2D, worldTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 worldTexW,
                 worldTexH,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 data);

    stbi_image_free(data);
    return true;
}

void drawWorldMap() {
    if (worldTex == 0) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, worldTex);

    glColor3f(1.9f, 1.9f, 1.9f); // no tint

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f,  1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f,  1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}


int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Mini Risk (2-Player)");

    glClearColor(0.9f,0.9f,0.9f,1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // load the world map texture
    if (!loadWorldTexture("atlas.jpg")) {
        std::fprintf(stderr, "WARNING: could not load atlas.jpg\n");
    }

    glutDisplayFunc(displayCB);
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyCB);
    glutMouseFunc(mouseCB);
    glutIdleFunc(idleCB);

    glutMainLoop();
    return 0;
}
