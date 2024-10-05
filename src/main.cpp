#include "raylib.h"
#include "Math.h"

constexpr float SCREEN_WIDTH = 1200.0f;
constexpr float SCREEN_HEIGHT = 800.0f;
constexpr Vector2 CENTER{ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };

// Ball can move half the screen width per-second
constexpr float BALL_SPEED = SCREEN_WIDTH * 0.5f;
constexpr float BALL_SIZE = 40.0f;

// Paddles can move half the screen height per-second
constexpr float PADDLE_SPEED = SCREEN_HEIGHT * 0.5f;
constexpr float PADDLE_WIDTH = 40.0f;
constexpr float PADDLE_HEIGHT = 80.0f;

Sound ballHit;
Music backgroundSong;
Music victoryMusic;

struct Box
{
    float xMin;
    float xMax;
    float yMin;
    float yMax;
};

bool BoxOverlap(Box box1, Box box2)
{
    bool x = box1.xMax >= box2.xMin && box1.xMin <= box2.xMax;
    bool y = box1.yMax >= box2.yMin && box1.yMin <= box2.yMax;
    return x && y;
}

Rectangle BoxToRec(Box box)
{
    Rectangle rec;
    rec.x = box.xMin;
    rec.y = box.yMin;
    rec.width = box.xMax - box.xMin;
    rec.height = box.yMax - box.yMin;
    return rec;
}

Box BallBox(Vector2 position)
{
    Box box;
    box.xMin = position.x - BALL_SIZE * 0.5f;
    box.xMax = position.x + BALL_SIZE * 0.5f;
    box.yMin = position.y - BALL_SIZE * 0.5f;
    box.yMax = position.y + BALL_SIZE * 0.5f;
    return box;
}

Box PaddleBox(Vector2 position)
{
    Box box;
    box.xMin = position.x - PADDLE_WIDTH * 0.5f;
    box.xMax = position.x + PADDLE_WIDTH * 0.5f;
    box.yMin = position.y - PADDLE_HEIGHT * 0.5f;
    box.yMax = position.y + PADDLE_HEIGHT * 0.5f;
    return box;
}

void ResetBall(Vector2& position, Vector2& direction)
{
    position = CENTER;
    direction.x = rand() % 2 == 0 ? -1.0f : 1.0f;
    direction.y = 0.0f;
    direction = Rotate(direction, Random(-45.0f, 260.0f) * DEG2RAD);
}

void DrawBall(Vector2 position, Color color)
{
    Box ballBox = BallBox(position);
    DrawRectangleRec(BoxToRec(ballBox), color);
}

void DrawPaddle(Vector2 position, Color color)
{
    Box paddleBox = PaddleBox(position);
    DrawRectangleRec(BoxToRec(paddleBox), color);
}

int main()
{
    Vector2 ballPosition;
    Vector2 ballDirection;
    ResetBall(ballPosition, ballDirection);

    Vector2 paddle1Position, paddle2Position;
    paddle1Position.x = SCREEN_WIDTH * 0.05f;
    paddle2Position.x = SCREEN_WIDTH * 0.95f;
    paddle1Position.y = paddle2Position.y = CENTER.y;

    int redScore = 0;
    int blueScore = 0;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong");
    SetTargetFPS(60);
    InitAudioDevice();
    ballHit = LoadSound("assets/betterPongNoise.mp3");
    backgroundSong = LoadMusicStream("assets/SlightChanceOfZombies.mp3");
    victoryMusic = LoadMusicStream("assets/lethalCompany.mp3");
    PlayMusicStream(backgroundSong);
    SetSoundVolume(ballHit, 0.1);
    SetMusicVolume(backgroundSong, 0.05);
    SetMusicVolume(victoryMusic, 0.3);
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        float ballDelta = BALL_SPEED * dt;
        float paddleDelta = PADDLE_SPEED * dt;

        UpdateMusicStream(backgroundSong);
        UpdateMusicStream(victoryMusic);

        // Move paddle 1 with W and A keys
        if (IsKeyDown(KEY_W))
            paddle1Position.y -= paddleDelta;
        if (IsKeyDown(KEY_S))
            paddle1Position.y += paddleDelta;

        // Move paddle 2 with arrow keys
        if (IsKeyDown(KEY_UP))
            paddle2Position.y -= paddleDelta;
        if (IsKeyDown(KEY_DOWN))
            paddle2Position.y += paddleDelta;

        //Resets the game after Space bar is pressed
        if (IsKeyPressed(KEY_SPACE)) 
        {
            ResetBall(ballPosition, ballDirection);
            redScore = 0;
            blueScore = 0;
            //Checks if the background music is playing in order to reset it properly
            if (IsMusicStreamPlaying(backgroundSong)) 
            {
                StopMusicStream(backgroundSong);
            }
            StopMusicStream(victoryMusic);
            PlayMusicStream(backgroundSong);
            EndDrawing();
        }

        float phh = PADDLE_HEIGHT * 0.5f;
        paddle1Position.y = Clamp(paddle1Position.y, phh, SCREEN_HEIGHT - phh);
        paddle2Position.y = Clamp(paddle2Position.y, phh, SCREEN_HEIGHT - phh);

        // Change the ball's direction on-collision
        Vector2 ballPositionNext = ballPosition + ballDirection * ballDelta;
        Box ballBox = BallBox(ballPositionNext);
        Box paddle1Box = PaddleBox(paddle1Position);
        Box paddle2Box = PaddleBox(paddle2Position);

        // TODO -- increment the scoring player's score after they've touched the ball and the ball goes too far right/left
        
        // Resets the ball and adds a point to the team that hits its opposite side of the screen
        if (ballBox.xMin < 0.0f) 
        {
            ballDirection.x *= -1.0f;
            blueScore++;
            ResetBall(ballPosition, ballDirection);
        }
        if (ballBox.xMax > SCREEN_WIDTH)
        {
            ballDirection.x *= -1.0f;
            redScore++;
            ResetBall(ballPosition, ballDirection);
        }

        // I decided to only put the ball sound fgor the paddle hit because it got lowkey annoying ngl
        if (ballBox.yMin < 0.0f || ballBox.yMax > SCREEN_HEIGHT)
        {
            ballDirection.y *= -1.0f;
        }
        if (BoxOverlap(ballBox, paddle1Box) || BoxOverlap(ballBox, paddle2Box))
        {
            ballDirection.x *= -1.0f;
            PlaySound(ballHit);
        }

        // Update ball position after collision resolution, then render
        ballPosition = ballPosition + ballDirection * ballDelta;

        BeginDrawing();
        ClearBackground(BLACK);
        DrawBall(ballPosition, WHITE);
        DrawPaddle(paddle1Position, RED);
        DrawPaddle(paddle2Position, BLUE);

        // Text format requires you to put a '%i' wherever you want an integer, then add said integer after the comma
        const char* redScoreText = TextFormat(" %i ", redScore);
        const char* blueScoreText = TextFormat(" %i ", blueScore);

        
        // We can measure our text for more exact positioning. This puts our score in the center of our screen!
        DrawText(redScoreText, SCREEN_WIDTH * 0.4f - MeasureText(redScoreText, 20) * 0.5f, 50, 20, RED);
        DrawText(blueScoreText, SCREEN_WIDTH * 0.6f - MeasureText(blueScoreText, 20) * 0.5f, 50, 20, BLUE);

        // Displays which team won and stops ball from moving
        if (redScore > 5) 
        {
            const char* redVictoryText = TextFormat("Red Team Wins!");
            DrawText(redVictoryText, CENTER.x - MeasureText(redVictoryText, 50), CENTER.y, 100, RED);
            ballPosition = CENTER;
            PlayMusicStream(victoryMusic);
            StopMusicStream(backgroundSong);
        }
        else if (blueScore > 5) 
        {
            const char* blueVictoryText = TextFormat("Blue Team Wins!");
            DrawText(blueVictoryText, CENTER.x - MeasureText(blueVictoryText, 50), CENTER.y, 100, BLUE);
            ballPosition = CENTER;
            PlayMusicStream(victoryMusic);
            StopMusicStream(backgroundSong);
        }
        
        EndDrawing();
    }
    CloseAudioDevice();
    UnloadMusicStream(backgroundSong);
    UnloadMusicStream(victoryMusic);
    UnloadSound(ballHit);
    CloseWindow();
    return 0;
}
