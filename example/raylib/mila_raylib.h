#include <stdio.h>

// Color(...) constructor
typedef struct {
	unsigned char r;
	unsigned char b;
	unsigned char g;
	unsigned char a;
} Color;

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