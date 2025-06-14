#include "raylib.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <locale>
#include <queue>

const int BLOCK_SIZE = 32;
const int GRID_WIDTH = 16;
const int GRID_HEIGHT = 16;
const int MARGIN = 32;
const int WINDOW_WIDTH = GRID_WIDTH * BLOCK_SIZE + MARGIN * 2;
const int WINDOW_HEIGHT = GRID_HEIGHT * BLOCK_SIZE + MARGIN * 2;
float moveTimer = 0.0f;
const float moveInterval = 0.12f;

const Color LIGHT_GREEN = {170, 215, 81, 255};
const Color DARK_GREEN = {162, 209, 63, 255};
const Color BACKGROUND_COLOR = {113, 170, 52, 255};

const Color BODY_BLUE = {66, 135, 245, 255};
const Color HEAD_BLUE = {16, 85, 200, 255};

enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

std::queue<Direction> inputQueue;

struct Segment
{
    int x, y;
};

class Snake
{
public:
    Snake()
    {
        segments.push_back({8, 8});
        segments.push_back({7, 8});
        segments.push_back({6, 8});
        dir = RIGHT;
        while (!inputQueue.empty())
            inputQueue.pop();
    }

    void queueDirection(Direction d)
    {
        Direction lastDir = inputQueue.empty() ? dir : inputQueue.back();
        if ((lastDir == UP && d != DOWN) ||
            (lastDir == DOWN && d != UP) ||
            (lastDir == LEFT && d != RIGHT) ||
            (lastDir == RIGHT && d != LEFT))
        {
            if (inputQueue.size() < 2)
            {
                inputQueue.push(d);
            }
        }
    }

    void move()
    {
        if (!inputQueue.empty())
        {
            Direction newDir = inputQueue.front();
            inputQueue.pop();
            dir = newDir;
        }

        Segment head = segments[0];
        switch (dir)
        {
        case UP:
            head.y--;
            break;
        case DOWN:
            head.y++;
            break;
        case LEFT:
            head.x--;
            break;
        case RIGHT:
            head.x++;
            break;
        }

        segments.insert(segments.begin(), head);
        segments.pop_back();
    }

    void grow()
    {
        segments.push_back(segments.back());
    }

    bool checkSelfCollision()
    {
        Segment head = segments[0];
        for (size_t i = 1; i < segments.size(); i++)
        {
            if (head.x == segments[i].x && head.y == segments[i].y)
                return true;
        }
        return false;
    }

    bool checkBorderCollision()
    {
        Segment head = segments[0];
        return (head.x < 0 || head.x >= GRID_WIDTH ||
                head.y < 0 || head.y >= GRID_HEIGHT);
    }

    const std::vector<Segment> &getSegments() const
    {
        return segments;
    }

    Segment getHead() const
    {
        return segments[0];
    }

private:
    std::vector<Segment> segments;
    Direction dir;
};

Segment generateFood()
{
    return {rand() % GRID_WIDTH, rand() % GRID_HEIGHT};
}

void DrawStyledBackground()
{
    for (int row = 0; row < GRID_HEIGHT; ++row)
    {
        for (int col = 0; col < GRID_WIDTH; ++col)
        {
            Color color = ((row + col) % 2 == 0) ? LIGHT_GREEN : DARK_GREEN;
            DrawRectangle(MARGIN + col * BLOCK_SIZE, MARGIN + row * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, color);
        }
    }
}

void DrawSnakeSegment(const Segment &segment, bool isHead = false)
{
    Color color = isHead ? HEAD_BLUE : BODY_BLUE;
    DrawRectangle(MARGIN + segment.x * BLOCK_SIZE + 2, MARGIN + segment.y * BLOCK_SIZE + 2,
                  BLOCK_SIZE - 4, BLOCK_SIZE - 4, color);
}

void DrawFood(const Segment &food)
{
    DrawRectangle(MARGIN + food.x * BLOCK_SIZE + 2, MARGIN + food.y * BLOCK_SIZE + 2,
                  BLOCK_SIZE - 4, BLOCK_SIZE - 4, RED);
}

int main()
{
    setlocale(LC_ALL, "pl_PL.UTF-8");

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Snake");
    SetTargetFPS(60);
    srand(time(nullptr));

    Snake snake;
    Segment food = generateFood();
    int score = 0;
    bool gameOver = false;

    Font font = LoadFont("resources/font.ttf");
    if (!IsFontValid(font))
    {
        font = GetFontDefault();
    }

    while (!WindowShouldClose())
    {
        if (!gameOver)
        {
            // Process input every frame for smoother response
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                snake.queueDirection(UP);
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                snake.queueDirection(DOWN);
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
                snake.queueDirection(LEFT);
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
                snake.queueDirection(RIGHT);

            moveTimer += GetFrameTime();
            if (moveTimer >= moveInterval)
            {
                snake.move();
                moveTimer = 0.0f;

                if (snake.checkSelfCollision() || snake.checkBorderCollision())
                {
                    gameOver = true;
                }

                Segment head = snake.getHead();
                if (head.x == food.x && head.y == food.y)
                {
                    snake.grow();
                    score++;
                    food = generateFood();

                    bool onSnake = true;
                    while (onSnake)
                    {
                        onSnake = false;
                        for (const auto &segment : snake.getSegments())
                        {
                            if (food.x == segment.x && food.y == segment.y)
                            {
                                food = generateFood();
                                onSnake = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (IsKeyPressed(KEY_SPACE))
            {
                snake = Snake();
                food = generateFood();
                score = 0;
                gameOver = false;
                moveTimer = 0.0f;
            }
        }

        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        DrawStyledBackground();

        for (size_t i = 0; i < snake.getSegments().size(); i++)
        {
            DrawSnakeSegment(snake.getSegments()[i], i == 0);
        }

        DrawFood(food);
        char scoreText[64];
        sprintf(scoreText, "Wynik: %d", score);
        int scoreWidth = MeasureText(scoreText, 20);
        DrawText(scoreText, (WINDOW_WIDTH - scoreWidth) / 2, 10, 20, BLACK);

        if (gameOver)
        {
            const char *gameOverText = "KONIEC GRY";
            const char *restartText = "Nacisnij SPACJE aby zagrac ponownie";
            int gameOverWidth = MeasureText(gameOverText, 40);
            int restartWidth = MeasureText(restartText, 20);

            DrawText(gameOverText, (WINDOW_WIDTH - gameOverWidth) / 2,
                     WINDOW_HEIGHT / 2 - 40, 40, RED);
            DrawText(restartText, (WINDOW_WIDTH - restartWidth) / 2,
                     WINDOW_HEIGHT / 2 + 20, 20, BLACK);
        }

        EndDrawing();
    }

    if (IsFontValid(font) && font.texture.id != GetFontDefault().texture.id)
    {
        UnloadFont(font);
    }
    CloseWindow();
    return 0;
}
