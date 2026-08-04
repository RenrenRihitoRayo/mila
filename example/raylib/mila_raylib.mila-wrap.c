#define ML_LIB
#include "mila.c"
#include "/home/renren/proj/c/mila/example/raylib/mila_raylib.h"


Value* native_mila_DrawTextureRec(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawTextureRec: wrong arg count");

    DrawTextureRec(*((Texture2D*)argv[0]->v), *((Rectangle*)argv[1]->v), *((Vector2*)argv[2]->v), *((Color*)argv[3]->v));
    return vnull();
}


Value* native_mila_DrawLine(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawLine: wrong arg count");

    DrawLine(((int)GET_INTEGER(argv[0])), ((int)GET_INTEGER(argv[1])), ((int)GET_INTEGER(argv[2])), ((int)GET_INTEGER(argv[3])), *((Color*)argv[4]->v));
    return vnull();
}


Value* native_mila_InitWindow(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("InitWindow: wrong arg count");

    InitWindow(((int)GET_INTEGER(argv[0])), ((int)GET_INTEGER(argv[1])), (char*)argv[2]->v);
    return vnull();
}


Value* native_mila_WindowShouldClose(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("WindowShouldClose: wrong arg count");

    int res = WindowShouldClose();
    return vint((long)res);
}


Value* native_mila_BeginDrawing(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("BeginDrawing: wrong arg count");

    BeginDrawing();
    return vnull();
}


Value* native_mila_EndDrawing(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("EndDrawing: wrong arg count");

    EndDrawing();
    return vnull();
}


Value* native_mila_ClearBackground(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ClearBackground: wrong arg count");

    ClearBackground(*((Color*)argv[0]->v));
    return vnull();
}


Value* native_mila_DrawRectangle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawRectangle: wrong arg count");

    DrawRectangle(((int)GET_INTEGER(argv[0])), ((int)GET_INTEGER(argv[1])), ((int)GET_INTEGER(argv[2])), ((int)GET_INTEGER(argv[3])), *((Color*)argv[4]->v));
    return vnull();
}


Value* native_mila_DrawText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawText: wrong arg count");

    DrawText((char*)argv[0]->v, ((int)GET_INTEGER(argv[1])), ((int)GET_INTEGER(argv[2])), ((int)GET_INTEGER(argv[3])), *((Color*)argv[4]->v));
    return vnull();
}


Value* native_mila_IsKeyDown(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsKeyDown: wrong arg count");

    int res = IsKeyDown(((int)GET_INTEGER(argv[0])));
    return vint((long)res);
}


Value* native_mila_IsKeyPressed(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsKeyPressed: wrong arg count");

    int res = IsKeyPressed(((int)GET_INTEGER(argv[0])));
    return vint((long)res);
}


Value* native_mila_SetTargetFPS(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetTargetFPS: wrong arg count");

    SetTargetFPS(((int)GET_INTEGER(argv[0])));
    return vnull();
}


Value* native_mila_GetFPS(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetFPS: wrong arg count");

    int res = GetFPS();
    return vint((long)res);
}


Value* native_mila_CloseWindow(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("CloseWindow: wrong arg count");

    CloseWindow();
    return vnull();
}


Value* native_mila_DrawRectangleRec(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("DrawRectangleRec: wrong arg count");

    DrawRectangleRec(*((Rectangle*)argv[0]->v), *((Color*)argv[1]->v));
    return vnull();
}


Value* native_mila_CheckCollisionRecs(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("CheckCollisionRecs: wrong arg count");

    int res = CheckCollisionRecs(*((Rectangle*)argv[0]->v), *((Rectangle*)argv[1]->v));
    return vint((long)res);
}


Value* native_mila_GetRandomValue(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetRandomValue: wrong arg count");

    int res = GetRandomValue(((int)GET_INTEGER(argv[0])), ((int)GET_INTEGER(argv[1])));
    return vint((long)res);
}


Value* native_mila_GetScreenWidth(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetScreenWidth: wrong arg count");

    int res = GetScreenWidth();
    return vint((long)res);
}


Value* native_mila_GetScreenHeight(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetScreenHeight: wrong arg count");

    int res = GetScreenHeight();
    return vint((long)res);
}


Value* native_mila_DisableCursor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("DisableCursor: wrong arg count");

    DisableCursor();
    return vnull();
}


Value* native_mila_FileExists(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("FileExists: wrong arg count");

    int res = FileExists((char*)argv[0]->v);
    return vint((long)res);
}


Value* native_mila_GetMouseDelta(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetMouseDelta: wrong arg count");
    Vector2* res = (Vector2*)malloc(sizeof(Vector2)); 
    Vector2 tmp = GetMouseDelta();
    memcpy(res, &tmp, sizeof(Vector2));
    return vopaque(res);
}


Value* native_mila_Fade(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("Fade: wrong arg count");
    Color* res = (Color*)malloc(sizeof(Color)); 
    Color tmp = Fade(*((Color*)argv[0]->v), argv[1]->v->f);
    memcpy(res, &tmp, sizeof(Color));
    return vopaque(res);
}


Value* native_mila_DrawRectangleGradientV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawRectangleGradientV: wrong arg count");

    DrawRectangleGradientV(((int)GET_INTEGER(argv[0])), ((int)GET_INTEGER(argv[1])), ((int)GET_INTEGER(argv[2])), ((int)GET_INTEGER(argv[3])), *((Color*)argv[4]->v), *((Color*)argv[5]->v));
    return vnull();
}


Value* native_mila_WaitTime(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("WaitTime: wrong arg count");

    WaitTime(argv[0]->v->f);
    return vnull();
}


Value* native_mila_MeasureText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("MeasureText: wrong arg count");

    int res = MeasureText((char*)argv[0]->v, ((int)GET_INTEGER(argv[1])));
    return vint((long)res);
}


Value* native_mila_DrawTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawTexture: wrong arg count");

    DrawTexture(*((Texture2D*)argv[0]->v), ((int)GET_INTEGER(argv[1])), ((int)GET_INTEGER(argv[2])), *((Color*)argv[3]->v));
    return vnull();
}


Value* native_mila_LoadTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadTexture: wrong arg count");
    Texture2D* res = (Texture2D*)malloc(sizeof(Texture2D)); 
    Texture2D tmp = LoadTexture((char*)argv[0]->v);
    memcpy(res, &tmp, sizeof(Texture2D));
    return vopaque(res);
}


Value* native_mila_IsWindowResized(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsWindowResized: wrong arg count");

    int res = IsWindowResized();
    return vint((long)res);
}


Value* native_mila_UnloadTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadTexture: wrong arg count");

    UnloadTexture(*((Texture2D*)argv[0]->v));
    return vnull();
}


Value* native_mila_DrawTexturePro(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawTexturePro: wrong arg count");

    DrawTexturePro(*((Texture2D*)argv[0]->v), *((Rectangle*)argv[1]->v), *((Rectangle*)argv[2]->v), *((Vector2*)argv[3]->v), argv[4]->v->f, *((Color*)argv[5]->v));
    return vnull();
}


Value* native_mila_DrawCircle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawCircle: wrong arg count");

    DrawCircle(((int)GET_INTEGER(argv[0])), ((int)GET_INTEGER(argv[1])), argv[2]->v->f, *((Color*)argv[3]->v));
    return vnull();
}

Value* _type_mila_Color_get_r(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Color.get_r(Color s): Expected 1 argument!");
    return vint(((Color*)(argv[0]->v))->r);}

Value* _type_mila_Color_set_r(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Color.get_r(Color s, unsigned char r): Expected 2 arguments!");
    ((Color*)(argv[0]->v))->r = (unsigned char)(GET_INTEGER(argv[1]));
    return vnull();
}

Value* _type_mila_Color_get_b(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Color.get_b(Color s): Expected 1 argument!");
    return vint(((Color*)(argv[0]->v))->b);}

Value* _type_mila_Color_set_b(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Color.get_b(Color s, unsigned char b): Expected 2 arguments!");
    ((Color*)(argv[0]->v))->b = (unsigned char)(GET_INTEGER(argv[1]));
    return vnull();
}

Value* _type_mila_Color_get_g(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Color.get_g(Color s): Expected 1 argument!");
    return vint(((Color*)(argv[0]->v))->g);}

Value* _type_mila_Color_set_g(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Color.get_g(Color s, unsigned char g): Expected 2 arguments!");
    ((Color*)(argv[0]->v))->g = (unsigned char)(GET_INTEGER(argv[1]));
    return vnull();
}

Value* _type_mila_Color_get_a(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Color.get_a(Color s): Expected 1 argument!");
    return vint(((Color*)(argv[0]->v))->a);}

Value* _type_mila_Color_set_a(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Color.get_a(Color s, unsigned char a): Expected 2 arguments!");
    ((Color*)(argv[0]->v))->a = (unsigned char)(GET_INTEGER(argv[1]));
    return vnull();
}

Value* _type_mila_Color_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4) return verror("Color_new: expected 4 arguments, got %i", argc);
    Color* tmp = (Color*)malloc(sizeof(Color));
    tmp->r = (unsigned char)(GET_INTEGER(argv[0]));
    tmp->b = (unsigned char)(GET_INTEGER(argv[1]));
    tmp->g = (unsigned char)(GET_INTEGER(argv[2]));
    tmp->a = (unsigned char)(GET_INTEGER(argv[3]));
    return vowned_opaque_extra(tmp, NULL, "struct Color");
}

Value* _type_mila_Rectangle_get_x(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Rectangle.get_x(Rectangle s): Expected 1 argument!");
    return vfloat((double)((Rectangle*)(argv[0]->v))->x);}

Value* _type_mila_Rectangle_set_x(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Rectangle.get_x(Rectangle s, float x): Expected 2 arguments!");
    ((Rectangle*)(argv[0]->v))->x = argv[1]->v->f;
    return vnull();
}

Value* _type_mila_Rectangle_get_y(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Rectangle.get_y(Rectangle s): Expected 1 argument!");
    return vfloat((double)((Rectangle*)(argv[0]->v))->y);}

Value* _type_mila_Rectangle_set_y(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Rectangle.get_y(Rectangle s, float y): Expected 2 arguments!");
    ((Rectangle*)(argv[0]->v))->y = argv[1]->v->f;
    return vnull();
}

Value* _type_mila_Rectangle_get_width(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Rectangle.get_width(Rectangle s): Expected 1 argument!");
    return vfloat((double)((Rectangle*)(argv[0]->v))->width);}

Value* _type_mila_Rectangle_set_width(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Rectangle.get_width(Rectangle s, float width): Expected 2 arguments!");
    ((Rectangle*)(argv[0]->v))->width = argv[1]->v->f;
    return vnull();
}

Value* _type_mila_Rectangle_get_height(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Rectangle.get_height(Rectangle s): Expected 1 argument!");
    return vfloat((double)((Rectangle*)(argv[0]->v))->height);}

Value* _type_mila_Rectangle_set_height(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Rectangle.get_height(Rectangle s, float height): Expected 2 arguments!");
    ((Rectangle*)(argv[0]->v))->height = argv[1]->v->f;
    return vnull();
}

Value* _type_mila_Rectangle_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4) return verror("Rectangle_new: expected 4 arguments, got %i", argc);
    Rectangle* tmp = (Rectangle*)malloc(sizeof(Rectangle));
    tmp->x = argv[0]->v->f;
    tmp->y = argv[1]->v->f;
    tmp->width = argv[2]->v->f;
    tmp->height = argv[3]->v->f;
    return vowned_opaque_extra(tmp, NULL, "struct Rectangle");
}

Value* _type_mila_Texture2D_get_id(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Texture2D.get_id(Texture2D s): Expected 1 argument!");
    return vint(((Texture2D*)(argv[0]->v))->id);}

Value* _type_mila_Texture2D_set_id(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Texture2D.get_id(Texture2D s, unsigned int id): Expected 2 arguments!");
    ((Texture2D*)(argv[0]->v))->id = argv[1]->v->ui;
    return vnull();
}

Value* _type_mila_Texture2D_get_width(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Texture2D.get_width(Texture2D s): Expected 1 argument!");
    return vint((long)((Texture2D*)(argv[0]->v))->width);}

Value* _type_mila_Texture2D_set_width(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Texture2D.get_width(Texture2D s, int width): Expected 2 arguments!");
    ((Texture2D*)(argv[0]->v))->width = ((int)GET_INTEGER(argv[1]));
    return vnull();
}

Value* _type_mila_Texture2D_get_height(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Texture2D.get_height(Texture2D s): Expected 1 argument!");
    return vint((long)((Texture2D*)(argv[0]->v))->height);}

Value* _type_mila_Texture2D_set_height(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Texture2D.get_height(Texture2D s, int height): Expected 2 arguments!");
    ((Texture2D*)(argv[0]->v))->height = ((int)GET_INTEGER(argv[1]));
    return vnull();
}

Value* _type_mila_Texture2D_get_mipmaps(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Texture2D.get_mipmaps(Texture2D s): Expected 1 argument!");
    return vint((long)((Texture2D*)(argv[0]->v))->mipmaps);}

Value* _type_mila_Texture2D_set_mipmaps(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Texture2D.get_mipmaps(Texture2D s, int mipmaps): Expected 2 arguments!");
    ((Texture2D*)(argv[0]->v))->mipmaps = ((int)GET_INTEGER(argv[1]));
    return vnull();
}

Value* _type_mila_Texture2D_get_format(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Texture2D.get_format(Texture2D s): Expected 1 argument!");
    return vint((long)((Texture2D*)(argv[0]->v))->format);}

Value* _type_mila_Texture2D_set_format(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Texture2D.get_format(Texture2D s, int format): Expected 2 arguments!");
    ((Texture2D*)(argv[0]->v))->format = ((int)GET_INTEGER(argv[1]));
    return vnull();
}

Value* _type_mila_Texture2D_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5) return verror("Texture2D_new: expected 5 arguments, got %i", argc);
    Texture2D* tmp = (Texture2D*)malloc(sizeof(Texture2D));
    tmp->id = argv[0]->v->ui;
    tmp->width = ((int)GET_INTEGER(argv[1]));
    tmp->height = ((int)GET_INTEGER(argv[2]));
    tmp->mipmaps = ((int)GET_INTEGER(argv[3]));
    tmp->format = ((int)GET_INTEGER(argv[4]));
    return vowned_opaque_extra(tmp, NULL, "struct Texture2D");
}

Value* _type_mila_Vector2_get_x(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Vector2.get_x(Vector2 s): Expected 1 argument!");
    return vfloat((double)((Vector2*)(argv[0]->v))->x);}

Value* _type_mila_Vector2_set_x(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Vector2.get_x(Vector2 s, float x): Expected 2 arguments!");
    ((Vector2*)(argv[0]->v))->x = argv[1]->v->f;
    return vnull();
}

Value* _type_mila_Vector2_get_y(Env* env, int argc, Value** argv) {
    if (argc != 1) verror("Vector2.get_y(Vector2 s): Expected 1 argument!");
    return vfloat((double)((Vector2*)(argv[0]->v))->y);}

Value* _type_mila_Vector2_set_y(Env* env, int argc, Value** argv) {
    if (argc != 2) verror("Vector2.get_y(Vector2 s, float y): Expected 2 arguments!");
    ((Vector2*)(argv[0]->v))->y = argv[1]->v->f;
    return vnull();
}

Value* _type_mila_Vector2_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2) return verror("Vector2_new: expected 2 arguments, got %i", argc);
    Vector2* tmp = (Vector2*)malloc(sizeof(Vector2));
    tmp->x = argv[0]->v->f;
    tmp->y = argv[1]->v->f;
    return vowned_opaque_extra(tmp, NULL, "struct Vector2");
}

const NativeEntry lib_function_entries[] = {
    {"DrawTextureRec", native_mila_DrawTextureRec},
    {"DrawLine", native_mila_DrawLine},
    {"InitWindow", native_mila_InitWindow},
    {"WindowShouldClose", native_mila_WindowShouldClose},
    {"BeginDrawing", native_mila_BeginDrawing},
    {"EndDrawing", native_mila_EndDrawing},
    {"ClearBackground", native_mila_ClearBackground},
    {"DrawRectangle", native_mila_DrawRectangle},
    {"DrawText", native_mila_DrawText},
    {"IsKeyDown", native_mila_IsKeyDown},
    {"IsKeyPressed", native_mila_IsKeyPressed},
    {"SetTargetFPS", native_mila_SetTargetFPS},
    {"GetFPS", native_mila_GetFPS},
    {"CloseWindow", native_mila_CloseWindow},
    {"DrawRectangleRec", native_mila_DrawRectangleRec},
    {"CheckCollisionRecs", native_mila_CheckCollisionRecs},
    {"GetRandomValue", native_mila_GetRandomValue},
    {"GetScreenWidth", native_mila_GetScreenWidth},
    {"GetScreenHeight", native_mila_GetScreenHeight},
    {"DisableCursor", native_mila_DisableCursor},
    {"FileExists", native_mila_FileExists},
    {"GetMouseDelta", native_mila_GetMouseDelta},
    {"Fade", native_mila_Fade},
    {"DrawRectangleGradientV", native_mila_DrawRectangleGradientV},
    {"WaitTime", native_mila_WaitTime},
    {"MeasureText", native_mila_MeasureText},
    {"DrawTexture", native_mila_DrawTexture},
    {"LoadTexture", native_mila_LoadTexture},
    {"IsWindowResized", native_mila_IsWindowResized},
    {"UnloadTexture", native_mila_UnloadTexture},
    {"DrawTexturePro", native_mila_DrawTexturePro},
    {"DrawCircle", native_mila_DrawCircle},
    {"Color", _type_mila_Color_new},
    {"Color.get_r", _type_mila_Color_get_r},
    {"Color.set_r", _type_mila_Color_set_r},
    {"Color.get_b", _type_mila_Color_get_b},
    {"Color.set_b", _type_mila_Color_set_b},
    {"Color.get_g", _type_mila_Color_get_g},
    {"Color.set_g", _type_mila_Color_set_g},
    {"Color.get_a", _type_mila_Color_get_a},
    {"Color.set_a", _type_mila_Color_set_a},
    {"Rectangle", _type_mila_Rectangle_new},
    {"Rectangle.get_x", _type_mila_Rectangle_get_x},
    {"Rectangle.set_x", _type_mila_Rectangle_set_x},
    {"Rectangle.get_y", _type_mila_Rectangle_get_y},
    {"Rectangle.set_y", _type_mila_Rectangle_set_y},
    {"Rectangle.get_width", _type_mila_Rectangle_get_width},
    {"Rectangle.set_width", _type_mila_Rectangle_set_width},
    {"Rectangle.get_height", _type_mila_Rectangle_get_height},
    {"Rectangle.set_height", _type_mila_Rectangle_set_height},
    {"Texture2D", _type_mila_Texture2D_new},
    {"Texture2D.get_id", _type_mila_Texture2D_get_id},
    {"Texture2D.set_id", _type_mila_Texture2D_set_id},
    {"Texture2D.get_width", _type_mila_Texture2D_get_width},
    {"Texture2D.set_width", _type_mila_Texture2D_set_width},
    {"Texture2D.get_height", _type_mila_Texture2D_get_height},
    {"Texture2D.set_height", _type_mila_Texture2D_set_height},
    {"Texture2D.get_mipmaps", _type_mila_Texture2D_get_mipmaps},
    {"Texture2D.set_mipmaps", _type_mila_Texture2D_set_mipmaps},
    {"Texture2D.get_format", _type_mila_Texture2D_get_format},
    {"Texture2D.set_format", _type_mila_Texture2D_set_format},
    {"Vector2", _type_mila_Vector2_new},
    {"Vector2.get_x", _type_mila_Vector2_get_x},
    {"Vector2.set_x", _type_mila_Vector2_set_x},
    {"Vector2.get_y", _type_mila_Vector2_get_y},
    {"Vector2.set_y", _type_mila_Vector2_set_y},
    {NULL, NULL}
};

void _mila_lib_init(Env* e) {
    env_set_raw(e, "KeyboardKey.KEY_NULL", vint(0));
    env_set_raw(e, "KeyboardKey.KEY_APOSTROPHE", vint(39));
    env_set_raw(e, "KeyboardKey.KEY_COMMA", vint(44));
    env_set_raw(e, "KeyboardKey.KEY_MINUS", vint(45));
    env_set_raw(e, "KeyboardKey.KEY_PERIOD", vint(46));
    env_set_raw(e, "KeyboardKey.KEY_SLASH", vint(47));
    env_set_raw(e, "KeyboardKey.KEY_ZERO", vint(48));
    env_set_raw(e, "KeyboardKey.KEY_ONE", vint(49));
    env_set_raw(e, "KeyboardKey.KEY_TWO", vint(50));
    env_set_raw(e, "KeyboardKey.KEY_THREE", vint(51));
    env_set_raw(e, "KeyboardKey.KEY_FOUR", vint(52));
    env_set_raw(e, "KeyboardKey.KEY_FIVE", vint(53));
    env_set_raw(e, "KeyboardKey.KEY_SIX", vint(54));
    env_set_raw(e, "KeyboardKey.KEY_SEVEN", vint(55));
    env_set_raw(e, "KeyboardKey.KEY_EIGHT", vint(56));
    env_set_raw(e, "KeyboardKey.KEY_NINE", vint(57));
    env_set_raw(e, "KeyboardKey.KEY_SEMICOLON", vint(59));
    env_set_raw(e, "KeyboardKey.KEY_EQUAL", vint(61));
    env_set_raw(e, "KeyboardKey.KEY_A", vint(65));
    env_set_raw(e, "KeyboardKey.KEY_B", vint(66));
    env_set_raw(e, "KeyboardKey.KEY_C", vint(67));
    env_set_raw(e, "KeyboardKey.KEY_D", vint(68));
    env_set_raw(e, "KeyboardKey.KEY_E", vint(69));
    env_set_raw(e, "KeyboardKey.KEY_F", vint(70));
    env_set_raw(e, "KeyboardKey.KEY_G", vint(71));
    env_set_raw(e, "KeyboardKey.KEY_H", vint(72));
    env_set_raw(e, "KeyboardKey.KEY_I", vint(73));
    env_set_raw(e, "KeyboardKey.KEY_J", vint(74));
    env_set_raw(e, "KeyboardKey.KEY_K", vint(75));
    env_set_raw(e, "KeyboardKey.KEY_L", vint(76));
    env_set_raw(e, "KeyboardKey.KEY_M", vint(77));
    env_set_raw(e, "KeyboardKey.KEY_N", vint(78));
    env_set_raw(e, "KeyboardKey.KEY_O", vint(79));
    env_set_raw(e, "KeyboardKey.KEY_P", vint(80));
    env_set_raw(e, "KeyboardKey.KEY_Q", vint(81));
    env_set_raw(e, "KeyboardKey.KEY_R", vint(82));
    env_set_raw(e, "KeyboardKey.KEY_S", vint(83));
    env_set_raw(e, "KeyboardKey.KEY_T", vint(84));
    env_set_raw(e, "KeyboardKey.KEY_U", vint(85));
    env_set_raw(e, "KeyboardKey.KEY_V", vint(86));
    env_set_raw(e, "KeyboardKey.KEY_W", vint(87));
    env_set_raw(e, "KeyboardKey.KEY_X", vint(88));
    env_set_raw(e, "KeyboardKey.KEY_Y", vint(89));
    env_set_raw(e, "KeyboardKey.KEY_Z", vint(90));
    env_set_raw(e, "KeyboardKey.KEY_LEFT_BRACKET", vint(91));
    env_set_raw(e, "KeyboardKey.KEY_BACKSLASH", vint(92));
    env_set_raw(e, "KeyboardKey.KEY_RIGHT_BRACKET", vint(93));
    env_set_raw(e, "KeyboardKey.KEY_GRAVE", vint(96));
    env_set_raw(e, "KeyboardKey.KEY_SPACE", vint(32));
    env_set_raw(e, "KeyboardKey.KEY_ESCAPE", vint(256));
    env_set_raw(e, "KeyboardKey.KEY_ENTER", vint(257));
    env_set_raw(e, "KeyboardKey.KEY_TAB", vint(258));
    env_set_raw(e, "KeyboardKey.KEY_BACKSPACE", vint(259));
    env_set_raw(e, "KeyboardKey.KEY_INSERT", vint(260));
    env_set_raw(e, "KeyboardKey.KEY_DELETE", vint(261));
    env_set_raw(e, "KeyboardKey.KEY_RIGHT", vint(262));
    env_set_raw(e, "KeyboardKey.KEY_LEFT", vint(263));
    env_set_raw(e, "KeyboardKey.KEY_DOWN", vint(264));
    env_set_raw(e, "KeyboardKey.KEY_UP", vint(265));
    env_set_raw(e, "KeyboardKey.KEY_PAGE_UP", vint(266));
    env_set_raw(e, "KeyboardKey.KEY_PAGE_DOWN", vint(267));
    env_set_raw(e, "KeyboardKey.KEY_HOME", vint(268));
    env_set_raw(e, "KeyboardKey.KEY_END", vint(269));
    env_set_raw(e, "KeyboardKey.KEY_CAPS_LOCK", vint(280));
    env_set_raw(e, "KeyboardKey.KEY_SCROLL_LOCK", vint(281));
    env_set_raw(e, "KeyboardKey.KEY_NUM_LOCK", vint(282));
    env_set_raw(e, "KeyboardKey.KEY_PRINT_SCREEN", vint(283));
    env_set_raw(e, "KeyboardKey.KEY_PAUSE", vint(284));
    env_set_raw(e, "KeyboardKey.KEY_F1", vint(290));
    env_set_raw(e, "KeyboardKey.KEY_F2", vint(291));
    env_set_raw(e, "KeyboardKey.KEY_F3", vint(292));
    env_set_raw(e, "KeyboardKey.KEY_F4", vint(293));
    env_set_raw(e, "KeyboardKey.KEY_F5", vint(294));
    env_set_raw(e, "KeyboardKey.KEY_F6", vint(295));
    env_set_raw(e, "KeyboardKey.KEY_F7", vint(296));
    env_set_raw(e, "KeyboardKey.KEY_F8", vint(297));
    env_set_raw(e, "KeyboardKey.KEY_F9", vint(298));
    env_set_raw(e, "KeyboardKey.KEY_F10", vint(299));
    env_set_raw(e, "KeyboardKey.KEY_F11", vint(300));
    env_set_raw(e, "KeyboardKey.KEY_F12", vint(301));
    env_set_raw(e, "KeyboardKey.KEY_LEFT_SHIFT", vint(340));
    env_set_raw(e, "KeyboardKey.KEY_LEFT_CONTROL", vint(341));
    env_set_raw(e, "KeyboardKey.KEY_LEFT_ALT", vint(342));
    env_set_raw(e, "KeyboardKey.KEY_LEFT_SUPER", vint(343));
    env_set_raw(e, "KeyboardKey.KEY_RIGHT_SHIFT", vint(344));
    env_set_raw(e, "KeyboardKey.KEY_RIGHT_CONTROL", vint(345));
    env_set_raw(e, "KeyboardKey.KEY_RIGHT_ALT", vint(346));
    env_set_raw(e, "KeyboardKey.KEY_RIGHT_SUPER", vint(347));
    env_set_raw(e, "KeyboardKey.KEY_KB_MENU", vint(348));
    env_set_raw(e, "KeyboardKey.KEY_KP_0", vint(320));
    env_set_raw(e, "KeyboardKey.KEY_KP_1", vint(321));
    env_set_raw(e, "KeyboardKey.KEY_KP_2", vint(322));
    env_set_raw(e, "KeyboardKey.KEY_KP_3", vint(323));
    env_set_raw(e, "KeyboardKey.KEY_KP_4", vint(324));
    env_set_raw(e, "KeyboardKey.KEY_KP_5", vint(325));
    env_set_raw(e, "KeyboardKey.KEY_KP_6", vint(326));
    env_set_raw(e, "KeyboardKey.KEY_KP_7", vint(327));
    env_set_raw(e, "KeyboardKey.KEY_KP_8", vint(328));
    env_set_raw(e, "KeyboardKey.KEY_KP_9", vint(329));
    env_set_raw(e, "KeyboardKey.KEY_KP_DECIMAL", vint(330));
    env_set_raw(e, "KeyboardKey.KEY_KP_DIVIDE", vint(331));
    env_set_raw(e, "KeyboardKey.KEY_KP_MULTIPLY", vint(332));
    env_set_raw(e, "KeyboardKey.KEY_KP_SUBTRACT", vint(333));
    env_set_raw(e, "KeyboardKey.KEY_KP_ADD", vint(334));
    env_set_raw(e, "KeyboardKey.KEY_KP_ENTER", vint(335));
    env_set_raw(e, "KeyboardKey.KEY_KP_EQUAL", vint(336));
    env_set_raw(e, "KeyboardKey.KEY_BACK", vint(4));
    env_set_raw(e, "KeyboardKey.KEY_MENU", vint(5));
    env_set_raw(e, "KeyboardKey.KEY_VOLUME_UP", vint(24));
    env_set_raw(e, "KeyboardKey.KEY_VOLUME_DOWN", vint(25));
}
