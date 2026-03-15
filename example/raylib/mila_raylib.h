// #include <raylib.h>
#include <stdio.h>

// Color(...) constructor
typedef struct {
	unsigned char r;
	unsigned char b;
	unsigned char g;
	unsigned char a;
} Color;

// Rectangle(...) constructor
typedef struct Rectangle {
    float x;                // Rectangle top-left corner position x
    float y;                // Rectangle top-left corner position y
    float width;            // Rectangle width
    float height;           // Rectangle height
} Rectangle;

typedef struct Texture2D {
    unsigned int id;        // OpenGL texture id
    int width;              // Texture2D base width
    int height;             // Texture2D base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format (PixelFormat type)
} Texture2D;

typedef struct Vector2 {
    float x;                // Vector x component
    float y;                // Vector y component
} Vector2;

void InitWindow(int width, int height, char* title);
bool WindowShouldClose(void);
void BeginDrawing(void);
void EndDrawing(void);
void ClearBackground(Color bg);
void DrawRectangle(int x, int y, int width, int height, Color color);
void DrawText(char* text, int x, int y, int font_size, Color color);
bool IsKeyDown(int key);
bool IsKeyPressed(int key);
void SetTargetFPS(int);
int GetFPS(void);
void CloseWindow(void);
void DrawRectangleRec(Rectangle rec, Color color);
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);
int GetRandomValue(int min, int max);
int GetScreenWidth(void);
int GetScreenHeight(void);
void DisableCursor(void);
bool FileExists(const char *fileName);
Vector2 GetMouseDelta(void);
Color Fade(Color color, float alpha);
void DrawRectangleGradientV(int posX, int posY, int width, int height, Color top, Color bottom);
void WaitTime(double seconds);