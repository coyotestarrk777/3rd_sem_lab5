#ifndef GAME_H
#define GAME_H

#include "entities.h"
#include <iostream>
#include <list>
#include <memory>
#include <chrono>
#include <thread>
#include <iomanip>
#include <ncurses.h> 

enum class Difficulty {
    EASY = 0,
    NORMAL = 1,
    HARD = 2
};

class Game {
private:
    bool gameOn;
    Player player;
    EntityList entities; 
    
    Difficulty currentDifficulty;

    InputCommand currentInput = InputCommand::None;

    void initNcurses();
    void closeNcurses();
    void pauseUntilKey();

    void input();
    void draw();
    void update();

    void handleCollisions();
    void handleInvaderActions();
    void cleanupEntities();
    void checkWinCondition();
    
    void initializeInvaders();

public:
    Game(Difficulty difficulty); 
    void run();
};

#endif // GAME_H