#include "entities.h"
#include <iostream>
#include <algorithm>
#include <cmath>

Entity::Entity(int x, int y, char sym, int spd, int dir) // инициализация пролей
    : x(x), y(y), symbol(sym), speed(spd), directionX(dir) {}

void Entity::moveX(int dx) { x += dx; }
void Entity::moveY(int dy) { y += dy; }

Bullet::Bullet(int x, int y, bool isPlayerBullet) // конструктор пули
    : Entity(x, y, isPlayerBullet ? '|' : '*', isPlayerBullet ? -1 : 1, 0), // -1 вверх для игрока, 1 вниз для врага
      isPlayers(isPlayerBullet) {} // инициализация принадлежности

void Bullet::update() { // обновление состояния
    y += speed;
}

// Метод для определения принадлежности пули
bool Bullet::isPlayerBullet() const { return isPlayers; }

// Конструктор игрока
Player::Player(int x, int y, int initialLives)
    : Entity(x, y, 'A', 1, 0), lives(initialLives), maxLives(initialLives), score(0) {} 

void Player::update() {} // обновление состояния

// Обработка ввода пользователя
void Player::processInput(InputCommand cmd) { 
    if (cmd == InputCommand::Left && x > 0) {
        x -= speed;
    } else if (cmd == InputCommand::Right && x < SCREEN_WIDTH - 1) {
        x += speed;
    }
}

void Player::loseLife() { // уменьшение жизней
    if (lives > 0) {
        lives--;
    }
}

void Player::addScore(int points) {  // увеличение счета игрока
    score += points;
}

//  Геттеры для доступа к состоянию игрока
int Player::getLives() const { return lives; }
int Player::getMaxLives() const { return maxLives; }
int Player::getScore() const { return score; }

// Конструктор пришельца
Invader::Invader(int x, int y, char sym, int spd, int dir, int id)
    : Entity(x, y, sym, spd, dir), invaderID(id), moveCounter(0) {} // Вызов конструктора родительского класса

void Invader::onDeath(EntityList& entities) {}

void Invader::update() { // состояние пришельца
    moveCounter++;
    bool shouldChangeDirection = false;

    // проверяем, пришло ли время двигаться (на основе скорости)
    if (moveCounter >= speed) {
        x += directionX;
        moveCounter = 0;
        
        // проверка достижения границы экрана
        if (x >= SCREEN_WIDTH - 1 || x <= 0) {
            shouldChangeDirection = true;
        }

        // вызов метода для особого поведения конкретного типа врага
        specialMove(shouldChangeDirection); 

        if (shouldChangeDirection) { // если нужно сменить направление
            directionX *= -1;
            y += 1;
        }
    }
}

// обычный пришелец
NormalInvader::NormalInvader(int x, int y) : Invader(x, y, 'V', 1, 1, 1) {}
void NormalInvader::specialMove(bool& shouldChangeDirection) {}

// стреляющий пришелец
ShootingInvader::ShootingInvader(int x, int y) : Invader(x, y, 'O', 1, 1, 2) {}
void ShootingInvader::specialMove(bool& shouldChangeDirection) {}
bool ShootingInvader::shouldShoot() const { // способность стрелять
    return (rand() % 100 < 5); // примерно 5раз  в 100 кадров пришелец стреляет
}

// быстрый пришелец
FastInvader::FastInvader(int x, int y) : Invader(x, y, 'W', 1, 1, 4), dropCounter(0) {}
void FastInvader::specialMove(bool& shouldChangeDirection) {
    dropCounter++;
    if (dropCounter >= 3) { // каждые 3 шага дополнительно опускается на строку 
        y += 1;
        dropCounter = 0; // счетчик движения (падения)
    }
}

// медленный пришелец
SlowInvader::SlowInvader(int x, int y) : Invader(x, y, 'M', 2, 1, 3) {}
// при смерти создается быстрый на его месте
void SlowInvader::specialMove(bool& shouldChangeDirection) {}
void SlowInvader::onDeath(EntityList& entities) {
    entities.push_back(std::make_unique<FastInvader>(x, y));
}