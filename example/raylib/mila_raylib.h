#include <stdio.h>

void InitWindow(int a, int b, char* c);
bool WindowShouldClose();
void BeginDrawing();
void EndDrawing();
void ClearBackground(long a);
void DrawRectangle(int a, int b, int c, int d, long e);
void DrawText(char* a, int b, int c, int d, long e);
bool IsKeyDown(int key);
bool IsKeyPressed(int key);
bool IsKeyPressedRepeat(int key);
int GetKeyPressed(void);
void SetTargetFPS(int);