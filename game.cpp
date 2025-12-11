#include "game.h"
#include <algorithm>
#include <list>
#include <memory>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>

void Game::initNcurses() {
    initscr(); // инициализация ncurses, создание стандартного экрана stdscr
    cbreak();  // режим, при котором символы доступны сразу после ввода (без ожидания Enter)
    noecho();     
    nodelay(stdscr, TRUE); // неблокирующий ввод 
    keypad(stdscr, TRUE);   // обработка специальных клавиш
    curs_set(0);  
}

void Game::closeNcurses() { // завершение работы с ncurses
    endwin();     
}

void Game::pauseUntilKey() { // до нажатия любой клавиши
    nodelay(stdscr, FALSE); 
    
    mvprintw(SCREEN_HEIGHT + 6, 0, "Press any key to exit...");
    refresh();
    
    getch(); // ожидание нажатия клавиши
    
    nodelay(stdscr, TRUE); 
}

// конструктор игры с указанием уровня сложности
Game::Game(Difficulty difficulty) 
    : gameOn(true), // флаг продолжения игры
     player(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 1, 3), //игрок по центру внизу
     entities(),
     currentDifficulty(difficulty)
      
{
    srand(time(0));
    
    int initialLives = 3; // настройка количества жизней в зависимости от сложности
    if (difficulty == Difficulty::EASY) initialLives = 5;
    if (difficulty == Difficulty::HARD) initialLives = 2;
    
    player = Player(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 1, initialLives); 
    
    // инициализация графиков
    initNcurses();
    initializeInvaders();
}

// первая волна пришельцев
void Game::initializeInvaders() {
    for (int y = 1; y < 5; ++y) {
        for (int x = 2; x < SCREEN_WIDTH - 2; x += 4) {
            int type = rand() % 4;
            if (type == 0) {
                // типы пришельцев
                entities.push_back(std::make_unique<NormalInvader>(x, y));
            } else if (type == 1) {
                entities.push_back(std::make_unique<ShootingInvader>(x, y));
            } else if (type == 2) {
                entities.push_back(std::make_unique<SlowInvader>(x, y));
            } else {
                entities.push_back(std::make_unique<FastInvader>(x, y));
            }
        }
    }
}

// обработка ввода пользователя
void Game::input() {
    int ch = getch(); 

    currentInput = InputCommand::None; // сброс предыдущей команды

    if (ch == KEY_LEFT) { 
        currentInput = InputCommand::Left;
    } else if (ch == KEY_RIGHT) { 
        currentInput = InputCommand::Right;
    } else if (ch == ' ') { 
        currentInput = InputCommand::Shoot;
    } else if (ch == 'q' || ch == 'Q') {
        currentInput = InputCommand::Quit;
    }
    
    // передача команды игроку для обработки движения
    player.processInput(currentInput);

    // создание пули при нажатии на пробел
    if (currentInput == InputCommand::Shoot) {
        // если true - то пуля принадлежит игроку
        entities.push_back(std::make_unique<Bullet>(player.getX(), player.getY() - 1, true));
    }
    
    if (currentInput == InputCommand::Quit) {
        gameOn = false;
    }
}

void Game::draw() { // трисовка экрана
    clear(); 

    // Создание виртуального экрана в памяти для отрисовки
    std::vector<std::vector<char>> screen(SCREEN_HEIGHT, std::vector<char>(SCREEN_WIDTH, ' '));
    
     // Отрисовка игрока, если у него есть жизни
    if (player.getLives() > 0) {
        if (player.getY() >= 0 && player.getY() < SCREEN_HEIGHT && player.getX() >= 0 && player.getX() < SCREEN_WIDTH) {
            screen[player.getY()][player.getX()] = player.getSymbol();
        }
    }

    // отрисовка сущностей
    for (const auto& entity : entities) {
        int x = entity->getX();
        int y = entity->getY();
        if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
            screen[y][x] = entity->getSymbol();
        }
    }

    // вывод виртуального экрана
    for (int i = 0; i < SCREEN_HEIGHT; ++i) {
        mvaddch(i, 0, '|'); 
        for (int j = 0; j < SCREEN_WIDTH; ++j) {
            mvaddch(i, j + 1, screen[i][j]);
        }
        mvaddch(i, SCREEN_WIDTH + 1, '|'); // правая граница
    }
    
    // нижняя граница поля
    mvprintw(SCREEN_HEIGHT + 1, 0, "%s", std::string(SCREEN_WIDTH + 2, '-').c_str());
    
    // панель информации под игровым полем
    mvprintw(SCREEN_HEIGHT + 2, 0, "Lives: %d/%d | Score: %d | Entities: %lu", 
             player.getLives(), player.getMaxLives(), player.getScore(), entities.size()); 
             
    // подсказки по управлению         
    mvprintw(SCREEN_HEIGHT + 3, 0, "Use <-, -> to move, SPACE to shoot, Q to quit.");

    // если жизни закончились
    if (player.getLives() <= 0) {
        gameOn = false;
        mvprintw(SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 - 8, "--- G A M E   O V E R ---");
    }

    refresh(); 
}

// обновление состояния
void Game::update() {
    for (auto& entity : entities) {
        entity->update();
    }

    // столкновения
    handleCollisions();

    // дейсвия пришельцев
    handleInvaderActions();

    // удаление сущностей вне экрана
    cleanupEntities();

    // проверка условия победы
    checkWinCondition();
}

// обработка столкновений
void Game::handleCollisions() {
    std::vector<Entity*> toRemove; // Список сущностей для удаления

    // Проверка столкновений пуль игрока с пришельцами
    for (auto itB = entities.begin(); itB != entities.end(); ++itB) {
        Bullet* bullet = dynamic_cast<Bullet*>(itB->get());
        if (bullet && bullet->isPlayerBullet()) {
            for (auto itI = entities.begin(); itI != entities.end(); ++itI) {
                if (itB == itI) continue; 
                
                Invader* invader = dynamic_cast<Invader*>(itI->get()); // попытка преобразования пришельца
                
                // Если это пришелец и координаты совпадают - СТОЛКНОВЕНИЕ!
                if (invader && bullet->getX() == invader->getX() && bullet->getY() == invader->getY()) {
                    
                    player.addScore(10); // очки за уничтодение
                    toRemove.push_back(bullet); // добавление на удаление
                    toRemove.push_back(invader);
                    
                    invader->onDeath(entities);
                    break; 
                }
            }
        }
        // если пуля пришельца
        else if (bullet && !bullet->isPlayerBullet()) {
            // проверка столкновения с игроком
            if (bullet->getX() == player.getX() && bullet->getY() == player.getY()) {
                player.loseLife();
                toRemove.push_back(bullet);
            }
        }
    }

    // Проверка столкновений пришельцев с игроком и выход за границы
    for (auto itI = entities.begin(); itI != entities.end(); ++itI) {
        Invader* invader = dynamic_cast<Invader*>(itI->get());
        if (invader) {
            // если достиг нижней границы
            if (invader->getY() >= SCREEN_HEIGHT - 1) { 
                player.loseLife();
                toRemove.push_back(invader);
                continue;
            }
            // если столкнулся с игроком
            if (invader->getX() == player.getX() && invader->getY() == player.getY()) {
                player.loseLife();
                toRemove.push_back(invader);
            }
        }
    }
   
    // удаление всех помеченных сущностей
    for (Entity* ptr : toRemove) {
        entities.remove_if([ptr](const std::unique_ptr<Entity>& e){
            return e.get() == ptr;
        });
    }
}

// обработка действий
void Game::handleInvaderActions() {
    for (auto& entity : entities) {
        Invader* invader = dynamic_cast<Invader*>(entity.get());
        // если пришелец стреляет, то создается пуля
        if (invader && invader->shouldShoot()) {
             entities.push_back(std::make_unique<Bullet>(invader->getX(), invader->getY() + 1, false));
        }
    }
}

// очистка сущностей вне экрана
void Game::cleanupEntities() {
    // Удаление пуль, которые вылетели за границы экрана
    entities.remove_if([](const std::unique_ptr<Entity>& e) {
        Bullet* bullet = dynamic_cast<Bullet*>(e.get());
        if (bullet) {
            return bullet->getY() < 0 || bullet->getY() >= SCREEN_HEIGHT;
        }
        return false; // не пуля - не удаляем
    });
}

// проверка условия победы
void Game::checkWinCondition() {
    bool allInvadersGone = true;
    for (const auto& entity : entities) {
        if (dynamic_cast<Invader*>(entity.get())) {
            allInvadersGone = false; // если кто-то остался
            break;
        }
    }

    if (allInvadersGone) { // если всех победили
        mvprintw(SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 - 5, "--- V I C T O R Y ---");
        gameOn = false;
    }
}

// настройка игры в зависимости от сложности уровня
void Game::run() {
    int targetFrameTimeMs = 200; 
    
    if (currentDifficulty == Difficulty::EASY) {
        targetFrameTimeMs = 250; 
    } else if (currentDifficulty == Difficulty::HARD) {
        targetFrameTimeMs = 100; 
    }

    // главный игровой цикл
    while (gameOn) {
        auto start = std::chrono::high_resolution_clock::now();
        
        input();
        update();
        draw();

        // время выаолнения кода
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Регулировка скорости игры
        if (duration.count() < targetFrameTimeMs) {
            std::this_thread::sleep_for(std::chrono::milliseconds(targetFrameTimeMs - duration.count()));
        }
    }
    
    // игра завершена
    pauseUntilKey();
    closeNcurses(); 
}