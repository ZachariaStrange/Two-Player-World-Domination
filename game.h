// game.h
#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>

struct Territory {
    int owner;         // -1 none, 0 player1, 1 player2
    int armies;
    float r, g, b;     // base color tint for rendering
    std::vector<float> polyX;
    std::vector<float> polyY;
    std::vector<int> neighbors; // indices of adjacent territories
    float labelX, labelY;       // where to draw army text

     // NEW: simple capture animation
    bool  capturing = false;
    float animT = 0.0f; // 0 -> 1 over time when captured

    bool  reinforcing = false;
    float reinfT      = 0.0f;
};

enum Phase {
    PHASE_REINFORCE = 0,
    PHASE_ATTACK    = 1,
    PHASE_FORTIFY   = 2
};

struct AttackSelection {
    int fromTerr = -1;
    int toTerr   = -1;
};

struct FortifySelection {
    int fromTerr = -1;
    int toTerr   = -1;
};

class Game {
public:
    Game();

    // core state
    std::vector<Territory> terrs;
    int currentPlayer;  // 0 or 1
    Phase phase;
    int reinforcementsLeft;
    bool gameOver;
    int winner; // -1 if none

    AttackSelection attackSel;
    FortifySelection fortSel;
    bool fortifyDone;

    // --- logic functions ---
    void initSimpleMap();
    void nextPhase();
    void endTurnIfNeeded();
    void checkWin();

    // reinforce
    bool canPlaceReinforcement(int terrIdx) const;
    void placeReinforcement(int terrIdx);

    // attack
    bool selectAttackFrom(int terrIdx);
    bool selectAttackTo(int terrIdx);
    void resolveAttack(); // rolls dice + applies result

    // fortify
    bool selectFortifyFrom(int terrIdx);
    bool selectFortifyTo(int terrIdx);
    void doFortifyMove();

    // helpers
    bool isAdjacent(int a, int b) const;
    bool isOwner(int terrIdx, int player) const;
    int  rollDie() const; // 1-6

    std::string phaseName() const;
};

#endif
