// risk.cpp
#include <GL/glut.h>
#include <cstdio>
#include <string>
#include <cmath>
#include "game.h"

Game game;

int windowWidth = 800;
int windowHeight = 600;

// Convert screen (mouse) -> world coords (-1..1)
void screenToWorld(int sx, int sy, float &wx, float &wy) {
    wx =  2.0f * ( (float)sx / (float)windowWidth  ) - 1.0f;
    wy = -2.0f * ( (float)sy / (float)windowHeight ) + 1.0f;
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
    // color by owner
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

    glColor3f(baseR, baseG, baseB);
    glBegin(GL_POLYGON);
    for (size_t i=0;i<t.polyX.size();i++){
        glVertex2f(t.polyX[i], t.polyY[i]);
    }
    glEnd();

    // outline
    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
    for (size_t i=0;i<t.polyX.size();i++){
        glVertex2f(t.polyX[i], t.polyY[i]);
    }
    glEnd();
}

// render
void displayCB(){
    glClear(GL_COLOR_BUFFER_BIT);

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
    std::string status;
    if (game.gameOver) {
        status = "GAME OVER! Winner: Player " + std::to_string(game.winner+1);
    } else {
        status = "Player " + std::to_string(game.currentPlayer+1)
               + " | Phase: " + game.phaseName();
        if (game.phase == PHASE_REINFORCE) {
            status += " | Reinforce left: " + std::to_string(game.reinforcementsLeft);
        }
        if (game.phase == PHASE_ATTACK) {
            status += " | Click YOUR territory, then ENEMY";
        }
        if (game.phase == PHASE_FORTIFY) {
            status += " | Click YOUR source, then YOUR neighbor (once)";
        }
        status += " | ENTER=NextPhase";
    }

    drawText(-0.95f, -0.9f, status, 0,0,0);

    glutSwapBuffers();
}

void reshapeCB(int w, int h){
    windowWidth = w;
    windowHeight = h;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // We’ll just use orthographic -1..1
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
    glutPostRedisplay();
}

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Mini Risk (2-Player)");

    glClearColor(0.9f,0.9f,0.9f,1.0f);

    glutDisplayFunc(displayCB);
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyCB);
    glutMouseFunc(mouseCB);

    glutMainLoop();
    return 0;
}
