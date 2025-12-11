#ifndef ENTITIES_H
#define ENTITIES_H

#include <list>
#include <memory>
#include <vector>
#include <cstdlib>
#include <ncurses.h>

const int SCREEN_WIDTH = 40; // размер экрана
const int SCREEN_HEIGHT = 20;

class Entity;
class Player; 

using EntityList = std::list<std::unique_ptr<Entity>>; // тип для списка сущностей

class Entity { // абстрактный класс сущностей (является родителем)
protected:
    // координаты
    int x;
    int y;
    // отрисовка
    char symbol;
    int speed;
    int directionX; // движение по оси Ох

public:
    Entity(int x, int y, char sym, int spd, int dir);
    virtual ~Entity() = default; // ДЕСТРУКТОР ДЛЯ УДАЛЕНИЯ НАСЛЕДНИКОВ
    
    virtual void update() = 0;

    // геттеры для доступа к приватным полям
    int getX() const { return x; } 
    int getY() const { return y; }
    char getSymbol() const { return symbol; }
    void moveX(int dx);
    void moveY(int dy);
};

class Bullet : public Entity {  // класс снаряда
private:
    bool isPlayers; // флаг, означ кому принадлежит пуля

public:
    Bullet(int x, int y, bool isPlayerBullet);
    void update() override; // движение пули
    bool isPlayerBullet() const;
};

enum class InputCommand { None, Left, Right, Shoot, Quit }; // команды ввода

class Player : public Entity { // класс игрока
private:
    int lives;
    int maxLives; 
    int score;

public:
    Player(int x, int y, int initialLives);  // конструктор с начальными жизнями
    void update() override; // состояние игрока

    void processInput(InputCommand cmd);
    void loseLife();
    void addScore(int points);

    // геттеры для доступа к состоянию игрока
    int getLives() const;
    int getMaxLives() const;
    int getScore() const;
};

class Invader : public Entity { // класс пришельцев, абстрактный для всех типов врага
protected:
    int invaderID;
    int moveCounter; // счетчик для частоты движения

public:
    Invader(int x, int y, char sym, int spd, int dir, int id);

    // Виртуальный метод для определения, должен ли пришелец стрелять
    virtual bool shouldShoot() const { return false; } 
    virtual void onDeath(EntityList& entities);
    virtual void specialMove(bool& shouldChangeDirection) = 0;

    void update() override; // логика движения по сетке
};

class NormalInvader : public Invader { // обычный пришелец
public:
    NormalInvader(int x, int y);
    void specialMove(bool& shouldChangeDirection) override;
};

class ShootingInvader : public Invader { // стреляющий пришелец
public:
    ShootingInvader(int x, int y);
    void specialMove(bool& shouldChangeDirection) override;
    bool shouldShoot() const override;
};

class FastInvader : public Invader { // быстрый пришелец
private:
    int dropCounter; // счетчик для отслеживания ускорения
public:
    FastInvader(int x, int y);
    void specialMove(bool& shouldChangeDirection) override;
};

class SlowInvader : public Invader { // медленный пришелец
public:
    SlowInvader(int x, int y);
    void specialMove(bool& shouldChangeDirection) override;
    void onDeath(EntityList& entities) override;
};

#endif // ENTITIES_H