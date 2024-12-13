#include <iostream>
#include <sstream>
#include <conio.h>
#include <stdio.h>
#include <windows.h>
#include <chrono>
using namespace std::chrono;
using namespace std;
struct Point {
    int X;
    int Y;

    Point() : X(0), Y(0) {}
    Point(int x, int y) : X(x), Y(y) {}
};

struct Node {
private:
    Point pos;
public:
    int X() const { return pos.X; }
    int Y() const { return pos.Y; }
    void X(int x) { pos.X = x; }
    void Y(int y) { pos.Y = y; }

    Node* next;

    Node() : pos(Point(0, 0)), next(nullptr) {}
    Node(int x, int y) : pos(Point(x, y)), next(nullptr) {}
};

enum class Direction {
    Up,
    Down,
    Left,
    Right
};

class Snake {
private:
    Node body; 
    Direction dir = Direction::Up;

    Node* getLast() const {
        const Node* cur = &body;
        while (cur->next) {
            cur = cur->next;
        }
        return const_cast<Node*>(cur);
    }
public:
    Snake(int x, int y) : body(Node(x, y)) {
        Add();
    }
    ~Snake() {
        Node* cur = body.next;
        while (cur) {
            Node* toDelete = cur;
            cur = cur->next;
            delete toDelete;
        }
    }
    void Add() {
        Node* tail = getLast();
        tail->next = new Node(tail->X(), tail->Y());
    }
    void Direction(Direction newDirection) {
        if ((dir == Direction::Up && newDirection != Direction::Down) ||
            (dir == Direction::Down && newDirection != Direction::Up) ||
            (dir == Direction::Left && newDirection != Direction::Right) ||
            (dir == Direction::Right && newDirection != Direction::Left)) {
            dir = newDirection;
        }
    }
    void Move(int fieldWidth, int fieldHeight) {
        int prevX = body.X();
        int prevY = body.Y();

        int newX = prevX;
        int newY = prevY;

        switch (dir) {
        case Direction::Up:
            newY -= 1;
            break;
        case Direction::Down:
            newY += 1;
            break;
        case Direction::Left:
            newX -= 1;
            break;
        case Direction::Right:
            newX += 1;
            break;
        }

        if (newX < 0) newX = fieldWidth - 1;
        if (newY < 0) newY = fieldHeight - 1;
        if (newX >= fieldWidth) newX = 0;
        if (newY >= fieldHeight) newY = 0;

        body.X(newX);
        body.Y(newY);

        Node* cur = body.next;
        while (cur) {
            int tempX = cur->X();
            int tempY = cur->Y();

            cur->X(prevX);
            cur->Y(prevY);

            prevX = tempX;
            prevY = tempY;

            cur = cur->next;
        }
    }
    bool IsSelfColliding() const {
        const int headX = body.X();
        const int headY = body.Y();

        const Node* cur = body.next;
        while (cur) {
            if (cur->X() == headX && cur->Y() == headY) {
                return true;
            }
            cur = cur->next;
        }
        return false;
    }
    void Draw(char** field) {
        field[body.Y()][body.X()] = '0'; 

        Node* cur = body.next;
        while (cur) {
            field[cur->Y()][cur->X()] = 'o';
            cur = cur->next;
        }
    }
    bool IsCollidingWithApple(const Point& apple) {
        return body.X() == apple.X && body.Y() == apple.Y;
    }
};

class Game {
private:
    Snake snake;
    Point apple;
    int fieldWidth;
    int fieldHeight;
    int speed;
    char** field;
    steady_clock::time_point lastMoveTime;

public:
    bool active = true;
    int count = 2;

    Game(int width, int height, int gameSpeed)
        : fieldWidth(width), fieldHeight(height), speed(gameSpeed), snake(width / 2, height / 2) {
        srand(time(NULL));
        field = new char* [fieldHeight];
        for (int i = 0; i < fieldHeight; ++i) {
            field[i] = new char[fieldWidth];
        }
        apple.X = width / 2 + width / 4;
        apple.Y = height / 2;
        lastMoveTime = steady_clock::now();
    }

    ~Game() {
        for (int i = 0; i < fieldHeight; ++i) {
            delete[] field[i];
        }
        delete[] field;
    }

    void GenerateApple() {
        int newX, newY;
        do {
            newX = rand() % fieldWidth;
            newY = rand() % fieldHeight;
        } while (field[newY][newX] != ' ');

        apple.X = newX;
        apple.Y = newY;
    }

    void Update() {
        steady_clock::time_point now = steady_clock::now();
        auto elapsed = duration_cast<milliseconds>(now - lastMoveTime).count();
        if (elapsed >= speed) {
            lastMoveTime = now;
            for (int y = 0; y < fieldHeight; ++y) {
                for (int x = 0; x < fieldWidth; ++x) {
                    field[y][x] = ' ';
                }
            }
            snake.Move(fieldWidth, fieldHeight);
            snake.Draw(field);
            field[apple.Y][apple.X] = '*';
            if (snake.IsSelfColliding()) active = false;
            if (snake.IsCollidingWithApple(apple)) {
                snake.Add();
                count++;
                GenerateApple();
            }
        }
    }

    void Draw() {
        std::ostringstream buffer;

        for (int y = 0; y < fieldHeight; ++y) {
            for (int x = 0; x < fieldWidth; ++x) {
                if (field[y][x] == '0') {
                    // Голова змейки - темно-синий фон
                    buffer << "\033[44m " << "\033[0m";
                }
                else if (field[y][x] == 'o') {
                    // Тело змейки - ярко-синий фон
                    buffer << "\033[104m " << "\033[0m";
                }
                else if (field[y][x] == '*') {
                    // Яблоко - красный фон
                    buffer << "\033[41m " << "\033[0m";
                }
                else {
                    // Пустое поле - зеленый фон
                    buffer << "\033[42m " << "\033[0m";
                }
            }
            buffer << '\n';
        }
        buffer << to_string(count);

        std::cout << "\033[H" << buffer.str();
    }

    void HandleInput() {
        if (_kbhit()) {
            char key = _getch();
            switch (key) {
            case 'w': case 'W': snake.Direction(Direction::Up); break;
            case 's': case 'S': snake.Direction(Direction::Down); break;
            case 'a': case 'A': snake.Direction(Direction::Left); break;
            case 'd': case 'D': snake.Direction(Direction::Right); break;
            case 72: snake.Direction(Direction::Up); break;   
            case 80: snake.Direction(Direction::Down); break;  
            case 75: snake.Direction(Direction::Left); break;  
            case 77: snake.Direction(Direction::Right); break;
            }
        }
    }
};

int main()
{
    int speed = 80;
    Game game(40, 20, speed);
    
    while (game.active) {
        game.HandleInput();
        game.Update();
        game.Draw();
        Sleep(speed);
    }

    return 0;
}
