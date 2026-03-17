// #include <raylib.h>
// #include "raylib.h"
#include <stdio.h>

// Color(...) constructor
typedef struct {
	unsigned char r;
	unsigned char b;
	unsigned char g;
	unsigned char a;
} Color;

const Color LIGHTGRAY  = (Color){ 200, 200, 200, 255 };   // Light Gray
const Color GRAY       = (Color){ 130, 130, 130, 255 };   // Gray
const Color DARKGRAY   = (Color){ 80, 80, 80, 255 };      // Dark Gray
const Color YELLOW     = (Color){ 253, 249, 0, 255 };     // Yellow
const Color GOLD       = (Color){ 255, 203, 0, 255 };     // Gold
const Color ORANGE     = (Color){ 255, 161, 0, 255 };     // Orange
const Color PINK       = (Color){ 255, 109, 194, 255 };   // Pink
const Color RED        = (Color){ 230, 41, 55, 255 };     // Red
const Color MAROON     = (Color){ 190, 33, 55, 255 };     // Maroon
const Color GREEN      = (Color){ 0, 228, 48, 255 };      // Green
const Color LIME       = (Color){ 0, 158, 47, 255 };      // Lime
const Color DARKGREEN  = (Color){ 0, 117, 44, 255 };      // Dark Green
const Color SKYBLUE    = (Color){ 102, 191, 255, 255 };   // Sky Blue
const Color BLUE       = (Color){ 0, 121, 241, 255 };     // Blue
const Color DARKBLUE   = (Color){ 0, 82, 172, 255 };      // Dark Blue
const Color PURPLE     = (Color){ 200, 122, 255, 255 };   // Purple
const Color VIOLET     = (Color){ 135, 60, 190, 255 };    // Violet
const Color DARKPURPLE = (Color){ 112, 31, 126, 255 };    // Dark Purple
const Color BEIGE      = (Color){ 211, 176, 131, 255 };   // Beige
const Color BROWN      = (Color){ 127, 106, 79, 255 };    // Brown
const Color DARKBROWN  = (Color){ 76, 63, 47, 255 };      // Dark Brown
const Color WHITE      = (Color){ 255, 255, 255, 255 };   // White
const Color BLACK      = (Color){ 0, 0, 0, 255 };         // Black
const Color BLANK      = (Color){ 0, 0, 0, 0 };           // Blank (Transparent)
const Color MAGENTA    = (Color){ 255, 0, 255, 255 };     // Magenta
const Color RAYWHITE   = (Color){ 245, 245, 245, 255 };   // My own White (raylib logo)


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

typedef enum {
    KEY_NULL            = 0,        // Key: NULL, used for no key pressed
    // Alphanumeric keys
    KEY_APOSTROPHE      = 39,       // Key: '
    KEY_COMMA           = 44,       // Key: ,
    KEY_MINUS           = 45,       // Key: -
    KEY_PERIOD          = 46,       // Key: .
    KEY_SLASH           = 47,       // Key: /
    KEY_ZERO            = 48,       // Key: 0
    KEY_ONE             = 49,       // Key: 1
    KEY_TWO             = 50,       // Key: 2
    KEY_THREE           = 51,       // Key: 3
    KEY_FOUR            = 52,       // Key: 4
    KEY_FIVE            = 53,       // Key: 5
    KEY_SIX             = 54,       // Key: 6
    KEY_SEVEN           = 55,       // Key: 7
    KEY_EIGHT           = 56,       // Key: 8
    KEY_NINE            = 57,       // Key: 9
    KEY_SEMICOLON       = 59,       // Key: ;
    KEY_EQUAL           = 61,       // Key: =
    KEY_A               = 65,       // Key: A | a
    KEY_B               = 66,       // Key: B | b
    KEY_C               = 67,       // Key: C | c
    KEY_D               = 68,       // Key: D | d
    KEY_E               = 69,       // Key: E | e
    KEY_F               = 70,       // Key: F | f
    KEY_G               = 71,       // Key: G | g
    KEY_H               = 72,       // Key: H | h
    KEY_I               = 73,       // Key: I | i
    KEY_J               = 74,       // Key: J | j
    KEY_K               = 75,       // Key: K | k
    KEY_L               = 76,       // Key: L | l
    KEY_M               = 77,       // Key: M | m
    KEY_N               = 78,       // Key: N | n
    KEY_O               = 79,       // Key: O | o
    KEY_P               = 80,       // Key: P | p
    KEY_Q               = 81,       // Key: Q | q
    KEY_R               = 82,       // Key: R | r
    KEY_S               = 83,       // Key: S | s
    KEY_T               = 84,       // Key: T | t
    KEY_U               = 85,       // Key: U | u
    KEY_V               = 86,       // Key: V | v
    KEY_W               = 87,       // Key: W | w
    KEY_X               = 88,       // Key: X | x
    KEY_Y               = 89,       // Key: Y | y
    KEY_Z               = 90,       // Key: Z | z
    KEY_LEFT_BRACKET    = 91,       // Key: [
    KEY_BACKSLASH       = 92,       // Key: '\'
    KEY_RIGHT_BRACKET   = 93,       // Key: ]
    KEY_GRAVE           = 96,       // Key: `
    // Function keys
    KEY_SPACE           = 32,       // Key: Space
    KEY_ESCAPE          = 256,      // Key: Esc
    KEY_ENTER           = 257,      // Key: Enter
    KEY_TAB             = 258,      // Key: Tab
    KEY_BACKSPACE       = 259,      // Key: Backspace
    KEY_INSERT          = 260,      // Key: Ins
    KEY_DELETE          = 261,      // Key: Del
    KEY_RIGHT           = 262,      // Key: Cursor right
    KEY_LEFT            = 263,      // Key: Cursor left
    KEY_DOWN            = 264,      // Key: Cursor down
    KEY_UP              = 265,      // Key: Cursor up
    KEY_PAGE_UP         = 266,      // Key: Page up
    KEY_PAGE_DOWN       = 267,      // Key: Page down
    KEY_HOME            = 268,      // Key: Home
    KEY_END             = 269,      // Key: End
    KEY_CAPS_LOCK       = 280,      // Key: Caps lock
    KEY_SCROLL_LOCK     = 281,      // Key: Scroll down
    KEY_NUM_LOCK        = 282,      // Key: Num lock
    KEY_PRINT_SCREEN    = 283,      // Key: Print screen
    KEY_PAUSE           = 284,      // Key: Pause
    KEY_F1              = 290,      // Key: F1
    KEY_F2              = 291,      // Key: F2
    KEY_F3              = 292,      // Key: F3
    KEY_F4              = 293,      // Key: F4
    KEY_F5              = 294,      // Key: F5
    KEY_F6              = 295,      // Key: F6
    KEY_F7              = 296,      // Key: F7
    KEY_F8              = 297,      // Key: F8
    KEY_F9              = 298,      // Key: F9
    KEY_F10             = 299,      // Key: F10
    KEY_F11             = 300,      // Key: F11
    KEY_F12             = 301,      // Key: F12
    KEY_LEFT_SHIFT      = 340,      // Key: Shift left
    KEY_LEFT_CONTROL    = 341,      // Key: Control left
    KEY_LEFT_ALT        = 342,      // Key: Alt left
    KEY_LEFT_SUPER      = 343,      // Key: Super left
    KEY_RIGHT_SHIFT     = 344,      // Key: Shift right
    KEY_RIGHT_CONTROL   = 345,      // Key: Control right
    KEY_RIGHT_ALT       = 346,      // Key: Alt right
    KEY_RIGHT_SUPER     = 347,      // Key: Super right
    KEY_KB_MENU         = 348,      // Key: KB menu
    // Keypad keys
    KEY_KP_0            = 320,      // Key: Keypad 0
    KEY_KP_1            = 321,      // Key: Keypad 1
    KEY_KP_2            = 322,      // Key: Keypad 2
    KEY_KP_3            = 323,      // Key: Keypad 3
    KEY_KP_4            = 324,      // Key: Keypad 4
    KEY_KP_5            = 325,      // Key: Keypad 5
    KEY_KP_6            = 326,      // Key: Keypad 6
    KEY_KP_7            = 327,      // Key: Keypad 7
    KEY_KP_8            = 328,      // Key: Keypad 8
    KEY_KP_9            = 329,      // Key: Keypad 9
    KEY_KP_DECIMAL      = 330,      // Key: Keypad .
    KEY_KP_DIVIDE       = 331,      // Key: Keypad /
    KEY_KP_MULTIPLY     = 332,      // Key: Keypad *
    KEY_KP_SUBTRACT     = 333,      // Key: Keypad -
    KEY_KP_ADD          = 334,      // Key: Keypad +
    KEY_KP_ENTER        = 335,      // Key: Keypad Enter
    KEY_KP_EQUAL        = 336,      // Key: Keypad =
    // Android key buttons
    KEY_BACK            = 4,        // Key: Android back button
    KEY_MENU            = 5,        // Key: Android menu button
    KEY_VOLUME_UP       = 24,       // Key: Android volume up button
    KEY_VOLUME_DOWN     = 25        // Key: Android volume down button
} KeyboardKey;

// typedef struct Image {
//     void *data;             // Image raw data
//     int width;              // Image base width
//     int height;             // Image base height
//     int mipmaps;            // Mipmap levels, 1 by default
//     int format;             // Data format (PixelFormat type)
// } Image;

// typedef struct GlyphInfo {
//     int value;              // Character value (Unicode)
//     int offsetX;            // Character offset X when drawing
//     int offsetY;            // Character offset Y when drawing
//     int advanceX;           // Character advance position X
//     Image image;            // Character image data
// } GlyphInfo;

// typedef struct Font {
//     int baseSize;           // Base size (default chars height)
//     int glyphCount;         // Number of glyph characters
//     int glyphPadding;       // Padding around the glyph characters
//     Texture2D texture;      // Texture atlas containing the glyphs
//     Rectangle *recs;        // Rectangles in texture for the glyphs
//     GlyphInfo *glyphs;      // Glyphs info data
// } Font;

void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint);
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color); 
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
int MeasureText(const char *text, int fontSize);
void DrawTexture(Texture2D texture, int posX, int posY, Color tint);
Texture2D LoadTexture(const char *fileName);
bool IsWindowResized(void);
void UnloadTexture(Texture2D texture);
void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);
// Font GetFontDefault(void);

// Secret ;)

// #define SCREEN_W 800
// #define SCREEN_H 500

// #define MAP_W 16
// #define MAP_H 16

// #define TEX_SIZE 128

// int worldMap[MAP_W][MAP_H] = {
// {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
// {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
// {1,0,2,0,0,0,0,0,0,0,0,2,0,0,0,1},
// {1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
// {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
// {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
// {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
// {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
// {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
// {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
// {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
// {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
// {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
// {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
// {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
// {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
// };

// Texture2D wallTextures[100] = {0};
// int wallTexturesCount = 0;
// const char* texts[] = {
//     "Hello!"
// };

// int run_game() {
//     InitWindow(SCREEN_W, SCREEN_H, "raylib raycaster");
//     DisableCursor();

//     Texture2D wallTex = {0};
//     bool useTexture = false;

//     wallTextures[wallTexturesCount++] = LoadTexture("wall.png");

//     float posX = 8, posY = 8;

//     float dirX = -1, dirY = 0;
//     float planeX = 0, planeY = 0.66f;

//     float pitch = 0;
//     float mouseSensitivity = 0.003f;

//     SetTargetFPS(60);

//     while (!WindowShouldClose())
//     {
//         float moveSpeed = 0.1f;

//         Vector2 mouseDelta = GetMouseDelta();

//         /* Mouse rotation */
//         float rot = -mouseDelta.x * mouseSensitivity;

//         float oldDirX = dirX;
//         dirX = dirX*cos(rot) - dirY*sin(rot);
//         dirY = oldDirX*sin(rot) + dirY*cos(rot);

//         float oldPlaneX = planeX;
//         planeX = planeX*cos(rot) - planeY*sin(rot);
//         planeY = oldPlaneX*sin(rot) + planeY*cos(rot);

//         /* Mouse vertical look */
//         pitch += -(mouseDelta.y * 2.0f);

//         if (pitch > SCREEN_H/2) pitch = SCREEN_H/2;
//         if (pitch < -SCREEN_H/2) pitch = -SCREEN_H/2;

//         /* Movement */
//         if (IsKeyDown(KEY_W)) {
//             if (!worldMap[(int)(posX + dirX*moveSpeed)][(int)posY]) posX += dirX*moveSpeed;
//             if (!worldMap[(int)posX][(int)(posY + dirY*moveSpeed)]) posY += dirY*moveSpeed;
//         }

//         if (IsKeyDown(KEY_S)) {
//             if (!worldMap[(int)(posX - dirX*moveSpeed)][(int)posY]) posX -= dirX*moveSpeed;
//             if (!worldMap[(int)posX][(int)(posY - dirY*moveSpeed)]) posY -= dirY*moveSpeed;
//         }

//         if (IsKeyDown(KEY_A)) {
//             if (!worldMap[(int)(posX - planeX*moveSpeed)][(int)posY]) posX -= planeX*moveSpeed;
//             if (!worldMap[(int)posX][(int)(posY - planeY*moveSpeed)]) posY -= planeY*moveSpeed;
//         }

//         if (IsKeyDown(KEY_D)) {
//             if (!worldMap[(int)(posX + planeX*moveSpeed)][(int)posY]) posX += planeX*moveSpeed;
//             if (!worldMap[(int)posX][(int)(posY + planeY*moveSpeed)]) posY += planeY*moveSpeed;
//         }

//         BeginDrawing();

//         int horizon = SCREEN_H/2 + pitch;

//         DrawRectangle(0, 0, SCREEN_W, horizon, SKYBLUE);
//         DrawRectangle(0, horizon, SCREEN_W, SCREEN_H - horizon, DARKGRAY);

//         for (int x = 0; x < SCREEN_W; x++)
//         {
//             float cameraX = 2*x/(float)SCREEN_W - 1;

//             float rayDirX = dirX + planeX*cameraX;
//             float rayDirY = dirY + planeY*cameraX;

//             int mapX = (int)posX;
//             int mapY = (int)posY;

//             float deltaDistX = fabs(1/rayDirX);
//             float deltaDistY = fabs(1/rayDirY);

//             float sideDistX, sideDistY;

//             int stepX, stepY;
//             int hit = 0;
//             int side;

//             if (rayDirX < 0) {
//                 stepX = -1;
//                 sideDistX = (posX - mapX) * deltaDistX;
//             } else {
//                 stepX = 1;
//                 sideDistX = (mapX + 1.0 - posX) * deltaDistX;
//             }

//             if (rayDirY < 0) {
//                 stepY = -1;
//                 sideDistY = (posY - mapY) * deltaDistY;
//             } else {
//                 stepY = 1;
//                 sideDistY = (mapY + 1.0 - posY) * deltaDistY;
//             }

//             while (!hit)
//             {
//                 if (sideDistX < sideDistY) {
//                     sideDistX += deltaDistX;
//                     mapX += stepX;
//                     side = 0;
//                 } else {
//                     sideDistY += deltaDistY;
//                     mapY += stepY;
//                     side = 1;
//                 }

//                 if (worldMap[mapX][mapY] > 0) hit = 1;
//             }

//             float perpWallDist;

//             if (side == 0)
//                 perpWallDist = (mapX - posX + (1 - stepX)/2) / rayDirX;
//             else
//                 perpWallDist = (mapY - posY + (1 - stepY)/2) / rayDirY;

//             int lineHeight = (int)(SCREEN_H / perpWallDist);

//             int drawStart = -lineHeight/2 + SCREEN_H/2 + pitch;
//             int drawEnd   =  lineHeight/2 + SCREEN_H/2 + pitch;

//             if (drawStart < -123) drawStart = 0;
//             if (drawEnd >= SCREEN_H + 123) drawEnd = SCREEN_H-1;

//             if (worldMap[mapX][mapY] < wallTexturesCount)
//             {
//                 float wallX;

//                 if (side == 0)
//                     wallX = posY + perpWallDist * rayDirY;
//                 else
//                     wallX = posX + perpWallDist * rayDirX;

//                 wallX -= floor(wallX);

//                 int texX = (int)(wallX * TEX_SIZE);

//                 Rectangle src = { texX, 0, 1, TEX_SIZE };
//                 Rectangle dst = { x, drawStart, 1, drawEnd - drawStart };

//                 ImageDrawTextEx(Image *dst, Font font, "HI", (Vector2){0,0}, 15.0, 0.2, WHITE);
//                 DrawTexturePro(wallTextures[worldMap[mapX][mapY]], src, dst, (Vector2){0,0}, 0, WHITE);
//             }
//             else
//             {
//                 DrawLine(x, drawStart, x, drawEnd, RED);
//             }
//         }

//         EndDrawing();
//     }

//     if (useTexture) UnloadTexture(wallTex);

//     CloseWindow();
// }