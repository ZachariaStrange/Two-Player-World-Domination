// game.cpp
#include "game.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>

Game::Game() {
    std::srand((unsigned int)std::time(nullptr));
    initSimpleMap();
    currentPlayer = 0;
    phase = PHASE_REINFORCE;
    reinforcementsLeft = 3;
    gameOver = false;
    winner = -1;
    attackSel = {};
    fortSel = {};
    fortifyDone = false;
}

void Game::initSimpleMap() {
    terrs.clear();
    terrs.resize(14);

    // helper: add a vertex from lon/lat (in degrees)
    auto addVertexDeg = [](Territory &t, float lonDeg, float latDeg) {
        float x = lonDeg / 180.0f; // [-180,180] -> [-1,1]
        float y = latDeg / 90.0f;  // [-90,90]   -> [-1,1]
        t.polyX.push_back(x);
        t.polyY.push_back(y);
    };

    // helper: set label from lon/lat
    auto setLabelDeg = [](Territory &t, float lonDeg, float latDeg) {
        t.labelX = lonDeg / 180.0f;
        t.labelY = latDeg / 90.0f;
    };

    auto setColor = [](Territory &t, float r, float g, float b) {
        t.r = r; t.g = g; t.b = b;
        t.capturing = false;
        t.animT = 0.0f;
    };

    // ================= NORTH AMERICA (0–3) =================
    // 0: NA North-West (Alaska / western Canada)
    {
        Territory &t = terrs[0];
        addVertexDeg(t, -170.0f, 72.0f);
        addVertexDeg(t, -130.0f, 72.0f);
        addVertexDeg(t, -125.0f, 60.0f);
        addVertexDeg(t, -120.0f, 50.0f);
        addVertexDeg(t, -150.0f, 55.0f);
        addVertexDeg(t, -170.0f, 60.0f);
        setLabelDeg(t, -145.0f, 62.0f);
        setColor(t, 0.9f, 0.4f, 0.4f);
    }

    // 1: NA North-East (eastern Canada / Greenland south)
    {
        Territory &t = terrs[1];
        addVertexDeg(t, -130.0f, 72.0f);
        addVertexDeg(t,  -70.0f, 72.0f);
        addVertexDeg(t,  -60.0f, 55.0f);
        addVertexDeg(t,  -65.0f, 50.0f);
        addVertexDeg(t,  -90.0f, 50.0f);
        addVertexDeg(t, -120.0f, 50.0f);
        setLabelDeg(t, -100.0f, 62.0f);
        setColor(t, 0.9f, 0.4f, 0.4f);
    }

    // 2: NA South-West (US west / Mexico west)
    {
        Territory &t = terrs[2];
        addVertexDeg(t, -130.0f, 50.0f);
        addVertexDeg(t, -100.0f, 50.0f);
        addVertexDeg(t, -100.0f, 35.0f);
        addVertexDeg(t, -90.0f, 15.0f);
        addVertexDeg(t, -110.0f, 15.0f);
        addVertexDeg(t, -120.0f, 30.0f);
        setLabelDeg(t, -115.0f, 35.0f);
        setColor(t, 0.9f, 0.4f, 0.4f);
    }

    // 3: NA South-East (US east / Mexico east / Caribbean)
    {
        Territory &t = terrs[3];
        addVertexDeg(t, -100.0f, 50.0f);
        addVertexDeg(t,  -65.0f, 50.0f);
        addVertexDeg(t,  -80.0f, 30.0f);
        addVertexDeg(t,  -65.0f, 15.0f);
        addVertexDeg(t,  -90.0f, 15.0f);
        addVertexDeg(t, -100.0f, 35.0f);
        setLabelDeg(t, -95.0f, 35.0f);
        setColor(t, 0.9f, 0.4f, 0.4f);
    }

    // ================= SOUTH AMERICA (4–6) =================
    // 4: SA North
    {
        Territory &t = terrs[4];
        addVertexDeg(t, -90.0f,  15.0f);
        addVertexDeg(t, -50.0f,  7.0f);
        addVertexDeg(t, -50.0f,   0.0f);
        addVertexDeg(t, -60.0f,  -5.0f);
        addVertexDeg(t, -75.0f,  -5.0f);
        setLabelDeg(t, -65.0f,  4.0f);
        setColor(t, 0.4f, 0.8f, 0.4f);
    }

    // 5: SA West / Central
    {
        Territory &t = terrs[5];
        addVertexDeg(t, -80.0f,   -5.0f);
        addVertexDeg(t, -60.0f,  -5.0f);
        addVertexDeg(t, -65.0f, -20.0f);
        addVertexDeg(t, -70.0f, -35.0f);
        addVertexDeg(t, -80.0f, -35.0f);
        setLabelDeg(t, -72.0f, -18.0f);
        setColor(t, 0.4f, 0.8f, 0.4f);
    }

    // 6: SA South / East
    {
        Territory &t = terrs[6];
        addVertexDeg(t, -60.0f,  -5.0f);
        addVertexDeg(t, -50.0f,   0.0f);
        addVertexDeg(t, -37.0f, -5.0f);
        addVertexDeg(t, -40.0f, -20.0f);
        addVertexDeg(t, -55.0f, -35.0f);
        addVertexDeg(t, -65.0f, -45.0f);
        addVertexDeg(t, -75.0f, -55.0f);
        setLabelDeg(t, -55.0f, -15.0f);
        setColor(t, 0.4f, 0.8f, 0.4f);
    }

    // ================= EUROPE (7–8) ======================
    // 7: Western Europe
    {
        Territory &t = terrs[7];
        addVertexDeg(t, -25.0f, 72.0f);
        addVertexDeg(t,   5.0f, 72.0f);
        addVertexDeg(t,  12.0f, 45.0f);
        addVertexDeg(t,   0.0f, 45.0f);
        addVertexDeg(t, -10.0f, 45.0f);
        addVertexDeg(t, -25.0f, 55.0f);
        setLabelDeg(t, -5.0f, 58.0f);
        setColor(t, 0.6f, 0.8f, 0.4f);
    }

    // 8: Eastern Europe
    {
        Territory &t = terrs[8];
        addVertexDeg(t,   5.0f, 72.0f);
        addVertexDeg(t,  45.0f, 72.0f);
        addVertexDeg(t,  45.0f, 35.0f);
        addVertexDeg(t,  16.0f, 35.0f);
        addVertexDeg(t,  10.0f, 55.0f);
        setLabelDeg(t, 25.0f, 58.0f);
        setColor(t, 0.6f, 0.8f, 0.4f);
    }

    // ================= AFRICA (9–10) =====================
    // 9: North Africa
    {
        Territory &t = terrs[9];
        addVertexDeg(t, -15.0f,  35.0f);
        addVertexDeg(t,  35.0f,  35.0f);
        addVertexDeg(t,  35.0f,  10.0f);
        addVertexDeg(t,  10.0f,   10.0f);
        addVertexDeg(t,  -5.0f,   10.0f);
        addVertexDeg(t, -20.0f,  10.0f);
        setLabelDeg(t,  5.0f, 20.0f);
        setColor(t, 0.9f, 0.7f, 0.3f);
    }

    // 10: South Africa
    {
        Territory &t = terrs[10];
        addVertexDeg(t, -20.0f,  10.0f);
        addVertexDeg(t,  35.0f,  10.0f);
        addVertexDeg(t,  35.0f, -35.0f);
        addVertexDeg(t,  10.0f, -35.0f);
        addVertexDeg(t,  -5.0f, -20.0f);
        addVertexDeg(t, -20.0f, -10.0f);
        setLabelDeg(t, 10.0f, -10.0f);
        setColor(t, 0.9f, 0.7f, 0.3f);
    }

    // ================= MIDDLE EAST (11) ==================
    {
        Territory &t = terrs[11];
        addVertexDeg(t,  35.0f,  35.0f);
        addVertexDeg(t,  45.0f,  35.0f);
        addVertexDeg(t,  52.5f,  25.0f);
        addVertexDeg(t,  55.0f,  20.0f);
        addVertexDeg(t,  45.0f,  12.0f);
        addVertexDeg(t,  35.0f,  25.0f);
        setLabelDeg(t, 40.0f, 25.0f);
        setColor(t, 0.9f, 0.8f, 0.4f);
    }

    // ================= ASIA (12) ========================
    {
        Territory &t = terrs[12];
        addVertexDeg(t,  45.0f, 80.0f);
        addVertexDeg(t, 180.0f, 80.0f);
        addVertexDeg(t, 180.0f,  5.0f);
        addVertexDeg(t, 120.0f,  5.0f);
        addVertexDeg(t,  80.0f,  5.0f);
        addVertexDeg(t,  55.0f, 20.0f);
        addVertexDeg(t,  45.0f, 35.0f);
        setLabelDeg(t, 100.0f, 40.0f);
        setColor(t, 0.95f, 0.8f, 0.4f);
    }

    // ================= OCEANIA (13) =====================
    {
        Territory &t = terrs[13];
        addVertexDeg(t, 110.0f,  -5.0f);
        addVertexDeg(t, 180.0f,  -5.0f);
        addVertexDeg(t, 180.0f, -48.0f);
        addVertexDeg(t, 150.0f, -48.0f);
        addVertexDeg(t, 120.0f, -35.0f);
        addVertexDeg(t, 110.0f, -20.0f);
        setLabelDeg(t, 140.0f, -25.0f);
        setColor(t, 0.6f, 0.7f, 1.0f);
    }

    // ================ ADJACENCY =========================
    // 0–3: North America
    terrs[0].neighbors = {1, 2};          // NA NW <-> NA NE, SW
    terrs[1].neighbors = {0, 3, 7};       // NA NE <-> NW, SE, W Europe (via Greenland/Iceland)
    terrs[2].neighbors = {0, 3, 4};       // NA SW <-> NW, SE, SA North
    terrs[3].neighbors = {1, 2, 4};       // NA SE <-> NE, SW, SA North

    // 4–6: South America
    terrs[4].neighbors = {2, 3, 5, 9};    // SA North <-> NA SW/SE, SA West, N Africa
    terrs[5].neighbors = {4, 6, 9};       // SA West <-> SA North/South, N Africa (Brazil-Africa vibe)
    terrs[6].neighbors = {5};             // SA South <-> SA West

    // 7–8: Europe
    terrs[7].neighbors = {1, 8, 9};       // W Europe <-> NA NE, E Europe, N Africa
    terrs[8].neighbors = {7, 9, 11, 12};  // E Europe <-> W Europe, N Africa, ME, Asia

    // 9–10: Africa
    terrs[9].neighbors  = {4, 5, 7, 8, 10, 11}; // N Africa <-> SA, Europe, S Africa, ME
    terrs[10].neighbors = {9, 11};              // S Africa <-> N Africa, ME

    // 11: Middle East
    terrs[11].neighbors = {8, 9, 10, 12};       // ME <-> E Europe, both Africas, Asia

    // 12: Asia
    terrs[12].neighbors = {1, 8, 11, 13};       // Asia <-> NA NE (Bering), E Europe, ME, Oceania

    // 13: Oceania
    terrs[13].neighbors = {12};                 // Oceania <-> Asia

    // ============== INITIAL OWNERSHIP ==================
    for (int i = 0; i < (int)terrs.size(); ++i) {
        terrs[i].armies = 3;
        terrs[i].owner  = (i % 2 == 0) ? 0 : 1;  // alternate P1/P2
    }
}



bool Game::isOwner(int terrIdx, int player) const {
    return terrs[terrIdx].owner == player;
}

bool Game::isAdjacent(int a, int b) const {
    for (int n : terrs[a].neighbors) if (n==b) return true;
    return false;
}

int Game::rollDie() const {
    return (std::rand() % 6) + 1; // 1-6
}

void Game::nextPhase() {
    if (phase == PHASE_REINFORCE) {
        // can't leave if you still have reinforcements
        if (reinforcementsLeft > 0) return;
        phase = PHASE_ATTACK;
        attackSel = {};
    } else if (phase == PHASE_ATTACK) {
        phase = PHASE_FORTIFY;
        fortSel = {};
        fortifyDone = false;
    } else {
        // FORTIFY -> end turn
        currentPlayer = 1 - currentPlayer;
        phase = PHASE_REINFORCE;
        reinforcementsLeft = 3;
        attackSel = {};
        fortSel = {};
        fortifyDone = false;
    }
    endTurnIfNeeded();
}

void Game::endTurnIfNeeded() {
    checkWin();
}

void Game::checkWin() {
    int owner0 = 0;
    int owner1 = 0;
    for (auto &t : terrs) {
        if (t.owner == 0) owner0++;
        if (t.owner == 1) owner1++;
    }
    if (owner0 == (int)terrs.size()-3) {
        gameOver = true;
        winner = 0;
    } 
    else if (owner1 == (int)terrs.size()-3) {
        gameOver = true;
        winner = 1;
}
}

// -------- Reinforce --------
bool Game::canPlaceReinforcement(int terrIdx) const {
    if (reinforcementsLeft <= 0) return false;
    return terrs[terrIdx].owner == currentPlayer;
}
void Game::placeReinforcement(int terrIdx) {
    if (!canPlaceReinforcement(terrIdx)) return;
    terrs[terrIdx].armies += 1;
    reinforcementsLeft -= 1;

    // start reinforcement flash
    terrs[terrIdx].reinforcing = true;
    terrs[terrIdx].reinfT      = 0.0f;
}

// -------- Attack ----------
bool Game::selectAttackFrom(int terrIdx) {
    // must be ours and have >1 army
    if (!isOwner(terrIdx, currentPlayer)) return false;
    if (terrs[terrIdx].armies < 2) return false;
    attackSel.fromTerr = terrIdx;
    attackSel.toTerr = -1;
    return true;
}
bool Game::selectAttackTo(int terrIdx) {
    if (attackSel.fromTerr < 0) return false;
    if (!isAdjacent(attackSel.fromTerr, terrIdx)) return false;
    if (isOwner(terrIdx, currentPlayer)) return false;
    attackSel.toTerr = terrIdx;
    resolveAttack();
    return true;
}

void Game::resolveAttack() {
    int A = attackSel.fromTerr;
    int D = attackSel.toTerr;
    if (A<0 || D<0) return;

    // how many dice?
    // attacker can roll up to 3 but not more than armies-1
    int aDice = std::min(3, terrs[A].armies - 1);
    if (aDice < 1) return; // can't attack

    // defender up to 2 but not more than armies
    int dDice = std::min(2, terrs[D].armies);
    if (dDice < 1) return;

    // roll
    std::vector<int> aRolls;
    std::vector<int> dRolls;
    for (int i=0;i<aDice;i++) aRolls.push_back(rollDie());
    for (int j=0;j<dDice;j++) dRolls.push_back(rollDie());

    std::sort(aRolls.begin(), aRolls.end(), std::greater<int>());
    std::sort(dRolls.begin(), dRolls.end(), std::greater<int>());

    int pairs = std::min((int)aRolls.size(), (int)dRolls.size());
    for (int k=0;k<pairs;k++) {
        if (aRolls[k] > dRolls[k]) {
            // defender loses 1
            terrs[D].armies -= 1;
        } else {
            // attacker loses 1
            terrs[A].armies -= 1;
        }
        if (terrs[D].armies <= 0) break;
        if (terrs[A].armies <= 1) break;
    }

   
  // capture?
    if (terrs[D].armies <= 0) {
        int oldOwner = terrs[D].owner;
        terrs[D].owner = terrs[A].owner;
        terrs[D].armies = 1;
        terrs[A].armies -= 1; // move in 1 army

        // start capture animation
        terrs[D].capturing = true;
        terrs[D].animT = 0.0f;
    }
    checkWin();

    // After battle, clear target so they can choose again
    attackSel.toTerr = -1;
}

// -------- Fortify ----------
bool Game::selectFortifyFrom(int terrIdx) {
    if (fortifyDone) return false;
    if (!isOwner(terrIdx, currentPlayer)) return false;
    if (terrs[terrIdx].armies < 2) return false;
    fortSel.fromTerr = terrIdx;
    fortSel.toTerr = -1;
    return true;
}
bool Game::selectFortifyTo(int terrIdx) {
    if (fortifyDone) return false;
    if (fortSel.fromTerr < 0) return false;
    if (!isOwner(terrIdx, currentPlayer)) return false;
    if (!isAdjacent(fortSel.fromTerr, terrIdx)) return false;
    fortSel.toTerr = terrIdx;
    doFortifyMove();
    return true;
}
void Game::doFortifyMove() {
    int A = fortSel.fromTerr;
    int B = fortSel.toTerr;
    if (A<0 || B<0) return;
    // move exactly 1 army
    terrs[A].armies -= 1;
    terrs[B].armies += 1;
    fortifyDone = true;
}

std::string Game::phaseName() const {
    switch(phase){
        case PHASE_REINFORCE: return "Reinforce";
        case PHASE_ATTACK:    return "Attack";
        case PHASE_FORTIFY:   return "Fortify";
    }
    return "???";
}
