#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

using namespace std;

bool gameOver;
const int width = 40;
const int height = 20;
int x, y, fruitX, fruitY, score;
vector<pair<int, int>> tail;
int nTail;
enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
eDirection dir;
bool paused = false;

char getChar() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0) perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0) perror("tcsetattr ~ICANON");
    return (buf);
}

bool kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }

    return false;
}

void Setup() {
    srand(time(0));
    gameOver = false;
    paused = false;
    dir = STOP;
    x = width / 2;
    y = height / 2;
    fruitX = rand() % width;
    fruitY = rand() % height;
    score = 0;
    nTail = 0;
    tail.clear();
}

void DrawBorder() {
    cout << "\033[32m";
    for (int i = 0; i < width + 2; i++)
        cout << "#";
    cout << "\033[0m\n";
}

void Draw() {
    cout << "\033[H\033[J";
    DrawBorder();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (j == 0) {
                cout << "\033[32m#\033[0m";
            }
            if (i == y && j == x) {
                cout << "\033[31mO\033[0m";
            } else if (i == fruitY && j == fruitX) {
                cout << "\033[33mF\033[0m";
            } else {
                bool print = false;
                for (int k = 0; k < nTail; k++) {
                    if (tail[k].first == j && tail[k].second == i) {
                        cout << "\033[36mo\033[0m";
                        print = true;
                    }
                }
                if (!print) {
                    cout << " ";
                }
            }
            if (j == width - 1) {
                cout << "\033[32m#\033[0m";
            }
        }
        cout << endl;
    }

    DrawBorder();
    cout << "Score: " << score << endl;
}

void Input() {
    if (kbhit()) {
        switch (getChar()) {
        case 'a':
            dir = LEFT;
            break;
        case 'd':
            dir = RIGHT;
            break;
        case 'w':
            dir = UP;
            break;
        case 's':
            dir = DOWN;
            break;
        case 'x':
            gameOver = true;
            break;
        case 'p':
            paused = !paused;
            break;
        }
    }
}

void Logic() {
    if (paused) return;

    tail.insert(tail.begin(), { x, y });
    if (nTail < tail.size()) tail.pop_back();

    switch (dir) {
    case LEFT:
        x--;
        break;
    case RIGHT:
        x++;
        break;
    case UP:
        y--;
        break;
    case DOWN:
        y++;
        break;
    default:
        break;
    }

    if (x >= width) x = 0; else if (x < 0) x = width - 1;
    if (y >= height) y = 0; else if (y < 0) y = height - 1;

    for (int i = 0; i < nTail; i++)
        if (tail[i].first == x && tail[i].second == y)
            gameOver = true;

    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = rand() % width;
        fruitY = rand() % height;
        nTail++;
    }
}

void GameOverScreen() {
    cout << "\033[H\033[J";
    cout << "\033[31mGAME OVER\033[0m" << endl;
    cout << "Final Score: " << score << endl;
    cout << "Press 'r' to restart or 'q' to quit" << endl;
    while (true) {
        if (kbhit()) {
            char c = getChar();
            if (c == 'r') {
                Setup();
                break;
            } else if (c == 'q') {
                gameOver = true;
                break;
            }
        }
    }
}

int main() {
    Setup();
    while (!gameOver) {
        Draw();
        Input();
        Logic();
        usleep(100000);
    }
    GameOverScreen();
    return 0;
}
