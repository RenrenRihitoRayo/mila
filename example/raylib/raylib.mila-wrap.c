#define ML_LIB
#include "mila.c"
#include "/home/renren/proj/c/mila/example/raylib/raylib.h"


Value* native_mila_InitWindow(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("InitWindow: wrong arg count");

    InitWindow(((int)argv[0]->v.i), ((int)argv[1]->v.i), argv[2]->v.s);
    return vnull();
}


Value* native_mila_CloseWindow(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("CloseWindow: wrong arg count");

    CloseWindow();
    return vnull();
}


Value* native_mila_WindowShouldClose(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("WindowShouldClose: wrong arg count");

    int res = WindowShouldClose();
    return vint((long)res);
}


Value* native_mila_IsWindowReady(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsWindowReady: wrong arg count");

    int res = IsWindowReady();
    return vint((long)res);
}


Value* native_mila_IsWindowFullscreen(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsWindowFullscreen: wrong arg count");

    int res = IsWindowFullscreen();
    return vint((long)res);
}


Value* native_mila_IsWindowHidden(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsWindowHidden: wrong arg count");

    int res = IsWindowHidden();
    return vint((long)res);
}


Value* native_mila_IsWindowMinimized(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsWindowMinimized: wrong arg count");

    int res = IsWindowMinimized();
    return vint((long)res);
}


Value* native_mila_IsWindowMaximized(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsWindowMaximized: wrong arg count");

    int res = IsWindowMaximized();
    return vint((long)res);
}


Value* native_mila_IsWindowFocused(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsWindowFocused: wrong arg count");

    int res = IsWindowFocused();
    return vint((long)res);
}


Value* native_mila_IsWindowResized(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsWindowResized: wrong arg count");

    int res = IsWindowResized();
    return vint((long)res);
}


Value* native_mila_IsWindowState(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsWindowState: wrong arg count");

    int res = IsWindowState(argv[0]->v.i);
    return vint((long)res);
}


Value* native_mila_SetWindowState(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetWindowState: wrong arg count");

    SetWindowState(argv[0]->v.i);
    return vnull();
}


Value* native_mila_ClearWindowState(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ClearWindowState: wrong arg count");

    ClearWindowState(argv[0]->v.i);
    return vnull();
}


Value* native_mila_ToggleFullscreen(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("ToggleFullscreen: wrong arg count");

    ToggleFullscreen();
    return vnull();
}


Value* native_mila_ToggleBorderlessWindowed(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("ToggleBorderlessWindowed: wrong arg count");

    ToggleBorderlessWindowed();
    return vnull();
}


Value* native_mila_MaximizeWindow(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("MaximizeWindow: wrong arg count");

    MaximizeWindow();
    return vnull();
}


Value* native_mila_MinimizeWindow(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("MinimizeWindow: wrong arg count");

    MinimizeWindow();
    return vnull();
}


Value* native_mila_RestoreWindow(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("RestoreWindow: wrong arg count");

    RestoreWindow();
    return vnull();
}


Value* native_mila_SetWindowIcon(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetWindowIcon: wrong arg count");

    SetWindowIcon(*((Image*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_SetWindowIcons(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetWindowIcons: wrong arg count");

    SetWindowIcons(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_SetWindowTitle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetWindowTitle: wrong arg count");

    SetWindowTitle(argv[0]->v.s);
    return vnull();
}


Value* native_mila_SetWindowPosition(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetWindowPosition: wrong arg count");

    SetWindowPosition(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_SetWindowMonitor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetWindowMonitor: wrong arg count");

    SetWindowMonitor(((int)argv[0]->v.i));
    return vnull();
}


Value* native_mila_SetWindowMinSize(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetWindowMinSize: wrong arg count");

    SetWindowMinSize(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_SetWindowMaxSize(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetWindowMaxSize: wrong arg count");

    SetWindowMaxSize(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_SetWindowSize(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetWindowSize: wrong arg count");

    SetWindowSize(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_SetWindowOpacity(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetWindowOpacity: wrong arg count");

    SetWindowOpacity(argv[0]->v.f);
    return vnull();
}


Value* native_mila_SetWindowFocused(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("SetWindowFocused: wrong arg count");

    SetWindowFocused();
    return vnull();
}


Value* native_mila_GetWindowHandle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetWindowHandle: wrong arg count");

    void * res = GetWindowHandle();
    return vopaque(res);
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


Value* native_mila_GetRenderWidth(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetRenderWidth: wrong arg count");

    int res = GetRenderWidth();
    return vint((long)res);
}


Value* native_mila_GetRenderHeight(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetRenderHeight: wrong arg count");

    int res = GetRenderHeight();
    return vint((long)res);
}


Value* native_mila_GetMonitorCount(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetMonitorCount: wrong arg count");

    int res = GetMonitorCount();
    return vint((long)res);
}


Value* native_mila_GetCurrentMonitor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetCurrentMonitor: wrong arg count");

    int res = GetCurrentMonitor();
    return vint((long)res);
}


Value* native_mila_GetMonitorPosition(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetMonitorPosition: wrong arg count");

    Vector2 res = GetMonitorPosition(((int)argv[0]->v.i));
    return vopaque(res);
}


Value* native_mila_GetMonitorWidth(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetMonitorWidth: wrong arg count");

    int res = GetMonitorWidth(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_GetMonitorHeight(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetMonitorHeight: wrong arg count");

    int res = GetMonitorHeight(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_GetMonitorPhysicalWidth(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetMonitorPhysicalWidth: wrong arg count");

    int res = GetMonitorPhysicalWidth(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_GetMonitorPhysicalHeight(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetMonitorPhysicalHeight: wrong arg count");

    int res = GetMonitorPhysicalHeight(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_GetMonitorRefreshRate(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetMonitorRefreshRate: wrong arg count");

    int res = GetMonitorRefreshRate(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_GetWindowPosition(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetWindowPosition: wrong arg count");

    Vector2 res = GetWindowPosition();
    return vopaque(res);
}


Value* native_mila_GetWindowScaleDPI(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetWindowScaleDPI: wrong arg count");

    Vector2 res = GetWindowScaleDPI();
    return vopaque(res);
}


Value* native_mila_GetMonitorName(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetMonitorName: wrong arg count");

    const char * res = GetMonitorName(((int)argv[0]->v.i));
    return vstring_take(res);
}


Value* native_mila_SetClipboardText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetClipboardText: wrong arg count");

    SetClipboardText(argv[0]->v.s);
    return vnull();
}


Value* native_mila_GetClipboardText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetClipboardText: wrong arg count");

    const char * res = GetClipboardText();
    return vstring_take(res);
}


Value* native_mila_GetClipboardImage(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetClipboardImage: wrong arg count");

    Image res = GetClipboardImage();
    return vopaque(res);
}


Value* native_mila_EnableEventWaiting(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("EnableEventWaiting: wrong arg count");

    EnableEventWaiting();
    return vnull();
}


Value* native_mila_DisableEventWaiting(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("DisableEventWaiting: wrong arg count");

    DisableEventWaiting();
    return vnull();
}


Value* native_mila_ShowCursor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("ShowCursor: wrong arg count");

    ShowCursor();
    return vnull();
}


Value* native_mila_HideCursor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("HideCursor: wrong arg count");

    HideCursor();
    return vnull();
}


Value* native_mila_IsCursorHidden(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsCursorHidden: wrong arg count");

    int res = IsCursorHidden();
    return vint((long)res);
}


Value* native_mila_EnableCursor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("EnableCursor: wrong arg count");

    EnableCursor();
    return vnull();
}


Value* native_mila_DisableCursor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("DisableCursor: wrong arg count");

    DisableCursor();
    return vnull();
}


Value* native_mila_IsCursorOnScreen(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsCursorOnScreen: wrong arg count");

    int res = IsCursorOnScreen();
    return vint((long)res);
}


Value* native_mila_ClearBackground(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ClearBackground: wrong arg count");

    ClearBackground(*((Color*)argv[0]->v.opaque));
    return vnull();
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


Value* native_mila_BeginMode2D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("BeginMode2D: wrong arg count");

    BeginMode2D(*((Camera2D*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_EndMode2D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("EndMode2D: wrong arg count");

    EndMode2D();
    return vnull();
}


Value* native_mila_BeginMode3D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("BeginMode3D: wrong arg count");

    BeginMode3D(*((Camera3D*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_EndMode3D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("EndMode3D: wrong arg count");

    EndMode3D();
    return vnull();
}


Value* native_mila_BeginTextureMode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("BeginTextureMode: wrong arg count");

    BeginTextureMode(*((RenderTexture*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_EndTextureMode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("EndTextureMode: wrong arg count");

    EndTextureMode();
    return vnull();
}


Value* native_mila_BeginShaderMode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("BeginShaderMode: wrong arg count");

    BeginShaderMode(*((Shader*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_EndShaderMode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("EndShaderMode: wrong arg count");

    EndShaderMode();
    return vnull();
}


Value* native_mila_BeginBlendMode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("BeginBlendMode: wrong arg count");

    BeginBlendMode(((int)argv[0]->v.i));
    return vnull();
}


Value* native_mila_EndBlendMode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("EndBlendMode: wrong arg count");

    EndBlendMode();
    return vnull();
}


Value* native_mila_BeginScissorMode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("BeginScissorMode: wrong arg count");

    BeginScissorMode(((int)argv[0]->v.i), ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i));
    return vnull();
}


Value* native_mila_EndScissorMode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("EndScissorMode: wrong arg count");

    EndScissorMode();
    return vnull();
}


Value* native_mila_BeginVrStereoMode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("BeginVrStereoMode: wrong arg count");

    BeginVrStereoMode(*((VrStereoConfig*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_EndVrStereoMode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("EndVrStereoMode: wrong arg count");

    EndVrStereoMode();
    return vnull();
}


Value* native_mila_LoadVrStereoConfig(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadVrStereoConfig: wrong arg count");

    VrStereoConfig res = LoadVrStereoConfig(*((VrDeviceInfo*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_UnloadVrStereoConfig(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadVrStereoConfig: wrong arg count");

    UnloadVrStereoConfig(*((VrStereoConfig*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_LoadShader(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("LoadShader: wrong arg count");

    Shader res = LoadShader(argv[0]->v.s, argv[1]->v.s);
    return vopaque(res);
}


Value* native_mila_LoadShaderFromMemory(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("LoadShaderFromMemory: wrong arg count");

    Shader res = LoadShaderFromMemory(argv[0]->v.s, argv[1]->v.s);
    return vopaque(res);
}


Value* native_mila_IsShaderValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsShaderValid: wrong arg count");

    int res = IsShaderValid(*((Shader*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_GetShaderLocation(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetShaderLocation: wrong arg count");

    int res = GetShaderLocation(*((Shader*)argv[0]->v.opaque), argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_GetShaderLocationAttrib(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetShaderLocationAttrib: wrong arg count");

    int res = GetShaderLocationAttrib(*((Shader*)argv[0]->v.opaque), argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_SetShaderValue(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("SetShaderValue: wrong arg count");

    SetShaderValue(*((Shader*)argv[0]->v.opaque), ((int)argv[1]->v.i), argv[2]->v.opaque, ((int)argv[3]->v.i));
    return vnull();
}


Value* native_mila_SetShaderValueV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("SetShaderValueV: wrong arg count");

    SetShaderValueV(*((Shader*)argv[0]->v.opaque), ((int)argv[1]->v.i), argv[2]->v.opaque, ((int)argv[3]->v.i), ((int)argv[4]->v.i));
    return vnull();
}


Value* native_mila_SetShaderValueMatrix(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("SetShaderValueMatrix: wrong arg count");

    SetShaderValueMatrix(*((Shader*)argv[0]->v.opaque), ((int)argv[1]->v.i), *((Matrix*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_SetShaderValueTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("SetShaderValueTexture: wrong arg count");

    SetShaderValueTexture(*((Shader*)argv[0]->v.opaque), ((int)argv[1]->v.i), *((Texture*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_UnloadShader(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadShader: wrong arg count");

    UnloadShader(*((Shader*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_GetScreenToWorldRay(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetScreenToWorldRay: wrong arg count");

    Ray res = GetScreenToWorldRay(*((Vector2*)argv[0]->v.opaque), *((Camera3D*)argv[1]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GetScreenToWorldRayEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("GetScreenToWorldRayEx: wrong arg count");

    Ray res = GetScreenToWorldRayEx(*((Vector2*)argv[0]->v.opaque), *((Camera3D*)argv[1]->v.opaque), ((int)argv[2]->v.i), ((int)argv[3]->v.i));
    return vopaque(res);
}


Value* native_mila_GetWorldToScreen(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetWorldToScreen: wrong arg count");

    Vector2 res = GetWorldToScreen(*((Vector3*)argv[0]->v.opaque), *((Camera3D*)argv[1]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GetWorldToScreenEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("GetWorldToScreenEx: wrong arg count");

    Vector2 res = GetWorldToScreenEx(*((Vector3*)argv[0]->v.opaque), *((Camera3D*)argv[1]->v.opaque), ((int)argv[2]->v.i), ((int)argv[3]->v.i));
    return vopaque(res);
}


Value* native_mila_GetWorldToScreen2D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetWorldToScreen2D: wrong arg count");

    Vector2 res = GetWorldToScreen2D(*((Vector2*)argv[0]->v.opaque), *((Camera2D*)argv[1]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GetScreenToWorld2D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetScreenToWorld2D: wrong arg count");

    Vector2 res = GetScreenToWorld2D(*((Vector2*)argv[0]->v.opaque), *((Camera2D*)argv[1]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GetCameraMatrix(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetCameraMatrix: wrong arg count");

    Matrix res = GetCameraMatrix(*((Camera3D*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GetCameraMatrix2D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetCameraMatrix2D: wrong arg count");

    Matrix res = GetCameraMatrix2D(*((Camera2D*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_SetTargetFPS(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetTargetFPS: wrong arg count");

    SetTargetFPS(((int)argv[0]->v.i));
    return vnull();
}


Value* native_mila_GetFrameTime(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetFrameTime: wrong arg count");

    float res = GetFrameTime();
    return vfloat((double)res);
}


Value* native_mila_GetTime(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetTime: wrong arg count");

    double res = GetTime();
    return vfloat(res);
}


Value* native_mila_GetFPS(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetFPS: wrong arg count");

    int res = GetFPS();
    return vint((long)res);
}


Value* native_mila_SwapScreenBuffer(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("SwapScreenBuffer: wrong arg count");

    SwapScreenBuffer();
    return vnull();
}


Value* native_mila_PollInputEvents(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("PollInputEvents: wrong arg count");

    PollInputEvents();
    return vnull();
}


Value* native_mila_WaitTime(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("WaitTime: wrong arg count");

    WaitTime(argv[0]->v.f);
    return vnull();
}


Value* native_mila_SetRandomSeed(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetRandomSeed: wrong arg count");

    SetRandomSeed(argv[0]->v.i);
    return vnull();
}


Value* native_mila_GetRandomValue(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetRandomValue: wrong arg count");

    int res = GetRandomValue(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vint((long)res);
}


Value* native_mila_LoadRandomSequence(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("LoadRandomSequence: wrong arg count");

    int * res = LoadRandomSequence(argv[0]->v.i, ((int)argv[1]->v.i), ((int)argv[2]->v.i));
    return vopaque(res);
}


Value* native_mila_UnloadRandomSequence(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadRandomSequence: wrong arg count");

    UnloadRandomSequence(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_TakeScreenshot(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("TakeScreenshot: wrong arg count");

    TakeScreenshot(argv[0]->v.s);
    return vnull();
}


Value* native_mila_SetConfigFlags(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetConfigFlags: wrong arg count");

    SetConfigFlags(argv[0]->v.i);
    return vnull();
}


Value* native_mila_OpenURL(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("OpenURL: wrong arg count");

    OpenURL(argv[0]->v.s);
    return vnull();
}


#include <ffi.h>

Value* native_mila_TraceLog(Env* e, int argc, Value** argv) {
    (void)e;

    if (argc < 1)
        return verror("TraceLog: missing required argument");

    ffi_cif cif;

    int fixed = 2;
    int total = argc;

    ffi_type *types[total];
    void *values[total];

    types[0] = &ffi_type_sint;
    values[0] = ((int*)&argv[0]->v.i);
    types[1] = &ffi_type_pointer;
    values[1] = &argv[1]->v.s;

    for (int i = fixed; i < argc; i++) {
        Value *v = argv[i];

        switch(v->type) {
            case T_INT:
                types[i] = &ffi_type_slong;
                values[i] = &v->v.i;
                break;

            case T_UINT:
                types[i] = &ffi_type_ulong;
                values[i] = &v->v.ui;
                break;

            case T_FLOAT:
                types[i] = &ffi_type_double;
                values[i] = &v->v.f;
                break;

            case T_BOOL:
                types[i] = &ffi_type_sint;
                values[i] = &v->v.b;
                break;

            case T_OWNED_OPAQUE:
            case T_OPAQUE:
                types[i] = &ffi_type_pointer;
                values[i] = &v->v.opaque;
                break;

            case T_STRING:
                types[i] = &ffi_type_pointer;
                values[i] = &v->v.s;
                break;

            default:
                types[i] = &ffi_type_pointer;
                values[i] = &v->v.opaque;
                break;
        }
    }

    

    ffi_prep_cif_var(
        &cif,
        FFI_DEFAULT_ABI,
        fixed,
        total,
        &ffi_type_sint,
        types
    );

    ffi_call(
        &cif,
        FFI_FN(TraceLog),
        NULL,
        values
    );

    return vnull();
}


Value* native_mila_SetTraceLogLevel(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetTraceLogLevel: wrong arg count");

    SetTraceLogLevel(((int)argv[0]->v.i));
    return vnull();
}


Value* native_mila_MemAlloc(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("MemAlloc: wrong arg count");

    void * res = MemAlloc(argv[0]->v.i);
    return vopaque(res);
}


Value* native_mila_MemRealloc(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("MemRealloc: wrong arg count");

    void * res = MemRealloc(argv[0]->v.opaque, argv[1]->v.i);
    return vopaque(res);
}


Value* native_mila_MemFree(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("MemFree: wrong arg count");

    MemFree(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_SetTraceLogCallback(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetTraceLogCallback: wrong arg count");

    SetTraceLogCallback(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_SetLoadFileDataCallback(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetLoadFileDataCallback: wrong arg count");

    SetLoadFileDataCallback(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_SetSaveFileDataCallback(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetSaveFileDataCallback: wrong arg count");

    SetSaveFileDataCallback(((int)argv[0]->v.i));
    return vnull();
}


Value* native_mila_SetLoadFileTextCallback(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetLoadFileTextCallback: wrong arg count");

    SetLoadFileTextCallback(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_SetSaveFileTextCallback(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetSaveFileTextCallback: wrong arg count");

    SetSaveFileTextCallback(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_LoadFileData(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("LoadFileData: wrong arg count");

    unsigned char * res = LoadFileData(argv[0]->v.s, argv[1]->v.opaque);
    return vopaque(res);
}


Value* native_mila_UnloadFileData(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadFileData: wrong arg count");

    UnloadFileData(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_SaveFileData(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("SaveFileData: wrong arg count");

    int res = SaveFileData(argv[0]->v.s, argv[1]->v.opaque, ((int)argv[2]->v.i));
    return vint((long)res);
}


Value* native_mila_ExportDataAsCode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ExportDataAsCode: wrong arg count");

    int res = ExportDataAsCode(argv[0]->v.opaque, ((int)argv[1]->v.i), argv[2]->v.s);
    return vint((long)res);
}


Value* native_mila_LoadFileText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadFileText: wrong arg count");

    char * res = LoadFileText(argv[0]->v.s);
    return vstring_dup(res);
}


Value* native_mila_UnloadFileText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadFileText: wrong arg count");

    UnloadFileText(argv[0]->v.s);
    return vnull();
}


Value* native_mila_SaveFileText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SaveFileText: wrong arg count");

    int res = SaveFileText(argv[0]->v.s, argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_FileExists(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("FileExists: wrong arg count");

    int res = FileExists(argv[0]->v.s);
    return vint((long)res);
}


Value* native_mila_DirectoryExists(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("DirectoryExists: wrong arg count");

    int res = DirectoryExists(argv[0]->v.s);
    return vint((long)res);
}


Value* native_mila_IsFileExtension(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("IsFileExtension: wrong arg count");

    int res = IsFileExtension(argv[0]->v.s, argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_GetFileLength(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetFileLength: wrong arg count");

    int res = GetFileLength(argv[0]->v.s);
    return vint((long)res);
}


Value* native_mila_GetFileExtension(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetFileExtension: wrong arg count");

    const char * res = GetFileExtension(argv[0]->v.s);
    return vstring_take(res);
}


Value* native_mila_GetFileName(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetFileName: wrong arg count");

    const char * res = GetFileName(argv[0]->v.s);
    return vstring_take(res);
}


Value* native_mila_GetFileNameWithoutExt(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetFileNameWithoutExt: wrong arg count");

    const char * res = GetFileNameWithoutExt(argv[0]->v.s);
    return vstring_take(res);
}


Value* native_mila_GetDirectoryPath(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetDirectoryPath: wrong arg count");

    const char * res = GetDirectoryPath(argv[0]->v.s);
    return vstring_take(res);
}


Value* native_mila_GetPrevDirectoryPath(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetPrevDirectoryPath: wrong arg count");

    const char * res = GetPrevDirectoryPath(argv[0]->v.s);
    return vstring_take(res);
}


Value* native_mila_GetWorkingDirectory(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetWorkingDirectory: wrong arg count");

    const char * res = GetWorkingDirectory();
    return vstring_take(res);
}


Value* native_mila_GetApplicationDirectory(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetApplicationDirectory: wrong arg count");

    const char * res = GetApplicationDirectory();
    return vstring_take(res);
}


Value* native_mila_MakeDirectory(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("MakeDirectory: wrong arg count");

    int res = MakeDirectory(argv[0]->v.s);
    return vint((long)res);
}


Value* native_mila_ChangeDirectory(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ChangeDirectory: wrong arg count");

    int res = ChangeDirectory(argv[0]->v.s);
    return vint((long)res);
}


Value* native_mila_IsPathFile(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsPathFile: wrong arg count");

    int res = IsPathFile(argv[0]->v.s);
    return vint((long)res);
}


Value* native_mila_IsFileNameValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsFileNameValid: wrong arg count");

    int res = IsFileNameValid(argv[0]->v.s);
    return vint((long)res);
}


Value* native_mila_LoadDirectoryFiles(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadDirectoryFiles: wrong arg count");

    FilePathList res = LoadDirectoryFiles(argv[0]->v.s);
    return vopaque(res);
}


Value* native_mila_LoadDirectoryFilesEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("LoadDirectoryFilesEx: wrong arg count");

    FilePathList res = LoadDirectoryFilesEx(argv[0]->v.s, argv[1]->v.s, argv[2]->v.opaque);
    return vopaque(res);
}


Value* native_mila_UnloadDirectoryFiles(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadDirectoryFiles: wrong arg count");

    UnloadDirectoryFiles(*((FilePathList*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_IsFileDropped(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsFileDropped: wrong arg count");

    int res = IsFileDropped();
    return vint((long)res);
}


Value* native_mila_LoadDroppedFiles(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("LoadDroppedFiles: wrong arg count");

    FilePathList res = LoadDroppedFiles();
    return vopaque(res);
}


Value* native_mila_UnloadDroppedFiles(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadDroppedFiles: wrong arg count");

    UnloadDroppedFiles(*((FilePathList*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_GetFileModTime(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetFileModTime: wrong arg count");

    long res = GetFileModTime(argv[0]->v.s);
    return vint(res);
}


Value* native_mila_CompressData(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("CompressData: wrong arg count");

    unsigned char * res = CompressData(argv[0]->v.opaque, ((int)argv[1]->v.i), argv[2]->v.opaque);
    return vopaque(res);
}


Value* native_mila_DecompressData(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DecompressData: wrong arg count");

    unsigned char * res = DecompressData(argv[0]->v.opaque, ((int)argv[1]->v.i), argv[2]->v.opaque);
    return vopaque(res);
}


Value* native_mila_EncodeDataBase64(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("EncodeDataBase64: wrong arg count");

    char * res = EncodeDataBase64(argv[0]->v.opaque, ((int)argv[1]->v.i), argv[2]->v.opaque);
    return vstring_dup(res);
}


Value* native_mila_DecodeDataBase64(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("DecodeDataBase64: wrong arg count");

    unsigned char * res = DecodeDataBase64(argv[0]->v.opaque, argv[1]->v.opaque);
    return vopaque(res);
}


Value* native_mila_ComputeCRC32(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ComputeCRC32: wrong arg count");

    unsigned int res = ComputeCRC32(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vint(res);
}


Value* native_mila_ComputeMD5(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ComputeMD5: wrong arg count");

    unsigned int * res = ComputeMD5(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vopaque(res);
}


Value* native_mila_ComputeSHA1(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ComputeSHA1: wrong arg count");

    unsigned int * res = ComputeSHA1(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vopaque(res);
}


Value* native_mila_LoadAutomationEventList(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadAutomationEventList: wrong arg count");

    AutomationEventList res = LoadAutomationEventList(argv[0]->v.s);
    return vopaque(res);
}


Value* native_mila_UnloadAutomationEventList(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadAutomationEventList: wrong arg count");

    UnloadAutomationEventList(*((AutomationEventList*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_ExportAutomationEventList(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ExportAutomationEventList: wrong arg count");

    int res = ExportAutomationEventList(*((AutomationEventList*)argv[0]->v.opaque), argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_SetAutomationEventList(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetAutomationEventList: wrong arg count");

    SetAutomationEventList(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_SetAutomationEventBaseFrame(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetAutomationEventBaseFrame: wrong arg count");

    SetAutomationEventBaseFrame(((int)argv[0]->v.i));
    return vnull();
}


Value* native_mila_StartAutomationEventRecording(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("StartAutomationEventRecording: wrong arg count");

    StartAutomationEventRecording();
    return vnull();
}


Value* native_mila_StopAutomationEventRecording(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("StopAutomationEventRecording: wrong arg count");

    StopAutomationEventRecording();
    return vnull();
}


Value* native_mila_PlayAutomationEvent(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("PlayAutomationEvent: wrong arg count");

    PlayAutomationEvent(*((AutomationEvent*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_IsKeyPressed(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsKeyPressed: wrong arg count");

    int res = IsKeyPressed(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_IsKeyPressedRepeat(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsKeyPressedRepeat: wrong arg count");

    int res = IsKeyPressedRepeat(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_IsKeyDown(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsKeyDown: wrong arg count");

    int res = IsKeyDown(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_IsKeyReleased(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsKeyReleased: wrong arg count");

    int res = IsKeyReleased(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_IsKeyUp(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsKeyUp: wrong arg count");

    int res = IsKeyUp(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_GetKeyPressed(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetKeyPressed: wrong arg count");

    int res = GetKeyPressed();
    return vint((long)res);
}


Value* native_mila_GetCharPressed(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetCharPressed: wrong arg count");

    int res = GetCharPressed();
    return vint((long)res);
}


Value* native_mila_SetExitKey(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetExitKey: wrong arg count");

    SetExitKey(((int)argv[0]->v.i));
    return vnull();
}


Value* native_mila_IsGamepadAvailable(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsGamepadAvailable: wrong arg count");

    int res = IsGamepadAvailable(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_GetGamepadName(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetGamepadName: wrong arg count");

    const char * res = GetGamepadName(((int)argv[0]->v.i));
    return vstring_take(res);
}


Value* native_mila_IsGamepadButtonPressed(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("IsGamepadButtonPressed: wrong arg count");

    int res = IsGamepadButtonPressed(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vint((long)res);
}


Value* native_mila_IsGamepadButtonDown(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("IsGamepadButtonDown: wrong arg count");

    int res = IsGamepadButtonDown(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vint((long)res);
}


Value* native_mila_IsGamepadButtonReleased(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("IsGamepadButtonReleased: wrong arg count");

    int res = IsGamepadButtonReleased(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vint((long)res);
}


Value* native_mila_IsGamepadButtonUp(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("IsGamepadButtonUp: wrong arg count");

    int res = IsGamepadButtonUp(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vint((long)res);
}


Value* native_mila_GetGamepadButtonPressed(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetGamepadButtonPressed: wrong arg count");

    int res = GetGamepadButtonPressed();
    return vint((long)res);
}


Value* native_mila_GetGamepadAxisCount(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetGamepadAxisCount: wrong arg count");

    int res = GetGamepadAxisCount(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_GetGamepadAxisMovement(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetGamepadAxisMovement: wrong arg count");

    float res = GetGamepadAxisMovement(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vfloat((double)res);
}


Value* native_mila_SetGamepadMappings(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetGamepadMappings: wrong arg count");

    int res = SetGamepadMappings(argv[0]->v.s);
    return vint((long)res);
}


Value* native_mila_SetGamepadVibration(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("SetGamepadVibration: wrong arg count");

    SetGamepadVibration(((int)argv[0]->v.i), argv[1]->v.f, argv[2]->v.f, argv[3]->v.f);
    return vnull();
}


Value* native_mila_IsMouseButtonPressed(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsMouseButtonPressed: wrong arg count");

    int res = IsMouseButtonPressed(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_IsMouseButtonDown(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsMouseButtonDown: wrong arg count");

    int res = IsMouseButtonDown(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_IsMouseButtonReleased(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsMouseButtonReleased: wrong arg count");

    int res = IsMouseButtonReleased(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_IsMouseButtonUp(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsMouseButtonUp: wrong arg count");

    int res = IsMouseButtonUp(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_GetMouseX(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetMouseX: wrong arg count");

    int res = GetMouseX();
    return vint((long)res);
}


Value* native_mila_GetMouseY(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetMouseY: wrong arg count");

    int res = GetMouseY();
    return vint((long)res);
}


Value* native_mila_GetMousePosition(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetMousePosition: wrong arg count");

    Vector2 res = GetMousePosition();
    return vopaque(res);
}


Value* native_mila_GetMouseDelta(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetMouseDelta: wrong arg count");

    Vector2 res = GetMouseDelta();
    return vopaque(res);
}


Value* native_mila_SetMousePosition(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetMousePosition: wrong arg count");

    SetMousePosition(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_SetMouseOffset(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetMouseOffset: wrong arg count");

    SetMouseOffset(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_SetMouseScale(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetMouseScale: wrong arg count");

    SetMouseScale(argv[0]->v.f, argv[1]->v.f);
    return vnull();
}


Value* native_mila_GetMouseWheelMove(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetMouseWheelMove: wrong arg count");

    float res = GetMouseWheelMove();
    return vfloat((double)res);
}


Value* native_mila_GetMouseWheelMoveV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetMouseWheelMoveV: wrong arg count");

    Vector2 res = GetMouseWheelMoveV();
    return vopaque(res);
}


Value* native_mila_SetMouseCursor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetMouseCursor: wrong arg count");

    SetMouseCursor(((int)argv[0]->v.i));
    return vnull();
}


Value* native_mila_GetTouchX(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetTouchX: wrong arg count");

    int res = GetTouchX();
    return vint((long)res);
}


Value* native_mila_GetTouchY(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetTouchY: wrong arg count");

    int res = GetTouchY();
    return vint((long)res);
}


Value* native_mila_GetTouchPosition(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetTouchPosition: wrong arg count");

    Vector2 res = GetTouchPosition(((int)argv[0]->v.i));
    return vopaque(res);
}


Value* native_mila_GetTouchPointId(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetTouchPointId: wrong arg count");

    int res = GetTouchPointId(((int)argv[0]->v.i));
    return vint((long)res);
}


Value* native_mila_GetTouchPointCount(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetTouchPointCount: wrong arg count");

    int res = GetTouchPointCount();
    return vint((long)res);
}


Value* native_mila_SetGesturesEnabled(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetGesturesEnabled: wrong arg count");

    SetGesturesEnabled(argv[0]->v.i);
    return vnull();
}


Value* native_mila_IsGestureDetected(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsGestureDetected: wrong arg count");

    int res = IsGestureDetected(argv[0]->v.i);
    return vint((long)res);
}


Value* native_mila_GetGestureDetected(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetGestureDetected: wrong arg count");

    int res = GetGestureDetected();
    return vint((long)res);
}


Value* native_mila_GetGestureHoldDuration(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetGestureHoldDuration: wrong arg count");

    float res = GetGestureHoldDuration();
    return vfloat((double)res);
}


Value* native_mila_GetGestureDragVector(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetGestureDragVector: wrong arg count");

    Vector2 res = GetGestureDragVector();
    return vopaque(res);
}


Value* native_mila_GetGestureDragAngle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetGestureDragAngle: wrong arg count");

    float res = GetGestureDragAngle();
    return vfloat((double)res);
}


Value* native_mila_GetGesturePinchVector(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetGesturePinchVector: wrong arg count");

    Vector2 res = GetGesturePinchVector();
    return vopaque(res);
}


Value* native_mila_GetGesturePinchAngle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetGesturePinchAngle: wrong arg count");

    float res = GetGesturePinchAngle();
    return vfloat((double)res);
}


Value* native_mila_UpdateCamera(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("UpdateCamera: wrong arg count");

    UpdateCamera(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_UpdateCameraPro(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("UpdateCameraPro: wrong arg count");

    UpdateCameraPro(argv[0]->v.opaque, *((Vector3*)argv[1]->v.opaque), *((Vector3*)argv[2]->v.opaque), argv[3]->v.f);
    return vnull();
}


Value* native_mila_SetShapesTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetShapesTexture: wrong arg count");

    SetShapesTexture(*((Texture*)argv[0]->v.opaque), *((Rectangle*)argv[1]->v.opaque));
    return vnull();
}


Value* native_mila_GetShapesTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetShapesTexture: wrong arg count");

    Texture2D res = GetShapesTexture();
    return vopaque(res);
}


Value* native_mila_GetShapesTextureRectangle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetShapesTextureRectangle: wrong arg count");

    Rectangle res = GetShapesTextureRectangle();
    return vopaque(res);
}


Value* native_mila_DrawPixel(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawPixel: wrong arg count");

    DrawPixel(((int)argv[0]->v.i), ((int)argv[1]->v.i), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawPixelV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("DrawPixelV: wrong arg count");

    DrawPixelV(*((Vector2*)argv[0]->v.opaque), *((Color*)argv[1]->v.opaque));
    return vnull();
}


Value* native_mila_DrawLine(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawLine: wrong arg count");

    DrawLine(((int)argv[0]->v.i), ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawLineV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawLineV: wrong arg count");

    DrawLineV(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawLineEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawLineEx: wrong arg count");

    DrawLineEx(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawLineStrip(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawLineStrip: wrong arg count");

    DrawLineStrip(argv[0]->v.opaque, ((int)argv[1]->v.i), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawLineBezier(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawLineBezier: wrong arg count");

    DrawLineBezier(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCircle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawCircle: wrong arg count");

    DrawCircle(((int)argv[0]->v.i), ((int)argv[1]->v.i), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCircleSector(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawCircleSector: wrong arg count");

    DrawCircleSector(*((Vector2*)argv[0]->v.opaque), argv[1]->v.f, argv[2]->v.f, argv[3]->v.f, ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCircleSectorLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawCircleSectorLines: wrong arg count");

    DrawCircleSectorLines(*((Vector2*)argv[0]->v.opaque), argv[1]->v.f, argv[2]->v.f, argv[3]->v.f, ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCircleGradient(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawCircleGradient: wrong arg count");

    DrawCircleGradient(((int)argv[0]->v.i), ((int)argv[1]->v.i), argv[2]->v.f, *((Color*)argv[3]->v.opaque), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCircleV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawCircleV: wrong arg count");

    DrawCircleV(*((Vector2*)argv[0]->v.opaque), argv[1]->v.f, *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCircleLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawCircleLines: wrong arg count");

    DrawCircleLines(((int)argv[0]->v.i), ((int)argv[1]->v.i), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCircleLinesV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawCircleLinesV: wrong arg count");

    DrawCircleLinesV(*((Vector2*)argv[0]->v.opaque), argv[1]->v.f, *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawEllipse(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawEllipse: wrong arg count");

    DrawEllipse(((int)argv[0]->v.i), ((int)argv[1]->v.i), argv[2]->v.f, argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawEllipseLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawEllipseLines: wrong arg count");

    DrawEllipseLines(((int)argv[0]->v.i), ((int)argv[1]->v.i), argv[2]->v.f, argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRing(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 7)
        return verror("DrawRing: wrong arg count");

    DrawRing(*((Vector2*)argv[0]->v.opaque), argv[1]->v.f, argv[2]->v.f, argv[3]->v.f, argv[4]->v.f, ((int)argv[5]->v.i), *((Color*)argv[6]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRingLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 7)
        return verror("DrawRingLines: wrong arg count");

    DrawRingLines(*((Vector2*)argv[0]->v.opaque), argv[1]->v.f, argv[2]->v.f, argv[3]->v.f, argv[4]->v.f, ((int)argv[5]->v.i), *((Color*)argv[6]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectangle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawRectangle: wrong arg count");

    DrawRectangle(((int)argv[0]->v.i), ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectangleV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawRectangleV: wrong arg count");

    DrawRectangleV(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectangleRec(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("DrawRectangleRec: wrong arg count");

    DrawRectangleRec(*((Rectangle*)argv[0]->v.opaque), *((Color*)argv[1]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectanglePro(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawRectanglePro: wrong arg count");

    DrawRectanglePro(*((Rectangle*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectangleGradientV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawRectangleGradientV: wrong arg count");

    DrawRectangleGradientV(((int)argv[0]->v.i), ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectangleGradientH(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawRectangleGradientH: wrong arg count");

    DrawRectangleGradientH(((int)argv[0]->v.i), ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectangleGradientEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawRectangleGradientEx: wrong arg count");

    DrawRectangleGradientEx(*((Rectangle*)argv[0]->v.opaque), *((Color*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque), *((Color*)argv[3]->v.opaque), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectangleLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawRectangleLines: wrong arg count");

    DrawRectangleLines(((int)argv[0]->v.i), ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectangleLinesEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawRectangleLinesEx: wrong arg count");

    DrawRectangleLinesEx(*((Rectangle*)argv[0]->v.opaque), argv[1]->v.f, *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectangleRounded(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawRectangleRounded: wrong arg count");

    DrawRectangleRounded(*((Rectangle*)argv[0]->v.opaque), argv[1]->v.f, ((int)argv[2]->v.i), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectangleRoundedLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawRectangleRoundedLines: wrong arg count");

    DrawRectangleRoundedLines(*((Rectangle*)argv[0]->v.opaque), argv[1]->v.f, ((int)argv[2]->v.i), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRectangleRoundedLinesEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawRectangleRoundedLinesEx: wrong arg count");

    DrawRectangleRoundedLinesEx(*((Rectangle*)argv[0]->v.opaque), argv[1]->v.f, ((int)argv[2]->v.i), argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTriangle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawTriangle: wrong arg count");

    DrawTriangle(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTriangleLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawTriangleLines: wrong arg count");

    DrawTriangleLines(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTriangleFan(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawTriangleFan: wrong arg count");

    DrawTriangleFan(argv[0]->v.opaque, ((int)argv[1]->v.i), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTriangleStrip(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawTriangleStrip: wrong arg count");

    DrawTriangleStrip(argv[0]->v.opaque, ((int)argv[1]->v.i), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawPoly(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawPoly: wrong arg count");

    DrawPoly(*((Vector2*)argv[0]->v.opaque), ((int)argv[1]->v.i), argv[2]->v.f, argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawPolyLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawPolyLines: wrong arg count");

    DrawPolyLines(*((Vector2*)argv[0]->v.opaque), ((int)argv[1]->v.i), argv[2]->v.f, argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawPolyLinesEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawPolyLinesEx: wrong arg count");

    DrawPolyLinesEx(*((Vector2*)argv[0]->v.opaque), ((int)argv[1]->v.i), argv[2]->v.f, argv[3]->v.f, argv[4]->v.f, *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSplineLinear(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawSplineLinear: wrong arg count");

    DrawSplineLinear(argv[0]->v.opaque, ((int)argv[1]->v.i), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSplineBasis(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawSplineBasis: wrong arg count");

    DrawSplineBasis(argv[0]->v.opaque, ((int)argv[1]->v.i), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSplineCatmullRom(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawSplineCatmullRom: wrong arg count");

    DrawSplineCatmullRom(argv[0]->v.opaque, ((int)argv[1]->v.i), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSplineBezierQuadratic(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawSplineBezierQuadratic: wrong arg count");

    DrawSplineBezierQuadratic(argv[0]->v.opaque, ((int)argv[1]->v.i), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSplineBezierCubic(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawSplineBezierCubic: wrong arg count");

    DrawSplineBezierCubic(argv[0]->v.opaque, ((int)argv[1]->v.i), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSplineSegmentLinear(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawSplineSegmentLinear: wrong arg count");

    DrawSplineSegmentLinear(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSplineSegmentBasis(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawSplineSegmentBasis: wrong arg count");

    DrawSplineSegmentBasis(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), argv[4]->v.f, *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSplineSegmentCatmullRom(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawSplineSegmentCatmullRom: wrong arg count");

    DrawSplineSegmentCatmullRom(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), argv[4]->v.f, *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSplineSegmentBezierQuadratic(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawSplineSegmentBezierQuadratic: wrong arg count");

    DrawSplineSegmentBezierQuadratic(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSplineSegmentBezierCubic(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawSplineSegmentBezierCubic: wrong arg count");

    DrawSplineSegmentBezierCubic(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), argv[4]->v.f, *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_GetSplinePointLinear(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GetSplinePointLinear: wrong arg count");

    Vector2 res = GetSplinePointLinear(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), argv[2]->v.f);
    return vopaque(res);
}


Value* native_mila_GetSplinePointBasis(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("GetSplinePointBasis: wrong arg count");

    Vector2 res = GetSplinePointBasis(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), argv[4]->v.f);
    return vopaque(res);
}


Value* native_mila_GetSplinePointCatmullRom(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("GetSplinePointCatmullRom: wrong arg count");

    Vector2 res = GetSplinePointCatmullRom(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), argv[4]->v.f);
    return vopaque(res);
}


Value* native_mila_GetSplinePointBezierQuad(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("GetSplinePointBezierQuad: wrong arg count");

    Vector2 res = GetSplinePointBezierQuad(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), argv[3]->v.f);
    return vopaque(res);
}


Value* native_mila_GetSplinePointBezierCubic(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("GetSplinePointBezierCubic: wrong arg count");

    Vector2 res = GetSplinePointBezierCubic(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), argv[4]->v.f);
    return vopaque(res);
}


Value* native_mila_CheckCollisionRecs(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("CheckCollisionRecs: wrong arg count");

    int res = CheckCollisionRecs(*((Rectangle*)argv[0]->v.opaque), *((Rectangle*)argv[1]->v.opaque));
    return vint((long)res);
}


Value* native_mila_CheckCollisionCircles(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("CheckCollisionCircles: wrong arg count");

    int res = CheckCollisionCircles(*((Vector2*)argv[0]->v.opaque), argv[1]->v.f, *((Vector2*)argv[2]->v.opaque), argv[3]->v.f);
    return vint((long)res);
}


Value* native_mila_CheckCollisionCircleRec(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("CheckCollisionCircleRec: wrong arg count");

    int res = CheckCollisionCircleRec(*((Vector2*)argv[0]->v.opaque), argv[1]->v.f, *((Rectangle*)argv[2]->v.opaque));
    return vint((long)res);
}


Value* native_mila_CheckCollisionCircleLine(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("CheckCollisionCircleLine: wrong arg count");

    int res = CheckCollisionCircleLine(*((Vector2*)argv[0]->v.opaque), argv[1]->v.f, *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque));
    return vint((long)res);
}


Value* native_mila_CheckCollisionPointRec(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("CheckCollisionPointRec: wrong arg count");

    int res = CheckCollisionPointRec(*((Vector2*)argv[0]->v.opaque), *((Rectangle*)argv[1]->v.opaque));
    return vint((long)res);
}


Value* native_mila_CheckCollisionPointCircle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("CheckCollisionPointCircle: wrong arg count");

    int res = CheckCollisionPointCircle(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), argv[2]->v.f);
    return vint((long)res);
}


Value* native_mila_CheckCollisionPointTriangle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("CheckCollisionPointTriangle: wrong arg count");

    int res = CheckCollisionPointTriangle(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque));
    return vint((long)res);
}


Value* native_mila_CheckCollisionPointLine(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("CheckCollisionPointLine: wrong arg count");

    int res = CheckCollisionPointLine(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), ((int)argv[3]->v.i));
    return vint((long)res);
}


Value* native_mila_CheckCollisionPointPoly(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("CheckCollisionPointPoly: wrong arg count");

    int res = CheckCollisionPointPoly(*((Vector2*)argv[0]->v.opaque), argv[1]->v.opaque, ((int)argv[2]->v.i));
    return vint((long)res);
}


Value* native_mila_CheckCollisionLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("CheckCollisionLines: wrong arg count");

    int res = CheckCollisionLines(*((Vector2*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), argv[4]->v.opaque);
    return vint((long)res);
}


Value* native_mila_GetCollisionRec(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetCollisionRec: wrong arg count");

    Rectangle res = GetCollisionRec(*((Rectangle*)argv[0]->v.opaque), *((Rectangle*)argv[1]->v.opaque));
    return vopaque(res);
}


Value* native_mila_LoadImage(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadImage: wrong arg count");

    Image res = LoadImage(argv[0]->v.s);
    return vopaque(res);
}


Value* native_mila_LoadImageRaw(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("LoadImageRaw: wrong arg count");

    Image res = LoadImageRaw(argv[0]->v.s, ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), ((int)argv[4]->v.i));
    return vopaque(res);
}


Value* native_mila_LoadImageAnim(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("LoadImageAnim: wrong arg count");

    Image res = LoadImageAnim(argv[0]->v.s, argv[1]->v.opaque);
    return vopaque(res);
}


Value* native_mila_LoadImageAnimFromMemory(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("LoadImageAnimFromMemory: wrong arg count");

    Image res = LoadImageAnimFromMemory(argv[0]->v.s, argv[1]->v.opaque, ((int)argv[2]->v.i), argv[3]->v.opaque);
    return vopaque(res);
}


Value* native_mila_LoadImageFromMemory(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("LoadImageFromMemory: wrong arg count");

    Image res = LoadImageFromMemory(argv[0]->v.s, argv[1]->v.opaque, ((int)argv[2]->v.i));
    return vopaque(res);
}


Value* native_mila_LoadImageFromTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadImageFromTexture: wrong arg count");

    Image res = LoadImageFromTexture(*((Texture*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_LoadImageFromScreen(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("LoadImageFromScreen: wrong arg count");

    Image res = LoadImageFromScreen();
    return vopaque(res);
}


Value* native_mila_IsImageValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsImageValid: wrong arg count");

    int res = IsImageValid(*((Image*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_UnloadImage(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadImage: wrong arg count");

    UnloadImage(*((Image*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_ExportImage(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ExportImage: wrong arg count");

    int res = ExportImage(*((Image*)argv[0]->v.opaque), argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_ExportImageToMemory(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ExportImageToMemory: wrong arg count");

    unsigned char * res = ExportImageToMemory(*((Image*)argv[0]->v.opaque), argv[1]->v.s, argv[2]->v.opaque);
    return vopaque(res);
}


Value* native_mila_ExportImageAsCode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ExportImageAsCode: wrong arg count");

    int res = ExportImageAsCode(*((Image*)argv[0]->v.opaque), argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_GenImageColor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GenImageColor: wrong arg count");

    Image res = GenImageColor(((int)argv[0]->v.i), ((int)argv[1]->v.i), *((Color*)argv[2]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GenImageGradientLinear(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("GenImageGradientLinear: wrong arg count");

    Image res = GenImageGradientLinear(((int)argv[0]->v.i), ((int)argv[1]->v.i), ((int)argv[2]->v.i), *((Color*)argv[3]->v.opaque), *((Color*)argv[4]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GenImageGradientRadial(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("GenImageGradientRadial: wrong arg count");

    Image res = GenImageGradientRadial(((int)argv[0]->v.i), ((int)argv[1]->v.i), argv[2]->v.f, *((Color*)argv[3]->v.opaque), *((Color*)argv[4]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GenImageGradientSquare(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("GenImageGradientSquare: wrong arg count");

    Image res = GenImageGradientSquare(((int)argv[0]->v.i), ((int)argv[1]->v.i), argv[2]->v.f, *((Color*)argv[3]->v.opaque), *((Color*)argv[4]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GenImageChecked(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("GenImageChecked: wrong arg count");

    Image res = GenImageChecked(((int)argv[0]->v.i), ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque), *((Color*)argv[5]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GenImageWhiteNoise(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GenImageWhiteNoise: wrong arg count");

    Image res = GenImageWhiteNoise(((int)argv[0]->v.i), ((int)argv[1]->v.i), argv[2]->v.f);
    return vopaque(res);
}


Value* native_mila_GenImagePerlinNoise(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("GenImagePerlinNoise: wrong arg count");

    Image res = GenImagePerlinNoise(((int)argv[0]->v.i), ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), argv[4]->v.f);
    return vopaque(res);
}


Value* native_mila_GenImageCellular(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GenImageCellular: wrong arg count");

    Image res = GenImageCellular(((int)argv[0]->v.i), ((int)argv[1]->v.i), ((int)argv[2]->v.i));
    return vopaque(res);
}


Value* native_mila_GenImageText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GenImageText: wrong arg count");

    Image res = GenImageText(((int)argv[0]->v.i), ((int)argv[1]->v.i), argv[2]->v.s);
    return vopaque(res);
}


Value* native_mila_ImageCopy(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ImageCopy: wrong arg count");

    Image res = ImageCopy(*((Image*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_ImageFromImage(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageFromImage: wrong arg count");

    Image res = ImageFromImage(*((Image*)argv[0]->v.opaque), *((Rectangle*)argv[1]->v.opaque));
    return vopaque(res);
}


Value* native_mila_ImageFromChannel(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageFromChannel: wrong arg count");

    Image res = ImageFromChannel(*((Image*)argv[0]->v.opaque), ((int)argv[1]->v.i));
    return vopaque(res);
}


Value* native_mila_ImageText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ImageText: wrong arg count");

    Image res = ImageText(argv[0]->v.s, ((int)argv[1]->v.i), *((Color*)argv[2]->v.opaque));
    return vopaque(res);
}


Value* native_mila_ImageTextEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("ImageTextEx: wrong arg count");

    Image res = ImageTextEx(*((Font*)argv[0]->v.opaque), argv[1]->v.s, argv[2]->v.f, argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vopaque(res);
}


Value* native_mila_ImageFormat(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageFormat: wrong arg count");

    ImageFormat(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_ImageToPOT(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageToPOT: wrong arg count");

    ImageToPOT(argv[0]->v.opaque, *((Color*)argv[1]->v.opaque));
    return vnull();
}


Value* native_mila_ImageCrop(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageCrop: wrong arg count");

    ImageCrop(argv[0]->v.opaque, *((Rectangle*)argv[1]->v.opaque));
    return vnull();
}


Value* native_mila_ImageAlphaCrop(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageAlphaCrop: wrong arg count");

    ImageAlphaCrop(argv[0]->v.opaque, argv[1]->v.f);
    return vnull();
}


Value* native_mila_ImageAlphaClear(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ImageAlphaClear: wrong arg count");

    ImageAlphaClear(argv[0]->v.opaque, *((Color*)argv[1]->v.opaque), argv[2]->v.f);
    return vnull();
}


Value* native_mila_ImageAlphaMask(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageAlphaMask: wrong arg count");

    ImageAlphaMask(argv[0]->v.opaque, *((Image*)argv[1]->v.opaque));
    return vnull();
}


Value* native_mila_ImageAlphaPremultiply(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ImageAlphaPremultiply: wrong arg count");

    ImageAlphaPremultiply(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_ImageBlurGaussian(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageBlurGaussian: wrong arg count");

    ImageBlurGaussian(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_ImageKernelConvolution(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ImageKernelConvolution: wrong arg count");

    ImageKernelConvolution(argv[0]->v.opaque, argv[1]->v.opaque, ((int)argv[2]->v.i));
    return vnull();
}


Value* native_mila_ImageResize(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ImageResize: wrong arg count");

    ImageResize(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i));
    return vnull();
}


Value* native_mila_ImageResizeNN(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ImageResizeNN: wrong arg count");

    ImageResizeNN(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i));
    return vnull();
}


Value* native_mila_ImageResizeCanvas(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("ImageResizeCanvas: wrong arg count");

    ImageResizeCanvas(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_ImageMipmaps(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ImageMipmaps: wrong arg count");

    ImageMipmaps(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_ImageDither(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("ImageDither: wrong arg count");

    ImageDither(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), ((int)argv[4]->v.i));
    return vnull();
}


Value* native_mila_ImageFlipVertical(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ImageFlipVertical: wrong arg count");

    ImageFlipVertical(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_ImageFlipHorizontal(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ImageFlipHorizontal: wrong arg count");

    ImageFlipHorizontal(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_ImageRotate(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageRotate: wrong arg count");

    ImageRotate(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_ImageRotateCW(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ImageRotateCW: wrong arg count");

    ImageRotateCW(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_ImageRotateCCW(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ImageRotateCCW: wrong arg count");

    ImageRotateCCW(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_ImageColorTint(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageColorTint: wrong arg count");

    ImageColorTint(argv[0]->v.opaque, *((Color*)argv[1]->v.opaque));
    return vnull();
}


Value* native_mila_ImageColorInvert(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ImageColorInvert: wrong arg count");

    ImageColorInvert(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_ImageColorGrayscale(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ImageColorGrayscale: wrong arg count");

    ImageColorGrayscale(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_ImageColorContrast(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageColorContrast: wrong arg count");

    ImageColorContrast(argv[0]->v.opaque, argv[1]->v.f);
    return vnull();
}


Value* native_mila_ImageColorBrightness(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageColorBrightness: wrong arg count");

    ImageColorBrightness(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_ImageColorReplace(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ImageColorReplace: wrong arg count");

    ImageColorReplace(argv[0]->v.opaque, *((Color*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_LoadImageColors(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadImageColors: wrong arg count");

    Color * res = LoadImageColors(*((Image*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_LoadImagePalette(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("LoadImagePalette: wrong arg count");

    Color * res = LoadImagePalette(*((Image*)argv[0]->v.opaque), ((int)argv[1]->v.i), argv[2]->v.opaque);
    return vopaque(res);
}


Value* native_mila_UnloadImageColors(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadImageColors: wrong arg count");

    UnloadImageColors(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_UnloadImagePalette(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadImagePalette: wrong arg count");

    UnloadImagePalette(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_GetImageAlphaBorder(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetImageAlphaBorder: wrong arg count");

    Rectangle res = GetImageAlphaBorder(*((Image*)argv[0]->v.opaque), argv[1]->v.f);
    return vopaque(res);
}


Value* native_mila_GetImageColor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GetImageColor: wrong arg count");

    Color res = GetImageColor(*((Image*)argv[0]->v.opaque), ((int)argv[1]->v.i), ((int)argv[2]->v.i));
    return vopaque(res);
}


Value* native_mila_ImageClearBackground(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ImageClearBackground: wrong arg count");

    ImageClearBackground(argv[0]->v.opaque, *((Color*)argv[1]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawPixel(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("ImageDrawPixel: wrong arg count");

    ImageDrawPixel(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawPixelV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ImageDrawPixelV: wrong arg count");

    ImageDrawPixelV(argv[0]->v.opaque, *((Vector2*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawLine(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("ImageDrawLine: wrong arg count");

    ImageDrawLine(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawLineV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("ImageDrawLineV: wrong arg count");

    ImageDrawLineV(argv[0]->v.opaque, *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawLineEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("ImageDrawLineEx: wrong arg count");

    ImageDrawLineEx(argv[0]->v.opaque, *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawCircle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("ImageDrawCircle: wrong arg count");

    ImageDrawCircle(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawCircleV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("ImageDrawCircleV: wrong arg count");

    ImageDrawCircleV(argv[0]->v.opaque, *((Vector2*)argv[1]->v.opaque), ((int)argv[2]->v.i), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawCircleLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("ImageDrawCircleLines: wrong arg count");

    ImageDrawCircleLines(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawCircleLinesV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("ImageDrawCircleLinesV: wrong arg count");

    ImageDrawCircleLinesV(argv[0]->v.opaque, *((Vector2*)argv[1]->v.opaque), ((int)argv[2]->v.i), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawRectangle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("ImageDrawRectangle: wrong arg count");

    ImageDrawRectangle(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawRectangleV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("ImageDrawRectangleV: wrong arg count");

    ImageDrawRectangleV(argv[0]->v.opaque, *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawRectangleRec(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ImageDrawRectangleRec: wrong arg count");

    ImageDrawRectangleRec(argv[0]->v.opaque, *((Rectangle*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawRectangleLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("ImageDrawRectangleLines: wrong arg count");

    ImageDrawRectangleLines(argv[0]->v.opaque, *((Rectangle*)argv[1]->v.opaque), ((int)argv[2]->v.i), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawTriangle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("ImageDrawTriangle: wrong arg count");

    ImageDrawTriangle(argv[0]->v.opaque, *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawTriangleEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 7)
        return verror("ImageDrawTriangleEx: wrong arg count");

    ImageDrawTriangleEx(argv[0]->v.opaque, *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), *((Color*)argv[4]->v.opaque), *((Color*)argv[5]->v.opaque), *((Color*)argv[6]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawTriangleLines(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("ImageDrawTriangleLines: wrong arg count");

    ImageDrawTriangleLines(argv[0]->v.opaque, *((Vector2*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawTriangleFan(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("ImageDrawTriangleFan: wrong arg count");

    ImageDrawTriangleFan(argv[0]->v.opaque, argv[1]->v.opaque, ((int)argv[2]->v.i), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawTriangleStrip(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("ImageDrawTriangleStrip: wrong arg count");

    ImageDrawTriangleStrip(argv[0]->v.opaque, argv[1]->v.opaque, ((int)argv[2]->v.i), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDraw(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("ImageDraw: wrong arg count");

    ImageDraw(argv[0]->v.opaque, *((Image*)argv[1]->v.opaque), *((Rectangle*)argv[2]->v.opaque), *((Rectangle*)argv[3]->v.opaque), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("ImageDrawText: wrong arg count");

    ImageDrawText(argv[0]->v.opaque, argv[1]->v.s, ((int)argv[2]->v.i), ((int)argv[3]->v.i), ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_ImageDrawTextEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 7)
        return verror("ImageDrawTextEx: wrong arg count");

    ImageDrawTextEx(argv[0]->v.opaque, *((Font*)argv[1]->v.opaque), argv[2]->v.s, *((Vector2*)argv[3]->v.opaque), argv[4]->v.f, argv[5]->v.f, *((Color*)argv[6]->v.opaque));
    return vnull();
}


Value* native_mila_LoadTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadTexture: wrong arg count");

    Texture2D res = LoadTexture(argv[0]->v.s);
    return vopaque(res);
}


Value* native_mila_LoadTextureFromImage(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadTextureFromImage: wrong arg count");

    Texture2D res = LoadTextureFromImage(*((Image*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_LoadTextureCubemap(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("LoadTextureCubemap: wrong arg count");

    TextureCubemap res = LoadTextureCubemap(*((Image*)argv[0]->v.opaque), ((int)argv[1]->v.i));
    return vopaque(res);
}


Value* native_mila_LoadRenderTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("LoadRenderTexture: wrong arg count");

    RenderTexture2D res = LoadRenderTexture(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vopaque(res);
}


Value* native_mila_IsTextureValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsTextureValid: wrong arg count");

    int res = IsTextureValid(*((Texture*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_UnloadTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadTexture: wrong arg count");

    UnloadTexture(*((Texture*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_IsRenderTextureValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsRenderTextureValid: wrong arg count");

    int res = IsRenderTextureValid(*((RenderTexture*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_UnloadRenderTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadRenderTexture: wrong arg count");

    UnloadRenderTexture(*((RenderTexture*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_UpdateTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("UpdateTexture: wrong arg count");

    UpdateTexture(*((Texture*)argv[0]->v.opaque), argv[1]->v.opaque);
    return vnull();
}


Value* native_mila_UpdateTextureRec(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("UpdateTextureRec: wrong arg count");

    UpdateTextureRec(*((Texture*)argv[0]->v.opaque), *((Rectangle*)argv[1]->v.opaque), argv[2]->v.opaque);
    return vnull();
}


Value* native_mila_GenTextureMipmaps(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GenTextureMipmaps: wrong arg count");

    GenTextureMipmaps(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_SetTextureFilter(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetTextureFilter: wrong arg count");

    SetTextureFilter(*((Texture*)argv[0]->v.opaque), ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_SetTextureWrap(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetTextureWrap: wrong arg count");

    SetTextureWrap(*((Texture*)argv[0]->v.opaque), ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_DrawTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawTexture: wrong arg count");

    DrawTexture(*((Texture*)argv[0]->v.opaque), ((int)argv[1]->v.i), ((int)argv[2]->v.i), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTextureV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawTextureV: wrong arg count");

    DrawTextureV(*((Texture*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTextureEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawTextureEx: wrong arg count");

    DrawTextureEx(*((Texture*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), argv[2]->v.f, argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTextureRec(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawTextureRec: wrong arg count");

    DrawTextureRec(*((Texture*)argv[0]->v.opaque), *((Rectangle*)argv[1]->v.opaque), *((Vector2*)argv[2]->v.opaque), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTexturePro(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawTexturePro: wrong arg count");

    DrawTexturePro(*((Texture*)argv[0]->v.opaque), *((Rectangle*)argv[1]->v.opaque), *((Rectangle*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), argv[4]->v.f, *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTextureNPatch(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawTextureNPatch: wrong arg count");

    DrawTextureNPatch(*((Texture*)argv[0]->v.opaque), *((NPatchInfo*)argv[1]->v.opaque), *((Rectangle*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), argv[4]->v.f, *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_ColorIsEqual(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ColorIsEqual: wrong arg count");

    int res = ColorIsEqual(*((Color*)argv[0]->v.opaque), *((Color*)argv[1]->v.opaque));
    return vint((long)res);
}


Value* native_mila_Fade(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("Fade: wrong arg count");

    Color res = Fade(*((Color*)argv[0]->v.opaque), argv[1]->v.f);
    return vopaque(res);
}


Value* native_mila_ColorToInt(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ColorToInt: wrong arg count");

    int res = ColorToInt(*((Color*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_ColorNormalize(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ColorNormalize: wrong arg count");

    Vector4 res = ColorNormalize(*((Color*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_ColorFromNormalized(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ColorFromNormalized: wrong arg count");

    Color res = ColorFromNormalized(*((Vector4*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_ColorToHSV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ColorToHSV: wrong arg count");

    Vector3 res = ColorToHSV(*((Color*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_ColorFromHSV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ColorFromHSV: wrong arg count");

    Color res = ColorFromHSV(argv[0]->v.f, argv[1]->v.f, argv[2]->v.f);
    return vopaque(res);
}


Value* native_mila_ColorTint(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ColorTint: wrong arg count");

    Color res = ColorTint(*((Color*)argv[0]->v.opaque), *((Color*)argv[1]->v.opaque));
    return vopaque(res);
}


Value* native_mila_ColorBrightness(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ColorBrightness: wrong arg count");

    Color res = ColorBrightness(*((Color*)argv[0]->v.opaque), argv[1]->v.f);
    return vopaque(res);
}


Value* native_mila_ColorContrast(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ColorContrast: wrong arg count");

    Color res = ColorContrast(*((Color*)argv[0]->v.opaque), argv[1]->v.f);
    return vopaque(res);
}


Value* native_mila_ColorAlpha(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ColorAlpha: wrong arg count");

    Color res = ColorAlpha(*((Color*)argv[0]->v.opaque), argv[1]->v.f);
    return vopaque(res);
}


Value* native_mila_ColorAlphaBlend(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ColorAlphaBlend: wrong arg count");

    Color res = ColorAlphaBlend(*((Color*)argv[0]->v.opaque), *((Color*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque));
    return vopaque(res);
}


Value* native_mila_ColorLerp(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("ColorLerp: wrong arg count");

    Color res = ColorLerp(*((Color*)argv[0]->v.opaque), *((Color*)argv[1]->v.opaque), argv[2]->v.f);
    return vopaque(res);
}


Value* native_mila_GetColor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetColor: wrong arg count");

    Color res = GetColor(argv[0]->v.i);
    return vopaque(res);
}


Value* native_mila_GetPixelColor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetPixelColor: wrong arg count");

    Color res = GetPixelColor(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vopaque(res);
}


Value* native_mila_SetPixelColor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("SetPixelColor: wrong arg count");

    SetPixelColor(argv[0]->v.opaque, *((Color*)argv[1]->v.opaque), ((int)argv[2]->v.i));
    return vnull();
}


Value* native_mila_GetPixelDataSize(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GetPixelDataSize: wrong arg count");

    int res = GetPixelDataSize(((int)argv[0]->v.i), ((int)argv[1]->v.i), ((int)argv[2]->v.i));
    return vint((long)res);
}


Value* native_mila_GetFontDefault(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetFontDefault: wrong arg count");

    Font res = GetFontDefault();
    return vopaque(res);
}


Value* native_mila_LoadFont(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadFont: wrong arg count");

    Font res = LoadFont(argv[0]->v.s);
    return vopaque(res);
}


Value* native_mila_LoadFontEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("LoadFontEx: wrong arg count");

    Font res = LoadFontEx(argv[0]->v.s, ((int)argv[1]->v.i), argv[2]->v.opaque, ((int)argv[3]->v.i));
    return vopaque(res);
}


Value* native_mila_LoadFontFromImage(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("LoadFontFromImage: wrong arg count");

    Font res = LoadFontFromImage(*((Image*)argv[0]->v.opaque), *((Color*)argv[1]->v.opaque), ((int)argv[2]->v.i));
    return vopaque(res);
}


Value* native_mila_LoadFontFromMemory(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("LoadFontFromMemory: wrong arg count");

    Font res = LoadFontFromMemory(argv[0]->v.s, argv[1]->v.opaque, ((int)argv[2]->v.i), ((int)argv[3]->v.i), argv[4]->v.opaque, ((int)argv[5]->v.i));
    return vopaque(res);
}


Value* native_mila_IsFontValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsFontValid: wrong arg count");

    int res = IsFontValid(*((Font*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_LoadFontData(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("LoadFontData: wrong arg count");

    GlyphInfo * res = LoadFontData(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i), argv[3]->v.opaque, ((int)argv[4]->v.i), ((int)argv[5]->v.i));
    return vopaque(res);
}


Value* native_mila_GenImageFontAtlas(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("GenImageFontAtlas: wrong arg count");

    Image res = GenImageFontAtlas(argv[0]->v.opaque, argv[1]->v.opaque, ((int)argv[2]->v.i), ((int)argv[3]->v.i), ((int)argv[4]->v.i), ((int)argv[5]->v.i));
    return vopaque(res);
}


Value* native_mila_UnloadFontData(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("UnloadFontData: wrong arg count");

    UnloadFontData(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_UnloadFont(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadFont: wrong arg count");

    UnloadFont(*((Font*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_ExportFontAsCode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ExportFontAsCode: wrong arg count");

    int res = ExportFontAsCode(*((Font*)argv[0]->v.opaque), argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_DrawFPS(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("DrawFPS: wrong arg count");

    DrawFPS(((int)argv[0]->v.i), ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_DrawText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawText: wrong arg count");

    DrawText(argv[0]->v.s, ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTextEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawTextEx: wrong arg count");

    DrawTextEx(*((Font*)argv[0]->v.opaque), argv[1]->v.s, *((Vector2*)argv[2]->v.opaque), argv[3]->v.f, argv[4]->v.f, *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTextPro(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 8)
        return verror("DrawTextPro: wrong arg count");

    DrawTextPro(*((Font*)argv[0]->v.opaque), argv[1]->v.s, *((Vector2*)argv[2]->v.opaque), *((Vector2*)argv[3]->v.opaque), argv[4]->v.f, argv[5]->v.f, argv[6]->v.f, *((Color*)argv[7]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTextCodepoint(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawTextCodepoint: wrong arg count");

    DrawTextCodepoint(*((Font*)argv[0]->v.opaque), ((int)argv[1]->v.i), *((Vector2*)argv[2]->v.opaque), argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTextCodepoints(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 7)
        return verror("DrawTextCodepoints: wrong arg count");

    DrawTextCodepoints(*((Font*)argv[0]->v.opaque), argv[1]->v.opaque, ((int)argv[2]->v.i), *((Vector2*)argv[3]->v.opaque), argv[4]->v.f, argv[5]->v.f, *((Color*)argv[6]->v.opaque));
    return vnull();
}


Value* native_mila_SetTextLineSpacing(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetTextLineSpacing: wrong arg count");

    SetTextLineSpacing(((int)argv[0]->v.i));
    return vnull();
}


Value* native_mila_MeasureText(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("MeasureText: wrong arg count");

    int res = MeasureText(argv[0]->v.s, ((int)argv[1]->v.i));
    return vint((long)res);
}


Value* native_mila_MeasureTextEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("MeasureTextEx: wrong arg count");

    Vector2 res = MeasureTextEx(*((Font*)argv[0]->v.opaque), argv[1]->v.s, argv[2]->v.f, argv[3]->v.f);
    return vopaque(res);
}


Value* native_mila_GetGlyphIndex(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetGlyphIndex: wrong arg count");

    int res = GetGlyphIndex(*((Font*)argv[0]->v.opaque), ((int)argv[1]->v.i));
    return vint((long)res);
}


Value* native_mila_GetGlyphInfo(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetGlyphInfo: wrong arg count");

    GlyphInfo res = GetGlyphInfo(*((Font*)argv[0]->v.opaque), ((int)argv[1]->v.i));
    return vopaque(res);
}


Value* native_mila_GetGlyphAtlasRec(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetGlyphAtlasRec: wrong arg count");

    Rectangle res = GetGlyphAtlasRec(*((Font*)argv[0]->v.opaque), ((int)argv[1]->v.i));
    return vopaque(res);
}


Value* native_mila_LoadUTF8(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("LoadUTF8: wrong arg count");

    char * res = LoadUTF8(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vstring_dup(res);
}


Value* native_mila_UnloadUTF8(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadUTF8: wrong arg count");

    UnloadUTF8(argv[0]->v.s);
    return vnull();
}


Value* native_mila_LoadCodepoints(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("LoadCodepoints: wrong arg count");

    int * res = LoadCodepoints(argv[0]->v.s, argv[1]->v.opaque);
    return vopaque(res);
}


Value* native_mila_UnloadCodepoints(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadCodepoints: wrong arg count");

    UnloadCodepoints(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_GetCodepointCount(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetCodepointCount: wrong arg count");

    int res = GetCodepointCount(argv[0]->v.s);
    return vint((long)res);
}


Value* native_mila_GetCodepoint(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetCodepoint: wrong arg count");

    int res = GetCodepoint(argv[0]->v.s, argv[1]->v.opaque);
    return vint((long)res);
}


Value* native_mila_GetCodepointNext(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetCodepointNext: wrong arg count");

    int res = GetCodepointNext(argv[0]->v.s, argv[1]->v.opaque);
    return vint((long)res);
}


Value* native_mila_GetCodepointPrevious(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetCodepointPrevious: wrong arg count");

    int res = GetCodepointPrevious(argv[0]->v.s, argv[1]->v.opaque);
    return vint((long)res);
}


Value* native_mila_CodepointToUTF8(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("CodepointToUTF8: wrong arg count");

    const char * res = CodepointToUTF8(((int)argv[0]->v.i), argv[1]->v.opaque);
    return vstring_take(res);
}


Value* native_mila_TextCopy(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("TextCopy: wrong arg count");

    int res = TextCopy(argv[0]->v.s, argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_TextIsEqual(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("TextIsEqual: wrong arg count");

    int res = TextIsEqual(argv[0]->v.s, argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_TextLength(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("TextLength: wrong arg count");

    unsigned int res = TextLength(argv[0]->v.s);
    return vint(res);
}


#include <ffi.h>

Value* native_mila_TextFormat(Env* e, int argc, Value** argv) {
    (void)e;

    if (argc < 1)
        return verror("TextFormat: missing required argument");

    ffi_cif cif;

    int fixed = 1;
    int total = argc;

    ffi_type *types[total];
    void *values[total];

    types[0] = &ffi_type_pointer;
    values[0] = &argv[0]->v.s;

    for (int i = fixed; i < argc; i++) {
        Value *v = argv[i];

        switch(v->type) {
            case T_INT:
                types[i] = &ffi_type_slong;
                values[i] = &v->v.i;
                break;

            case T_UINT:
                types[i] = &ffi_type_ulong;
                values[i] = &v->v.ui;
                break;

            case T_FLOAT:
                types[i] = &ffi_type_double;
                values[i] = &v->v.f;
                break;

            case T_BOOL:
                types[i] = &ffi_type_sint;
                values[i] = &v->v.b;
                break;

            case T_OWNED_OPAQUE:
            case T_OPAQUE:
                types[i] = &ffi_type_pointer;
                values[i] = &v->v.opaque;
                break;

            case T_STRING:
                types[i] = &ffi_type_pointer;
                values[i] = &v->v.s;
                break;

            default:
                types[i] = &ffi_type_pointer;
                values[i] = &v->v.opaque;
                break;
        }
    }

    int res;

    ffi_prep_cif_var(
        &cif,
        FFI_DEFAULT_ABI,
        fixed,
        total,
        &ffi_type_sint,
        types
    );

    ffi_call(
        &cif,
        FFI_FN(TextFormat),
        &res,
        values
    );

    return vstring_take(res);
}


Value* native_mila_TextSubtext(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("TextSubtext: wrong arg count");

    const char * res = TextSubtext(argv[0]->v.s, ((int)argv[1]->v.i), ((int)argv[2]->v.i));
    return vstring_take(res);
}


Value* native_mila_TextReplace(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("TextReplace: wrong arg count");

    char * res = TextReplace(argv[0]->v.s, argv[1]->v.s, argv[2]->v.s);
    return vstring_dup(res);
}


Value* native_mila_TextInsert(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("TextInsert: wrong arg count");

    char * res = TextInsert(argv[0]->v.s, argv[1]->v.s, ((int)argv[2]->v.i));
    return vstring_dup(res);
}


Value* native_mila_TextJoin(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("TextJoin: wrong arg count");

    const char * res = TextJoin(argv[0]->v.opaque, ((int)argv[1]->v.i), argv[2]->v.s);
    return vstring_take(res);
}


Value* native_mila_TextSplit(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("TextSplit: wrong arg count");

    const char ** res = TextSplit(argv[0]->v.s, argv[1]->v.opaque, argv[2]->v.opaque);
    return vopaque(res);
}


Value* native_mila_TextAppend(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("TextAppend: wrong arg count");

    TextAppend(argv[0]->v.s, argv[1]->v.s, argv[2]->v.opaque);
    return vnull();
}


Value* native_mila_TextFindIndex(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("TextFindIndex: wrong arg count");

    int res = TextFindIndex(argv[0]->v.s, argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_TextToUpper(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("TextToUpper: wrong arg count");

    const char * res = TextToUpper(argv[0]->v.s);
    return vstring_take(res);
}


Value* native_mila_TextToLower(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("TextToLower: wrong arg count");

    const char * res = TextToLower(argv[0]->v.s);
    return vstring_take(res);
}


Value* native_mila_TextToPascal(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("TextToPascal: wrong arg count");

    const char * res = TextToPascal(argv[0]->v.s);
    return vstring_take(res);
}


Value* native_mila_TextToSnake(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("TextToSnake: wrong arg count");

    const char * res = TextToSnake(argv[0]->v.s);
    return vstring_take(res);
}


Value* native_mila_TextToCamel(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("TextToCamel: wrong arg count");

    const char * res = TextToCamel(argv[0]->v.s);
    return vstring_take(res);
}


Value* native_mila_TextToInteger(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("TextToInteger: wrong arg count");

    int res = TextToInteger(argv[0]->v.s);
    return vint((long)res);
}


Value* native_mila_TextToFloat(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("TextToFloat: wrong arg count");

    float res = TextToFloat(argv[0]->v.s);
    return vfloat((double)res);
}


Value* native_mila_DrawLine3D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawLine3D: wrong arg count");

    DrawLine3D(*((Vector3*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawPoint3D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("DrawPoint3D: wrong arg count");

    DrawPoint3D(*((Vector3*)argv[0]->v.opaque), *((Color*)argv[1]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCircle3D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawCircle3D: wrong arg count");

    DrawCircle3D(*((Vector3*)argv[0]->v.opaque), argv[1]->v.f, *((Vector3*)argv[2]->v.opaque), argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTriangle3D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawTriangle3D: wrong arg count");

    DrawTriangle3D(*((Vector3*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), *((Vector3*)argv[2]->v.opaque), *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawTriangleStrip3D(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawTriangleStrip3D: wrong arg count");

    DrawTriangleStrip3D(argv[0]->v.opaque, ((int)argv[1]->v.i), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCube(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawCube: wrong arg count");

    DrawCube(*((Vector3*)argv[0]->v.opaque), argv[1]->v.f, argv[2]->v.f, argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCubeV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawCubeV: wrong arg count");

    DrawCubeV(*((Vector3*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCubeWires(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawCubeWires: wrong arg count");

    DrawCubeWires(*((Vector3*)argv[0]->v.opaque), argv[1]->v.f, argv[2]->v.f, argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCubeWiresV(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawCubeWiresV: wrong arg count");

    DrawCubeWiresV(*((Vector3*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSphere(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawSphere: wrong arg count");

    DrawSphere(*((Vector3*)argv[0]->v.opaque), argv[1]->v.f, *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSphereEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawSphereEx: wrong arg count");

    DrawSphereEx(*((Vector3*)argv[0]->v.opaque), argv[1]->v.f, ((int)argv[2]->v.i), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawSphereWires(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawSphereWires: wrong arg count");

    DrawSphereWires(*((Vector3*)argv[0]->v.opaque), argv[1]->v.f, ((int)argv[2]->v.i), ((int)argv[3]->v.i), *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCylinder(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawCylinder: wrong arg count");

    DrawCylinder(*((Vector3*)argv[0]->v.opaque), argv[1]->v.f, argv[2]->v.f, argv[3]->v.f, ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCylinderEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawCylinderEx: wrong arg count");

    DrawCylinderEx(*((Vector3*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), argv[2]->v.f, argv[3]->v.f, ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCylinderWires(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawCylinderWires: wrong arg count");

    DrawCylinderWires(*((Vector3*)argv[0]->v.opaque), argv[1]->v.f, argv[2]->v.f, argv[3]->v.f, ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCylinderWiresEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawCylinderWiresEx: wrong arg count");

    DrawCylinderWiresEx(*((Vector3*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), argv[2]->v.f, argv[3]->v.f, ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCapsule(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawCapsule: wrong arg count");

    DrawCapsule(*((Vector3*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), argv[2]->v.f, ((int)argv[3]->v.i), ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawCapsuleWires(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawCapsuleWires: wrong arg count");

    DrawCapsuleWires(*((Vector3*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), argv[2]->v.f, ((int)argv[3]->v.i), ((int)argv[4]->v.i), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawPlane(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawPlane: wrong arg count");

    DrawPlane(*((Vector3*)argv[0]->v.opaque), *((Vector2*)argv[1]->v.opaque), *((Color*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawRay(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("DrawRay: wrong arg count");

    DrawRay(*((Ray*)argv[0]->v.opaque), *((Color*)argv[1]->v.opaque));
    return vnull();
}


Value* native_mila_DrawGrid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("DrawGrid: wrong arg count");

    DrawGrid(((int)argv[0]->v.i), argv[1]->v.f);
    return vnull();
}


Value* native_mila_LoadModel(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadModel: wrong arg count");

    Model res = LoadModel(argv[0]->v.s);
    return vopaque(res);
}


Value* native_mila_LoadModelFromMesh(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadModelFromMesh: wrong arg count");

    Model res = LoadModelFromMesh(*((Mesh*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_IsModelValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsModelValid: wrong arg count");

    int res = IsModelValid(*((Model*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_UnloadModel(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadModel: wrong arg count");

    UnloadModel(*((Model*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_GetModelBoundingBox(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetModelBoundingBox: wrong arg count");

    BoundingBox res = GetModelBoundingBox(*((Model*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_DrawModel(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawModel: wrong arg count");

    DrawModel(*((Model*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawModelEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawModelEx: wrong arg count");

    DrawModelEx(*((Model*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), *((Vector3*)argv[2]->v.opaque), argv[3]->v.f, *((Vector3*)argv[4]->v.opaque), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawModelWires(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawModelWires: wrong arg count");

    DrawModelWires(*((Model*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawModelWiresEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawModelWiresEx: wrong arg count");

    DrawModelWiresEx(*((Model*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), *((Vector3*)argv[2]->v.opaque), argv[3]->v.f, *((Vector3*)argv[4]->v.opaque), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawModelPoints(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawModelPoints: wrong arg count");

    DrawModelPoints(*((Model*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), argv[2]->v.f, *((Color*)argv[3]->v.opaque));
    return vnull();
}


Value* native_mila_DrawModelPointsEx(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawModelPointsEx: wrong arg count");

    DrawModelPointsEx(*((Model*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), *((Vector3*)argv[2]->v.opaque), argv[3]->v.f, *((Vector3*)argv[4]->v.opaque), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawBoundingBox(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("DrawBoundingBox: wrong arg count");

    DrawBoundingBox(*((BoundingBox*)argv[0]->v.opaque), *((Color*)argv[1]->v.opaque));
    return vnull();
}


Value* native_mila_DrawBillboard(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("DrawBillboard: wrong arg count");

    DrawBillboard(*((Camera3D*)argv[0]->v.opaque), *((Texture*)argv[1]->v.opaque), *((Vector3*)argv[2]->v.opaque), argv[3]->v.f, *((Color*)argv[4]->v.opaque));
    return vnull();
}


Value* native_mila_DrawBillboardRec(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6)
        return verror("DrawBillboardRec: wrong arg count");

    DrawBillboardRec(*((Camera3D*)argv[0]->v.opaque), *((Texture*)argv[1]->v.opaque), *((Rectangle*)argv[2]->v.opaque), *((Vector3*)argv[3]->v.opaque), *((Vector2*)argv[4]->v.opaque), *((Color*)argv[5]->v.opaque));
    return vnull();
}


Value* native_mila_DrawBillboardPro(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 9)
        return verror("DrawBillboardPro: wrong arg count");

    DrawBillboardPro(*((Camera3D*)argv[0]->v.opaque), *((Texture*)argv[1]->v.opaque), *((Rectangle*)argv[2]->v.opaque), *((Vector3*)argv[3]->v.opaque), *((Vector3*)argv[4]->v.opaque), *((Vector2*)argv[5]->v.opaque), *((Vector2*)argv[6]->v.opaque), argv[7]->v.f, *((Color*)argv[8]->v.opaque));
    return vnull();
}


Value* native_mila_UploadMesh(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("UploadMesh: wrong arg count");

    UploadMesh(argv[0]->v.opaque, argv[1]->v.opaque);
    return vnull();
}


Value* native_mila_UpdateMeshBuffer(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("UpdateMeshBuffer: wrong arg count");

    UpdateMeshBuffer(*((Mesh*)argv[0]->v.opaque), ((int)argv[1]->v.i), argv[2]->v.opaque, ((int)argv[3]->v.i), ((int)argv[4]->v.i));
    return vnull();
}


Value* native_mila_UnloadMesh(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadMesh: wrong arg count");

    UnloadMesh(*((Mesh*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_DrawMesh(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("DrawMesh: wrong arg count");

    DrawMesh(*((Mesh*)argv[0]->v.opaque), *((Material*)argv[1]->v.opaque), *((Matrix*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_DrawMeshInstanced(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("DrawMeshInstanced: wrong arg count");

    DrawMeshInstanced(*((Mesh*)argv[0]->v.opaque), *((Material*)argv[1]->v.opaque), argv[2]->v.opaque, ((int)argv[3]->v.i));
    return vnull();
}


Value* native_mila_GetMeshBoundingBox(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetMeshBoundingBox: wrong arg count");

    BoundingBox res = GetMeshBoundingBox(*((Mesh*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GenMeshTangents(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GenMeshTangents: wrong arg count");

    GenMeshTangents(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_ExportMesh(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ExportMesh: wrong arg count");

    int res = ExportMesh(*((Mesh*)argv[0]->v.opaque), argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_ExportMeshAsCode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ExportMeshAsCode: wrong arg count");

    int res = ExportMeshAsCode(*((Mesh*)argv[0]->v.opaque), argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_GenMeshPoly(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GenMeshPoly: wrong arg count");

    Mesh res = GenMeshPoly(((int)argv[0]->v.i), argv[1]->v.f);
    return vopaque(res);
}


Value* native_mila_GenMeshPlane(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("GenMeshPlane: wrong arg count");

    Mesh res = GenMeshPlane(argv[0]->v.f, argv[1]->v.f, ((int)argv[2]->v.i), ((int)argv[3]->v.i));
    return vopaque(res);
}


Value* native_mila_GenMeshCube(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GenMeshCube: wrong arg count");

    Mesh res = GenMeshCube(argv[0]->v.f, argv[1]->v.f, argv[2]->v.f);
    return vopaque(res);
}


Value* native_mila_GenMeshSphere(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GenMeshSphere: wrong arg count");

    Mesh res = GenMeshSphere(argv[0]->v.f, ((int)argv[1]->v.i), ((int)argv[2]->v.i));
    return vopaque(res);
}


Value* native_mila_GenMeshHemiSphere(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GenMeshHemiSphere: wrong arg count");

    Mesh res = GenMeshHemiSphere(argv[0]->v.f, ((int)argv[1]->v.i), ((int)argv[2]->v.i));
    return vopaque(res);
}


Value* native_mila_GenMeshCylinder(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GenMeshCylinder: wrong arg count");

    Mesh res = GenMeshCylinder(argv[0]->v.f, argv[1]->v.f, ((int)argv[2]->v.i));
    return vopaque(res);
}


Value* native_mila_GenMeshCone(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GenMeshCone: wrong arg count");

    Mesh res = GenMeshCone(argv[0]->v.f, argv[1]->v.f, ((int)argv[2]->v.i));
    return vopaque(res);
}


Value* native_mila_GenMeshTorus(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("GenMeshTorus: wrong arg count");

    Mesh res = GenMeshTorus(argv[0]->v.f, argv[1]->v.f, ((int)argv[2]->v.i), ((int)argv[3]->v.i));
    return vopaque(res);
}


Value* native_mila_GenMeshKnot(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("GenMeshKnot: wrong arg count");

    Mesh res = GenMeshKnot(argv[0]->v.f, argv[1]->v.f, ((int)argv[2]->v.i), ((int)argv[3]->v.i));
    return vopaque(res);
}


Value* native_mila_GenMeshHeightmap(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GenMeshHeightmap: wrong arg count");

    Mesh res = GenMeshHeightmap(*((Image*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GenMeshCubicmap(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GenMeshCubicmap: wrong arg count");

    Mesh res = GenMeshCubicmap(*((Image*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque));
    return vopaque(res);
}


Value* native_mila_LoadMaterials(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("LoadMaterials: wrong arg count");

    Material * res = LoadMaterials(argv[0]->v.s, argv[1]->v.opaque);
    return vopaque(res);
}


Value* native_mila_LoadMaterialDefault(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("LoadMaterialDefault: wrong arg count");

    Material res = LoadMaterialDefault();
    return vopaque(res);
}


Value* native_mila_IsMaterialValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsMaterialValid: wrong arg count");

    int res = IsMaterialValid(*((Material*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_UnloadMaterial(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadMaterial: wrong arg count");

    UnloadMaterial(*((Material*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_SetMaterialTexture(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("SetMaterialTexture: wrong arg count");

    SetMaterialTexture(argv[0]->v.opaque, ((int)argv[1]->v.i), *((Texture*)argv[2]->v.opaque));
    return vnull();
}


Value* native_mila_SetModelMeshMaterial(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("SetModelMeshMaterial: wrong arg count");

    SetModelMeshMaterial(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i));
    return vnull();
}


Value* native_mila_LoadModelAnimations(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("LoadModelAnimations: wrong arg count");

    ModelAnimation * res = LoadModelAnimations(argv[0]->v.s, argv[1]->v.opaque);
    return vopaque(res);
}


Value* native_mila_UpdateModelAnimation(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("UpdateModelAnimation: wrong arg count");

    UpdateModelAnimation(*((Model*)argv[0]->v.opaque), *((ModelAnimation*)argv[1]->v.opaque), ((int)argv[2]->v.i));
    return vnull();
}


Value* native_mila_UpdateModelAnimationBones(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("UpdateModelAnimationBones: wrong arg count");

    UpdateModelAnimationBones(*((Model*)argv[0]->v.opaque), *((ModelAnimation*)argv[1]->v.opaque), ((int)argv[2]->v.i));
    return vnull();
}


Value* native_mila_UnloadModelAnimation(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadModelAnimation: wrong arg count");

    UnloadModelAnimation(*((ModelAnimation*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_UnloadModelAnimations(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("UnloadModelAnimations: wrong arg count");

    UnloadModelAnimations(argv[0]->v.opaque, ((int)argv[1]->v.i));
    return vnull();
}


Value* native_mila_IsModelAnimationValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("IsModelAnimationValid: wrong arg count");

    int res = IsModelAnimationValid(*((Model*)argv[0]->v.opaque), *((ModelAnimation*)argv[1]->v.opaque));
    return vint((long)res);
}


Value* native_mila_CheckCollisionSpheres(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("CheckCollisionSpheres: wrong arg count");

    int res = CheckCollisionSpheres(*((Vector3*)argv[0]->v.opaque), argv[1]->v.f, *((Vector3*)argv[2]->v.opaque), argv[3]->v.f);
    return vint((long)res);
}


Value* native_mila_CheckCollisionBoxes(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("CheckCollisionBoxes: wrong arg count");

    int res = CheckCollisionBoxes(*((BoundingBox*)argv[0]->v.opaque), *((BoundingBox*)argv[1]->v.opaque));
    return vint((long)res);
}


Value* native_mila_CheckCollisionBoxSphere(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("CheckCollisionBoxSphere: wrong arg count");

    int res = CheckCollisionBoxSphere(*((BoundingBox*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), argv[2]->v.f);
    return vint((long)res);
}


Value* native_mila_GetRayCollisionSphere(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GetRayCollisionSphere: wrong arg count");

    RayCollision res = GetRayCollisionSphere(*((Ray*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), argv[2]->v.f);
    return vopaque(res);
}


Value* native_mila_GetRayCollisionBox(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("GetRayCollisionBox: wrong arg count");

    RayCollision res = GetRayCollisionBox(*((Ray*)argv[0]->v.opaque), *((BoundingBox*)argv[1]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GetRayCollisionMesh(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("GetRayCollisionMesh: wrong arg count");

    RayCollision res = GetRayCollisionMesh(*((Ray*)argv[0]->v.opaque), *((Mesh*)argv[1]->v.opaque), *((Matrix*)argv[2]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GetRayCollisionTriangle(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("GetRayCollisionTriangle: wrong arg count");

    RayCollision res = GetRayCollisionTriangle(*((Ray*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), *((Vector3*)argv[2]->v.opaque), *((Vector3*)argv[3]->v.opaque));
    return vopaque(res);
}


Value* native_mila_GetRayCollisionQuad(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5)
        return verror("GetRayCollisionQuad: wrong arg count");

    RayCollision res = GetRayCollisionQuad(*((Ray*)argv[0]->v.opaque), *((Vector3*)argv[1]->v.opaque), *((Vector3*)argv[2]->v.opaque), *((Vector3*)argv[3]->v.opaque), *((Vector3*)argv[4]->v.opaque));
    return vopaque(res);
}


Value* native_mila_InitAudioDevice(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("InitAudioDevice: wrong arg count");

    InitAudioDevice();
    return vnull();
}


Value* native_mila_CloseAudioDevice(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("CloseAudioDevice: wrong arg count");

    CloseAudioDevice();
    return vnull();
}


Value* native_mila_IsAudioDeviceReady(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("IsAudioDeviceReady: wrong arg count");

    int res = IsAudioDeviceReady();
    return vint((long)res);
}


Value* native_mila_SetMasterVolume(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetMasterVolume: wrong arg count");

    SetMasterVolume(argv[0]->v.f);
    return vnull();
}


Value* native_mila_GetMasterVolume(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0)
        return verror("GetMasterVolume: wrong arg count");

    float res = GetMasterVolume();
    return vfloat((double)res);
}


Value* native_mila_LoadWave(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadWave: wrong arg count");

    Wave res = LoadWave(argv[0]->v.s);
    return vopaque(res);
}


Value* native_mila_LoadWaveFromMemory(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("LoadWaveFromMemory: wrong arg count");

    Wave res = LoadWaveFromMemory(argv[0]->v.s, argv[1]->v.opaque, ((int)argv[2]->v.i));
    return vopaque(res);
}


Value* native_mila_IsWaveValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsWaveValid: wrong arg count");

    int res = IsWaveValid(*((Wave*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_LoadSound(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadSound: wrong arg count");

    Sound res = LoadSound(argv[0]->v.s);
    return vopaque(res);
}


Value* native_mila_LoadSoundFromWave(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadSoundFromWave: wrong arg count");

    Sound res = LoadSoundFromWave(*((Wave*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_LoadSoundAlias(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadSoundAlias: wrong arg count");

    Sound res = LoadSoundAlias(*((Sound*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_IsSoundValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsSoundValid: wrong arg count");

    int res = IsSoundValid(*((Sound*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_UpdateSound(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("UpdateSound: wrong arg count");

    UpdateSound(*((Sound*)argv[0]->v.opaque), argv[1]->v.opaque, ((int)argv[2]->v.i));
    return vnull();
}


Value* native_mila_UnloadWave(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadWave: wrong arg count");

    UnloadWave(*((Wave*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_UnloadSound(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadSound: wrong arg count");

    UnloadSound(*((Sound*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_UnloadSoundAlias(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadSoundAlias: wrong arg count");

    UnloadSoundAlias(*((Sound*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_ExportWave(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ExportWave: wrong arg count");

    int res = ExportWave(*((Wave*)argv[0]->v.opaque), argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_ExportWaveAsCode(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("ExportWaveAsCode: wrong arg count");

    int res = ExportWaveAsCode(*((Wave*)argv[0]->v.opaque), argv[1]->v.s);
    return vint((long)res);
}


Value* native_mila_PlaySound(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("PlaySound: wrong arg count");

    PlaySound(*((Sound*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_StopSound(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("StopSound: wrong arg count");

    StopSound(*((Sound*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_PauseSound(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("PauseSound: wrong arg count");

    PauseSound(*((Sound*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_ResumeSound(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ResumeSound: wrong arg count");

    ResumeSound(*((Sound*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_IsSoundPlaying(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsSoundPlaying: wrong arg count");

    int res = IsSoundPlaying(*((Sound*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_SetSoundVolume(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetSoundVolume: wrong arg count");

    SetSoundVolume(*((Sound*)argv[0]->v.opaque), argv[1]->v.f);
    return vnull();
}


Value* native_mila_SetSoundPitch(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetSoundPitch: wrong arg count");

    SetSoundPitch(*((Sound*)argv[0]->v.opaque), argv[1]->v.f);
    return vnull();
}


Value* native_mila_SetSoundPan(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetSoundPan: wrong arg count");

    SetSoundPan(*((Sound*)argv[0]->v.opaque), argv[1]->v.f);
    return vnull();
}


Value* native_mila_WaveCopy(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("WaveCopy: wrong arg count");

    Wave res = WaveCopy(*((Wave*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_WaveCrop(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("WaveCrop: wrong arg count");

    WaveCrop(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i));
    return vnull();
}


Value* native_mila_WaveFormat(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4)
        return verror("WaveFormat: wrong arg count");

    WaveFormat(argv[0]->v.opaque, ((int)argv[1]->v.i), ((int)argv[2]->v.i), ((int)argv[3]->v.i));
    return vnull();
}


Value* native_mila_LoadWaveSamples(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadWaveSamples: wrong arg count");

    float * res = LoadWaveSamples(*((Wave*)argv[0]->v.opaque));
    return vopaque(res);
}


Value* native_mila_UnloadWaveSamples(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadWaveSamples: wrong arg count");

    UnloadWaveSamples(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_LoadMusicStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("LoadMusicStream: wrong arg count");

    Music res = LoadMusicStream(argv[0]->v.s);
    return vopaque(res);
}


Value* native_mila_LoadMusicStreamFromMemory(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("LoadMusicStreamFromMemory: wrong arg count");

    Music res = LoadMusicStreamFromMemory(argv[0]->v.s, argv[1]->v.opaque, ((int)argv[2]->v.i));
    return vopaque(res);
}


Value* native_mila_IsMusicValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsMusicValid: wrong arg count");

    int res = IsMusicValid(*((Music*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_UnloadMusicStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadMusicStream: wrong arg count");

    UnloadMusicStream(*((Music*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_PlayMusicStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("PlayMusicStream: wrong arg count");

    PlayMusicStream(*((Music*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_IsMusicStreamPlaying(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsMusicStreamPlaying: wrong arg count");

    int res = IsMusicStreamPlaying(*((Music*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_UpdateMusicStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UpdateMusicStream: wrong arg count");

    UpdateMusicStream(*((Music*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_StopMusicStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("StopMusicStream: wrong arg count");

    StopMusicStream(*((Music*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_PauseMusicStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("PauseMusicStream: wrong arg count");

    PauseMusicStream(*((Music*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_ResumeMusicStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ResumeMusicStream: wrong arg count");

    ResumeMusicStream(*((Music*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_SeekMusicStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SeekMusicStream: wrong arg count");

    SeekMusicStream(*((Music*)argv[0]->v.opaque), argv[1]->v.f);
    return vnull();
}


Value* native_mila_SetMusicVolume(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetMusicVolume: wrong arg count");

    SetMusicVolume(*((Music*)argv[0]->v.opaque), argv[1]->v.f);
    return vnull();
}


Value* native_mila_SetMusicPitch(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetMusicPitch: wrong arg count");

    SetMusicPitch(*((Music*)argv[0]->v.opaque), argv[1]->v.f);
    return vnull();
}


Value* native_mila_SetMusicPan(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetMusicPan: wrong arg count");

    SetMusicPan(*((Music*)argv[0]->v.opaque), argv[1]->v.f);
    return vnull();
}


Value* native_mila_GetMusicTimeLength(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetMusicTimeLength: wrong arg count");

    float res = GetMusicTimeLength(*((Music*)argv[0]->v.opaque));
    return vfloat((double)res);
}


Value* native_mila_GetMusicTimePlayed(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("GetMusicTimePlayed: wrong arg count");

    float res = GetMusicTimePlayed(*((Music*)argv[0]->v.opaque));
    return vfloat((double)res);
}


Value* native_mila_LoadAudioStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("LoadAudioStream: wrong arg count");

    AudioStream res = LoadAudioStream(argv[0]->v.i, argv[1]->v.i, argv[2]->v.i);
    return vopaque(res);
}


Value* native_mila_IsAudioStreamValid(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsAudioStreamValid: wrong arg count");

    int res = IsAudioStreamValid(*((AudioStream*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_UnloadAudioStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("UnloadAudioStream: wrong arg count");

    UnloadAudioStream(*((AudioStream*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_UpdateAudioStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3)
        return verror("UpdateAudioStream: wrong arg count");

    UpdateAudioStream(*((AudioStream*)argv[0]->v.opaque), argv[1]->v.opaque, ((int)argv[2]->v.i));
    return vnull();
}


Value* native_mila_IsAudioStreamProcessed(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsAudioStreamProcessed: wrong arg count");

    int res = IsAudioStreamProcessed(*((AudioStream*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_PlayAudioStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("PlayAudioStream: wrong arg count");

    PlayAudioStream(*((AudioStream*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_PauseAudioStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("PauseAudioStream: wrong arg count");

    PauseAudioStream(*((AudioStream*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_ResumeAudioStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("ResumeAudioStream: wrong arg count");

    ResumeAudioStream(*((AudioStream*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_IsAudioStreamPlaying(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("IsAudioStreamPlaying: wrong arg count");

    int res = IsAudioStreamPlaying(*((AudioStream*)argv[0]->v.opaque));
    return vint((long)res);
}


Value* native_mila_StopAudioStream(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("StopAudioStream: wrong arg count");

    StopAudioStream(*((AudioStream*)argv[0]->v.opaque));
    return vnull();
}


Value* native_mila_SetAudioStreamVolume(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetAudioStreamVolume: wrong arg count");

    SetAudioStreamVolume(*((AudioStream*)argv[0]->v.opaque), argv[1]->v.f);
    return vnull();
}


Value* native_mila_SetAudioStreamPitch(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetAudioStreamPitch: wrong arg count");

    SetAudioStreamPitch(*((AudioStream*)argv[0]->v.opaque), argv[1]->v.f);
    return vnull();
}


Value* native_mila_SetAudioStreamPan(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetAudioStreamPan: wrong arg count");

    SetAudioStreamPan(*((AudioStream*)argv[0]->v.opaque), argv[1]->v.f);
    return vnull();
}


Value* native_mila_SetAudioStreamBufferSizeDefault(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("SetAudioStreamBufferSizeDefault: wrong arg count");

    SetAudioStreamBufferSizeDefault(((int)argv[0]->v.i));
    return vnull();
}


Value* native_mila_SetAudioStreamCallback(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("SetAudioStreamCallback: wrong arg count");

    SetAudioStreamCallback(*((AudioStream*)argv[0]->v.opaque), argv[1]->v.opaque);
    return vnull();
}


Value* native_mila_AttachAudioStreamProcessor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("AttachAudioStreamProcessor: wrong arg count");

    AttachAudioStreamProcessor(*((AudioStream*)argv[0]->v.opaque), argv[1]->v.opaque);
    return vnull();
}


Value* native_mila_DetachAudioStreamProcessor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2)
        return verror("DetachAudioStreamProcessor: wrong arg count");

    DetachAudioStreamProcessor(*((AudioStream*)argv[0]->v.opaque), argv[1]->v.opaque);
    return vnull();
}


Value* native_mila_AttachAudioMixedProcessor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("AttachAudioMixedProcessor: wrong arg count");

    AttachAudioMixedProcessor(argv[0]->v.opaque);
    return vnull();
}


Value* native_mila_DetachAudioMixedProcessor(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 1)
        return verror("DetachAudioMixedProcessor: wrong arg count");

    DetachAudioMixedProcessor(argv[0]->v.opaque);
    return vnull();
}

Value* _type_mila_Vector2_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2) return verror("Vector2: wrong arg count");
    Vector2* tmp = malloc(sizeof(Vector2));
    tmp->x = argv[0]->v.f;
    tmp->y = argv[1]->v.f;
    return vowned_opaque_extra(tmp, NULL, "struct Vector2");
}
Value* _type_mila_Vector3_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3) return verror("Vector3: wrong arg count");
    Vector3* tmp = malloc(sizeof(Vector3));
    tmp->x = argv[0]->v.f;
    tmp->y = argv[1]->v.f;
    tmp->z = argv[2]->v.f;
    return vowned_opaque_extra(tmp, NULL, "struct Vector3");
}
Value* _type_mila_Vector4_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4) return verror("Vector4: wrong arg count");
    Vector4* tmp = malloc(sizeof(Vector4));
    tmp->x = argv[0]->v.f;
    tmp->y = argv[1]->v.f;
    tmp->z = argv[2]->v.f;
    tmp->w = argv[3]->v.f;
    return vowned_opaque_extra(tmp, NULL, "struct Vector4");
}
Value* _type_mila_Matrix_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 16) return verror("Matrix: wrong arg count");
    Matrix* tmp = malloc(sizeof(Matrix));
    tmp->m0 = argv[0]->v.f;
    tmp->m4 = argv[1]->v.f;
    tmp->m8 = argv[2]->v.f;
    tmp->m12 = argv[3]->v.f;
    tmp->m1 = argv[4]->v.f;
    tmp->m5 = argv[5]->v.f;
    tmp->m9 = argv[6]->v.f;
    tmp->m13 = argv[7]->v.f;
    tmp->m2 = argv[8]->v.f;
    tmp->m6 = argv[9]->v.f;
    tmp->m10 = argv[10]->v.f;
    tmp->m14 = argv[11]->v.f;
    tmp->m3 = argv[12]->v.f;
    tmp->m7 = argv[13]->v.f;
    tmp->m11 = argv[14]->v.f;
    tmp->m15 = argv[15]->v.f;
    return vowned_opaque_extra(tmp, NULL, "struct Matrix");
}
Value* _type_mila_Color_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4) return verror("Color: wrong arg count");
    Color* tmp = malloc(sizeof(Color));
    tmp->r = (unsigned char)(argv[0]->v.i);
    tmp->g = (unsigned char)(argv[1]->v.i);
    tmp->b = (unsigned char)(argv[2]->v.i);
    tmp->a = (unsigned char)(argv[3]->v.i);
    return vowned_opaque_extra(tmp, NULL, "struct Color");
}
Value* _type_mila_Rectangle_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4) return verror("Rectangle: wrong arg count");
    Rectangle* tmp = malloc(sizeof(Rectangle));
    tmp->x = argv[0]->v.f;
    tmp->y = argv[1]->v.f;
    tmp->width = argv[2]->v.f;
    tmp->height = argv[3]->v.f;
    return vowned_opaque_extra(tmp, NULL, "struct Rectangle");
}
Value* _type_mila_Image_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5) return verror("Image: wrong arg count");
    Image* tmp = malloc(sizeof(Image));
    tmp->data = argv[0]->v.opaque;
    tmp->width = ((int)argv[1]->v.i);
    tmp->height = ((int)argv[2]->v.i);
    tmp->mipmaps = ((int)argv[3]->v.i);
    tmp->format = ((int)argv[4]->v.i);
    return vowned_opaque_extra(tmp, NULL, "struct Image");
}
Value* _type_mila_Texture_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5) return verror("Texture: wrong arg count");
    Texture* tmp = malloc(sizeof(Texture));
    tmp->id = argv[0]->v.i;
    tmp->width = ((int)argv[1]->v.i);
    tmp->height = ((int)argv[2]->v.i);
    tmp->mipmaps = ((int)argv[3]->v.i);
    tmp->format = ((int)argv[4]->v.i);
    return vowned_opaque_extra(tmp, NULL, "struct Texture");
}
Value* _type_mila_RenderTexture_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3) return verror("RenderTexture: wrong arg count");
    RenderTexture* tmp = malloc(sizeof(RenderTexture));
    tmp->id = argv[0]->v.i;
    tmp->texture = argv[1]->v.opaque;
    tmp->depth = argv[2]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct RenderTexture");
}
Value* _type_mila_NPatchInfo_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6) return verror("NPatchInfo: wrong arg count");
    NPatchInfo* tmp = malloc(sizeof(NPatchInfo));
    tmp->source = argv[0]->v.opaque;
    tmp->left = ((int)argv[1]->v.i);
    tmp->top = ((int)argv[2]->v.i);
    tmp->right = ((int)argv[3]->v.i);
    tmp->bottom = ((int)argv[4]->v.i);
    tmp->layout = ((int)argv[5]->v.i);
    return vowned_opaque_extra(tmp, NULL, "struct NPatchInfo");
}
Value* _type_mila_GlyphInfo_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5) return verror("GlyphInfo: wrong arg count");
    GlyphInfo* tmp = malloc(sizeof(GlyphInfo));
    tmp->value = ((int)argv[0]->v.i);
    tmp->offsetX = ((int)argv[1]->v.i);
    tmp->offsetY = ((int)argv[2]->v.i);
    tmp->advanceX = ((int)argv[3]->v.i);
    tmp->image = argv[4]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct GlyphInfo");
}
Value* _type_mila_Font_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 6) return verror("Font: wrong arg count");
    Font* tmp = malloc(sizeof(Font));
    tmp->baseSize = ((int)argv[0]->v.i);
    tmp->glyphCount = ((int)argv[1]->v.i);
    tmp->glyphPadding = ((int)argv[2]->v.i);
    tmp->texture = argv[3]->v.opaque;
    tmp->recs = argv[4]->v.opaque;
    tmp->glyphs = argv[5]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct Font");
}
Value* _type_mila_Camera3D_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5) return verror("Camera3D: wrong arg count");
    Camera3D* tmp = malloc(sizeof(Camera3D));
    tmp->position = argv[0]->v.opaque;
    tmp->target = argv[1]->v.opaque;
    tmp->up = argv[2]->v.opaque;
    tmp->fovy = argv[3]->v.f;
    tmp->projection = ((int)argv[4]->v.i);
    return vowned_opaque_extra(tmp, NULL, "struct Camera3D");
}
Value* _type_mila_Camera2D_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4) return verror("Camera2D: wrong arg count");
    Camera2D* tmp = malloc(sizeof(Camera2D));
    tmp->offset = argv[0]->v.opaque;
    tmp->target = argv[1]->v.opaque;
    tmp->rotation = argv[2]->v.f;
    tmp->zoom = argv[3]->v.f;
    return vowned_opaque_extra(tmp, NULL, "struct Camera2D");
}
Value* _type_mila_Mesh_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 17) return verror("Mesh: wrong arg count");
    Mesh* tmp = malloc(sizeof(Mesh));
    tmp->vertexCount = ((int)argv[0]->v.i);
    tmp->triangleCount = ((int)argv[1]->v.i);
    tmp->vertices = argv[2]->v.opaque;
    tmp->texcoords = argv[3]->v.opaque;
    tmp->texcoords2 = argv[4]->v.opaque;
    tmp->normals = argv[5]->v.opaque;
    tmp->tangents = argv[6]->v.opaque;
    tmp->colors = argv[7]->v.opaque;
    tmp->indices = argv[8]->v.opaque;
    tmp->animVertices = argv[9]->v.opaque;
    tmp->animNormals = argv[10]->v.opaque;
    tmp->boneIds = argv[11]->v.opaque;
    tmp->boneWeights = argv[12]->v.opaque;
    tmp->boneMatrices = argv[13]->v.opaque;
    tmp->boneCount = ((int)argv[14]->v.i);
    tmp->vaoId = argv[15]->v.i;
    tmp->vboId = argv[16]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct Mesh");
}
Value* _type_mila_Shader_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2) return verror("Shader: wrong arg count");
    Shader* tmp = malloc(sizeof(Shader));
    tmp->id = argv[0]->v.i;
    tmp->locs = argv[1]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct Shader");
}
Value* _type_mila_MaterialMap_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3) return verror("MaterialMap: wrong arg count");
    MaterialMap* tmp = malloc(sizeof(MaterialMap));
    tmp->texture = argv[0]->v.opaque;
    tmp->color = argv[1]->v.opaque;
    tmp->value = argv[2]->v.f;
    return vowned_opaque_extra(tmp, NULL, "struct MaterialMap");
}
Value* _type_mila_Material_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3) return verror("Material: wrong arg count");
    Material* tmp = malloc(sizeof(Material));
    tmp->shader = argv[0]->v.opaque;
    tmp->maps = argv[1]->v.opaque;
    memcpy(tmp->params, argv[2]->v.opaque, sizeof(tmp->params));
    return vowned_opaque_extra(tmp, NULL, "struct Material");
}
Value* _type_mila_Transform_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3) return verror("Transform: wrong arg count");
    Transform* tmp = malloc(sizeof(Transform));
    tmp->translation = argv[0]->v.opaque;
    tmp->rotation = argv[1]->v.opaque;
    tmp->scale = argv[2]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct Transform");
}
Value* _type_mila_BoneInfo_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2) return verror("BoneInfo: wrong arg count");
    BoneInfo* tmp = malloc(sizeof(BoneInfo));
    strncpy(tmp->name, argv[0]->v.s, sizeof(tmp->name));
    tmp->parent = ((int)argv[1]->v.i);
    return vowned_opaque_extra(tmp, NULL, "struct BoneInfo");
}
Value* _type_mila_Model_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 9) return verror("Model: wrong arg count");
    Model* tmp = malloc(sizeof(Model));
    tmp->transform = argv[0]->v.opaque;
    tmp->meshCount = ((int)argv[1]->v.i);
    tmp->materialCount = ((int)argv[2]->v.i);
    tmp->meshes = argv[3]->v.opaque;
    tmp->materials = argv[4]->v.opaque;
    tmp->meshMaterial = argv[5]->v.opaque;
    tmp->boneCount = ((int)argv[6]->v.i);
    tmp->bones = argv[7]->v.opaque;
    tmp->bindPose = argv[8]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct Model");
}
Value* _type_mila_ModelAnimation_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5) return verror("ModelAnimation: wrong arg count");
    ModelAnimation* tmp = malloc(sizeof(ModelAnimation));
    tmp->boneCount = ((int)argv[0]->v.i);
    tmp->frameCount = ((int)argv[1]->v.i);
    tmp->bones = argv[2]->v.opaque;
    tmp->framePoses = argv[3]->v.opaque;
    strncpy(tmp->name, argv[4]->v.s, sizeof(tmp->name));
    return vowned_opaque_extra(tmp, NULL, "struct ModelAnimation");
}
Value* _type_mila_Ray_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2) return verror("Ray: wrong arg count");
    Ray* tmp = malloc(sizeof(Ray));
    tmp->position = argv[0]->v.opaque;
    tmp->direction = argv[1]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct Ray");
}
Value* _type_mila_RayCollision_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 4) return verror("RayCollision: wrong arg count");
    RayCollision* tmp = malloc(sizeof(RayCollision));
    tmp->hit = ((int)argv[0]->v.i);
    tmp->distance = argv[1]->v.f;
    tmp->point = argv[2]->v.opaque;
    tmp->normal = argv[3]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct RayCollision");
}
Value* _type_mila_BoundingBox_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2) return verror("BoundingBox: wrong arg count");
    BoundingBox* tmp = malloc(sizeof(BoundingBox));
    tmp->min = argv[0]->v.opaque;
    tmp->max = argv[1]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct BoundingBox");
}
Value* _type_mila_Wave_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5) return verror("Wave: wrong arg count");
    Wave* tmp = malloc(sizeof(Wave));
    tmp->frameCount = argv[0]->v.i;
    tmp->sampleRate = argv[1]->v.i;
    tmp->sampleSize = argv[2]->v.i;
    tmp->channels = argv[3]->v.i;
    tmp->data = argv[4]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct Wave");
}
Value* _type_mila_rAudioBuffer_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0) return verror("rAudioBuffer: wrong arg count");
    rAudioBuffer* tmp = malloc(sizeof(rAudioBuffer));
    return vowned_opaque_extra(tmp, NULL, "struct rAudioBuffer");
}
Value* _type_mila_rAudioProcessor_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 0) return verror("rAudioProcessor: wrong arg count");
    rAudioProcessor* tmp = malloc(sizeof(rAudioProcessor));
    return vowned_opaque_extra(tmp, NULL, "struct rAudioProcessor");
}
Value* _type_mila_AudioStream_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5) return verror("AudioStream: wrong arg count");
    AudioStream* tmp = malloc(sizeof(AudioStream));
    tmp->buffer = argv[0]->v.opaque;
    tmp->processor = argv[1]->v.opaque;
    tmp->sampleRate = argv[2]->v.i;
    tmp->sampleSize = argv[3]->v.i;
    tmp->channels = argv[4]->v.i;
    return vowned_opaque_extra(tmp, NULL, "struct AudioStream");
}
Value* _type_mila_Sound_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 2) return verror("Sound: wrong arg count");
    Sound* tmp = malloc(sizeof(Sound));
    tmp->stream = argv[0]->v.opaque;
    tmp->frameCount = argv[1]->v.i;
    return vowned_opaque_extra(tmp, NULL, "struct Sound");
}
Value* _type_mila_Music_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 5) return verror("Music: wrong arg count");
    Music* tmp = malloc(sizeof(Music));
    tmp->stream = argv[0]->v.opaque;
    tmp->frameCount = argv[1]->v.i;
    tmp->looping = ((int)argv[2]->v.i);
    tmp->ctxType = ((int)argv[3]->v.i);
    tmp->ctxData = argv[4]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct Music");
}
Value* _type_mila_VrDeviceInfo_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 9) return verror("VrDeviceInfo: wrong arg count");
    VrDeviceInfo* tmp = malloc(sizeof(VrDeviceInfo));
    tmp->hResolution = ((int)argv[0]->v.i);
    tmp->vResolution = ((int)argv[1]->v.i);
    tmp->hScreenSize = argv[2]->v.f;
    tmp->vScreenSize = argv[3]->v.f;
    tmp->eyeToScreenDistance = argv[4]->v.f;
    tmp->lensSeparationDistance = argv[5]->v.f;
    tmp->interpupillaryDistance = argv[6]->v.f;
    memcpy(tmp->lensDistortionValues, argv[7]->v.opaque, sizeof(tmp->lensDistortionValues));
    memcpy(tmp->chromaAbCorrection, argv[8]->v.opaque, sizeof(tmp->chromaAbCorrection));
    return vowned_opaque_extra(tmp, NULL, "struct VrDeviceInfo");
}
Value* _type_mila_VrStereoConfig_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 8) return verror("VrStereoConfig: wrong arg count");
    VrStereoConfig* tmp = malloc(sizeof(VrStereoConfig));
    memcpy(tmp->projection, argv[0]->v.opaque, sizeof(tmp->projection));
    memcpy(tmp->viewOffset, argv[1]->v.opaque, sizeof(tmp->viewOffset));
    memcpy(tmp->leftLensCenter, argv[2]->v.opaque, sizeof(tmp->leftLensCenter));
    memcpy(tmp->rightLensCenter, argv[3]->v.opaque, sizeof(tmp->rightLensCenter));
    memcpy(tmp->leftScreenCenter, argv[4]->v.opaque, sizeof(tmp->leftScreenCenter));
    memcpy(tmp->rightScreenCenter, argv[5]->v.opaque, sizeof(tmp->rightScreenCenter));
    memcpy(tmp->scale, argv[6]->v.opaque, sizeof(tmp->scale));
    memcpy(tmp->scaleIn, argv[7]->v.opaque, sizeof(tmp->scaleIn));
    return vowned_opaque_extra(tmp, NULL, "struct VrStereoConfig");
}
Value* _type_mila_FilePathList_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3) return verror("FilePathList: wrong arg count");
    FilePathList* tmp = malloc(sizeof(FilePathList));
    tmp->capacity = argv[0]->v.i;
    tmp->count = argv[1]->v.i;
    tmp->paths = argv[2]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct FilePathList");
}
Value* _type_mila_AutomationEvent_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3) return verror("AutomationEvent: wrong arg count");
    AutomationEvent* tmp = malloc(sizeof(AutomationEvent));
    tmp->frame = argv[0]->v.i;
    tmp->type = argv[1]->v.i;
    memcpy(tmp->params, argv[2]->v.opaque, sizeof(tmp->params));
    return vowned_opaque_extra(tmp, NULL, "struct AutomationEvent");
}
Value* _type_mila_AutomationEventList_new(Env* e, int argc, Value** argv) {
    (void)e;
    if(argc != 3) return verror("AutomationEventList: wrong arg count");
    AutomationEventList* tmp = malloc(sizeof(AutomationEventList));
    tmp->capacity = argv[0]->v.i;
    tmp->count = argv[1]->v.i;
    tmp->events = argv[2]->v.opaque;
    return vowned_opaque_extra(tmp, NULL, "struct AutomationEventList");
}
const NativeEntry lib_function_entries[] = {
    {"InitWindow", native_mila_InitWindow},
    {"CloseWindow", native_mila_CloseWindow},
    {"WindowShouldClose", native_mila_WindowShouldClose},
    {"IsWindowReady", native_mila_IsWindowReady},
    {"IsWindowFullscreen", native_mila_IsWindowFullscreen},
    {"IsWindowHidden", native_mila_IsWindowHidden},
    {"IsWindowMinimized", native_mila_IsWindowMinimized},
    {"IsWindowMaximized", native_mila_IsWindowMaximized},
    {"IsWindowFocused", native_mila_IsWindowFocused},
    {"IsWindowResized", native_mila_IsWindowResized},
    {"IsWindowState", native_mila_IsWindowState},
    {"SetWindowState", native_mila_SetWindowState},
    {"ClearWindowState", native_mila_ClearWindowState},
    {"ToggleFullscreen", native_mila_ToggleFullscreen},
    {"ToggleBorderlessWindowed", native_mila_ToggleBorderlessWindowed},
    {"MaximizeWindow", native_mila_MaximizeWindow},
    {"MinimizeWindow", native_mila_MinimizeWindow},
    {"RestoreWindow", native_mila_RestoreWindow},
    {"SetWindowIcon", native_mila_SetWindowIcon},
    {"SetWindowIcons", native_mila_SetWindowIcons},
    {"SetWindowTitle", native_mila_SetWindowTitle},
    {"SetWindowPosition", native_mila_SetWindowPosition},
    {"SetWindowMonitor", native_mila_SetWindowMonitor},
    {"SetWindowMinSize", native_mila_SetWindowMinSize},
    {"SetWindowMaxSize", native_mila_SetWindowMaxSize},
    {"SetWindowSize", native_mila_SetWindowSize},
    {"SetWindowOpacity", native_mila_SetWindowOpacity},
    {"SetWindowFocused", native_mila_SetWindowFocused},
    {"GetWindowHandle", native_mila_GetWindowHandle},
    {"GetScreenWidth", native_mila_GetScreenWidth},
    {"GetScreenHeight", native_mila_GetScreenHeight},
    {"GetRenderWidth", native_mila_GetRenderWidth},
    {"GetRenderHeight", native_mila_GetRenderHeight},
    {"GetMonitorCount", native_mila_GetMonitorCount},
    {"GetCurrentMonitor", native_mila_GetCurrentMonitor},
    {"GetMonitorPosition", native_mila_GetMonitorPosition},
    {"GetMonitorWidth", native_mila_GetMonitorWidth},
    {"GetMonitorHeight", native_mila_GetMonitorHeight},
    {"GetMonitorPhysicalWidth", native_mila_GetMonitorPhysicalWidth},
    {"GetMonitorPhysicalHeight", native_mila_GetMonitorPhysicalHeight},
    {"GetMonitorRefreshRate", native_mila_GetMonitorRefreshRate},
    {"GetWindowPosition", native_mila_GetWindowPosition},
    {"GetWindowScaleDPI", native_mila_GetWindowScaleDPI},
    {"GetMonitorName", native_mila_GetMonitorName},
    {"SetClipboardText", native_mila_SetClipboardText},
    {"GetClipboardText", native_mila_GetClipboardText},
    {"GetClipboardImage", native_mila_GetClipboardImage},
    {"EnableEventWaiting", native_mila_EnableEventWaiting},
    {"DisableEventWaiting", native_mila_DisableEventWaiting},
    {"ShowCursor", native_mila_ShowCursor},
    {"HideCursor", native_mila_HideCursor},
    {"IsCursorHidden", native_mila_IsCursorHidden},
    {"EnableCursor", native_mila_EnableCursor},
    {"DisableCursor", native_mila_DisableCursor},
    {"IsCursorOnScreen", native_mila_IsCursorOnScreen},
    {"ClearBackground", native_mila_ClearBackground},
    {"BeginDrawing", native_mila_BeginDrawing},
    {"EndDrawing", native_mila_EndDrawing},
    {"BeginMode2D", native_mila_BeginMode2D},
    {"EndMode2D", native_mila_EndMode2D},
    {"BeginMode3D", native_mila_BeginMode3D},
    {"EndMode3D", native_mila_EndMode3D},
    {"BeginTextureMode", native_mila_BeginTextureMode},
    {"EndTextureMode", native_mila_EndTextureMode},
    {"BeginShaderMode", native_mila_BeginShaderMode},
    {"EndShaderMode", native_mila_EndShaderMode},
    {"BeginBlendMode", native_mila_BeginBlendMode},
    {"EndBlendMode", native_mila_EndBlendMode},
    {"BeginScissorMode", native_mila_BeginScissorMode},
    {"EndScissorMode", native_mila_EndScissorMode},
    {"BeginVrStereoMode", native_mila_BeginVrStereoMode},
    {"EndVrStereoMode", native_mila_EndVrStereoMode},
    {"LoadVrStereoConfig", native_mila_LoadVrStereoConfig},
    {"UnloadVrStereoConfig", native_mila_UnloadVrStereoConfig},
    {"LoadShader", native_mila_LoadShader},
    {"LoadShaderFromMemory", native_mila_LoadShaderFromMemory},
    {"IsShaderValid", native_mila_IsShaderValid},
    {"GetShaderLocation", native_mila_GetShaderLocation},
    {"GetShaderLocationAttrib", native_mila_GetShaderLocationAttrib},
    {"SetShaderValue", native_mila_SetShaderValue},
    {"SetShaderValueV", native_mila_SetShaderValueV},
    {"SetShaderValueMatrix", native_mila_SetShaderValueMatrix},
    {"SetShaderValueTexture", native_mila_SetShaderValueTexture},
    {"UnloadShader", native_mila_UnloadShader},
    {"GetScreenToWorldRay", native_mila_GetScreenToWorldRay},
    {"GetScreenToWorldRayEx", native_mila_GetScreenToWorldRayEx},
    {"GetWorldToScreen", native_mila_GetWorldToScreen},
    {"GetWorldToScreenEx", native_mila_GetWorldToScreenEx},
    {"GetWorldToScreen2D", native_mila_GetWorldToScreen2D},
    {"GetScreenToWorld2D", native_mila_GetScreenToWorld2D},
    {"GetCameraMatrix", native_mila_GetCameraMatrix},
    {"GetCameraMatrix2D", native_mila_GetCameraMatrix2D},
    {"SetTargetFPS", native_mila_SetTargetFPS},
    {"GetFrameTime", native_mila_GetFrameTime},
    {"GetTime", native_mila_GetTime},
    {"GetFPS", native_mila_GetFPS},
    {"SwapScreenBuffer", native_mila_SwapScreenBuffer},
    {"PollInputEvents", native_mila_PollInputEvents},
    {"WaitTime", native_mila_WaitTime},
    {"SetRandomSeed", native_mila_SetRandomSeed},
    {"GetRandomValue", native_mila_GetRandomValue},
    {"LoadRandomSequence", native_mila_LoadRandomSequence},
    {"UnloadRandomSequence", native_mila_UnloadRandomSequence},
    {"TakeScreenshot", native_mila_TakeScreenshot},
    {"SetConfigFlags", native_mila_SetConfigFlags},
    {"OpenURL", native_mila_OpenURL},
    {"TraceLog", native_mila_TraceLog},
    {"SetTraceLogLevel", native_mila_SetTraceLogLevel},
    {"MemAlloc", native_mila_MemAlloc},
    {"MemRealloc", native_mila_MemRealloc},
    {"MemFree", native_mila_MemFree},
    {"SetTraceLogCallback", native_mila_SetTraceLogCallback},
    {"SetLoadFileDataCallback", native_mila_SetLoadFileDataCallback},
    {"SetSaveFileDataCallback", native_mila_SetSaveFileDataCallback},
    {"SetLoadFileTextCallback", native_mila_SetLoadFileTextCallback},
    {"SetSaveFileTextCallback", native_mila_SetSaveFileTextCallback},
    {"LoadFileData", native_mila_LoadFileData},
    {"UnloadFileData", native_mila_UnloadFileData},
    {"SaveFileData", native_mila_SaveFileData},
    {"ExportDataAsCode", native_mila_ExportDataAsCode},
    {"LoadFileText", native_mila_LoadFileText},
    {"UnloadFileText", native_mila_UnloadFileText},
    {"SaveFileText", native_mila_SaveFileText},
    {"FileExists", native_mila_FileExists},
    {"DirectoryExists", native_mila_DirectoryExists},
    {"IsFileExtension", native_mila_IsFileExtension},
    {"GetFileLength", native_mila_GetFileLength},
    {"GetFileExtension", native_mila_GetFileExtension},
    {"GetFileName", native_mila_GetFileName},
    {"GetFileNameWithoutExt", native_mila_GetFileNameWithoutExt},
    {"GetDirectoryPath", native_mila_GetDirectoryPath},
    {"GetPrevDirectoryPath", native_mila_GetPrevDirectoryPath},
    {"GetWorkingDirectory", native_mila_GetWorkingDirectory},
    {"GetApplicationDirectory", native_mila_GetApplicationDirectory},
    {"MakeDirectory", native_mila_MakeDirectory},
    {"ChangeDirectory", native_mila_ChangeDirectory},
    {"IsPathFile", native_mila_IsPathFile},
    {"IsFileNameValid", native_mila_IsFileNameValid},
    {"LoadDirectoryFiles", native_mila_LoadDirectoryFiles},
    {"LoadDirectoryFilesEx", native_mila_LoadDirectoryFilesEx},
    {"UnloadDirectoryFiles", native_mila_UnloadDirectoryFiles},
    {"IsFileDropped", native_mila_IsFileDropped},
    {"LoadDroppedFiles", native_mila_LoadDroppedFiles},
    {"UnloadDroppedFiles", native_mila_UnloadDroppedFiles},
    {"GetFileModTime", native_mila_GetFileModTime},
    {"CompressData", native_mila_CompressData},
    {"DecompressData", native_mila_DecompressData},
    {"EncodeDataBase64", native_mila_EncodeDataBase64},
    {"DecodeDataBase64", native_mila_DecodeDataBase64},
    {"ComputeCRC32", native_mila_ComputeCRC32},
    {"ComputeMD5", native_mila_ComputeMD5},
    {"ComputeSHA1", native_mila_ComputeSHA1},
    {"LoadAutomationEventList", native_mila_LoadAutomationEventList},
    {"UnloadAutomationEventList", native_mila_UnloadAutomationEventList},
    {"ExportAutomationEventList", native_mila_ExportAutomationEventList},
    {"SetAutomationEventList", native_mila_SetAutomationEventList},
    {"SetAutomationEventBaseFrame", native_mila_SetAutomationEventBaseFrame},
    {"StartAutomationEventRecording", native_mila_StartAutomationEventRecording},
    {"StopAutomationEventRecording", native_mila_StopAutomationEventRecording},
    {"PlayAutomationEvent", native_mila_PlayAutomationEvent},
    {"IsKeyPressed", native_mila_IsKeyPressed},
    {"IsKeyPressedRepeat", native_mila_IsKeyPressedRepeat},
    {"IsKeyDown", native_mila_IsKeyDown},
    {"IsKeyReleased", native_mila_IsKeyReleased},
    {"IsKeyUp", native_mila_IsKeyUp},
    {"GetKeyPressed", native_mila_GetKeyPressed},
    {"GetCharPressed", native_mila_GetCharPressed},
    {"SetExitKey", native_mila_SetExitKey},
    {"IsGamepadAvailable", native_mila_IsGamepadAvailable},
    {"GetGamepadName", native_mila_GetGamepadName},
    {"IsGamepadButtonPressed", native_mila_IsGamepadButtonPressed},
    {"IsGamepadButtonDown", native_mila_IsGamepadButtonDown},
    {"IsGamepadButtonReleased", native_mila_IsGamepadButtonReleased},
    {"IsGamepadButtonUp", native_mila_IsGamepadButtonUp},
    {"GetGamepadButtonPressed", native_mila_GetGamepadButtonPressed},
    {"GetGamepadAxisCount", native_mila_GetGamepadAxisCount},
    {"GetGamepadAxisMovement", native_mila_GetGamepadAxisMovement},
    {"SetGamepadMappings", native_mila_SetGamepadMappings},
    {"SetGamepadVibration", native_mila_SetGamepadVibration},
    {"IsMouseButtonPressed", native_mila_IsMouseButtonPressed},
    {"IsMouseButtonDown", native_mila_IsMouseButtonDown},
    {"IsMouseButtonReleased", native_mila_IsMouseButtonReleased},
    {"IsMouseButtonUp", native_mila_IsMouseButtonUp},
    {"GetMouseX", native_mila_GetMouseX},
    {"GetMouseY", native_mila_GetMouseY},
    {"GetMousePosition", native_mila_GetMousePosition},
    {"GetMouseDelta", native_mila_GetMouseDelta},
    {"SetMousePosition", native_mila_SetMousePosition},
    {"SetMouseOffset", native_mila_SetMouseOffset},
    {"SetMouseScale", native_mila_SetMouseScale},
    {"GetMouseWheelMove", native_mila_GetMouseWheelMove},
    {"GetMouseWheelMoveV", native_mila_GetMouseWheelMoveV},
    {"SetMouseCursor", native_mila_SetMouseCursor},
    {"GetTouchX", native_mila_GetTouchX},
    {"GetTouchY", native_mila_GetTouchY},
    {"GetTouchPosition", native_mila_GetTouchPosition},
    {"GetTouchPointId", native_mila_GetTouchPointId},
    {"GetTouchPointCount", native_mila_GetTouchPointCount},
    {"SetGesturesEnabled", native_mila_SetGesturesEnabled},
    {"IsGestureDetected", native_mila_IsGestureDetected},
    {"GetGestureDetected", native_mila_GetGestureDetected},
    {"GetGestureHoldDuration", native_mila_GetGestureHoldDuration},
    {"GetGestureDragVector", native_mila_GetGestureDragVector},
    {"GetGestureDragAngle", native_mila_GetGestureDragAngle},
    {"GetGesturePinchVector", native_mila_GetGesturePinchVector},
    {"GetGesturePinchAngle", native_mila_GetGesturePinchAngle},
    {"UpdateCamera", native_mila_UpdateCamera},
    {"UpdateCameraPro", native_mila_UpdateCameraPro},
    {"SetShapesTexture", native_mila_SetShapesTexture},
    {"GetShapesTexture", native_mila_GetShapesTexture},
    {"GetShapesTextureRectangle", native_mila_GetShapesTextureRectangle},
    {"DrawPixel", native_mila_DrawPixel},
    {"DrawPixelV", native_mila_DrawPixelV},
    {"DrawLine", native_mila_DrawLine},
    {"DrawLineV", native_mila_DrawLineV},
    {"DrawLineEx", native_mila_DrawLineEx},
    {"DrawLineStrip", native_mila_DrawLineStrip},
    {"DrawLineBezier", native_mila_DrawLineBezier},
    {"DrawCircle", native_mila_DrawCircle},
    {"DrawCircleSector", native_mila_DrawCircleSector},
    {"DrawCircleSectorLines", native_mila_DrawCircleSectorLines},
    {"DrawCircleGradient", native_mila_DrawCircleGradient},
    {"DrawCircleV", native_mila_DrawCircleV},
    {"DrawCircleLines", native_mila_DrawCircleLines},
    {"DrawCircleLinesV", native_mila_DrawCircleLinesV},
    {"DrawEllipse", native_mila_DrawEllipse},
    {"DrawEllipseLines", native_mila_DrawEllipseLines},
    {"DrawRing", native_mila_DrawRing},
    {"DrawRingLines", native_mila_DrawRingLines},
    {"DrawRectangle", native_mila_DrawRectangle},
    {"DrawRectangleV", native_mila_DrawRectangleV},
    {"DrawRectangleRec", native_mila_DrawRectangleRec},
    {"DrawRectanglePro", native_mila_DrawRectanglePro},
    {"DrawRectangleGradientV", native_mila_DrawRectangleGradientV},
    {"DrawRectangleGradientH", native_mila_DrawRectangleGradientH},
    {"DrawRectangleGradientEx", native_mila_DrawRectangleGradientEx},
    {"DrawRectangleLines", native_mila_DrawRectangleLines},
    {"DrawRectangleLinesEx", native_mila_DrawRectangleLinesEx},
    {"DrawRectangleRounded", native_mila_DrawRectangleRounded},
    {"DrawRectangleRoundedLines", native_mila_DrawRectangleRoundedLines},
    {"DrawRectangleRoundedLinesEx", native_mila_DrawRectangleRoundedLinesEx},
    {"DrawTriangle", native_mila_DrawTriangle},
    {"DrawTriangleLines", native_mila_DrawTriangleLines},
    {"DrawTriangleFan", native_mila_DrawTriangleFan},
    {"DrawTriangleStrip", native_mila_DrawTriangleStrip},
    {"DrawPoly", native_mila_DrawPoly},
    {"DrawPolyLines", native_mila_DrawPolyLines},
    {"DrawPolyLinesEx", native_mila_DrawPolyLinesEx},
    {"DrawSplineLinear", native_mila_DrawSplineLinear},
    {"DrawSplineBasis", native_mila_DrawSplineBasis},
    {"DrawSplineCatmullRom", native_mila_DrawSplineCatmullRom},
    {"DrawSplineBezierQuadratic", native_mila_DrawSplineBezierQuadratic},
    {"DrawSplineBezierCubic", native_mila_DrawSplineBezierCubic},
    {"DrawSplineSegmentLinear", native_mila_DrawSplineSegmentLinear},
    {"DrawSplineSegmentBasis", native_mila_DrawSplineSegmentBasis},
    {"DrawSplineSegmentCatmullRom", native_mila_DrawSplineSegmentCatmullRom},
    {"DrawSplineSegmentBezierQuadratic", native_mila_DrawSplineSegmentBezierQuadratic},
    {"DrawSplineSegmentBezierCubic", native_mila_DrawSplineSegmentBezierCubic},
    {"GetSplinePointLinear", native_mila_GetSplinePointLinear},
    {"GetSplinePointBasis", native_mila_GetSplinePointBasis},
    {"GetSplinePointCatmullRom", native_mila_GetSplinePointCatmullRom},
    {"GetSplinePointBezierQuad", native_mila_GetSplinePointBezierQuad},
    {"GetSplinePointBezierCubic", native_mila_GetSplinePointBezierCubic},
    {"CheckCollisionRecs", native_mila_CheckCollisionRecs},
    {"CheckCollisionCircles", native_mila_CheckCollisionCircles},
    {"CheckCollisionCircleRec", native_mila_CheckCollisionCircleRec},
    {"CheckCollisionCircleLine", native_mila_CheckCollisionCircleLine},
    {"CheckCollisionPointRec", native_mila_CheckCollisionPointRec},
    {"CheckCollisionPointCircle", native_mila_CheckCollisionPointCircle},
    {"CheckCollisionPointTriangle", native_mila_CheckCollisionPointTriangle},
    {"CheckCollisionPointLine", native_mila_CheckCollisionPointLine},
    {"CheckCollisionPointPoly", native_mila_CheckCollisionPointPoly},
    {"CheckCollisionLines", native_mila_CheckCollisionLines},
    {"GetCollisionRec", native_mila_GetCollisionRec},
    {"LoadImage", native_mila_LoadImage},
    {"LoadImageRaw", native_mila_LoadImageRaw},
    {"LoadImageAnim", native_mila_LoadImageAnim},
    {"LoadImageAnimFromMemory", native_mila_LoadImageAnimFromMemory},
    {"LoadImageFromMemory", native_mila_LoadImageFromMemory},
    {"LoadImageFromTexture", native_mila_LoadImageFromTexture},
    {"LoadImageFromScreen", native_mila_LoadImageFromScreen},
    {"IsImageValid", native_mila_IsImageValid},
    {"UnloadImage", native_mila_UnloadImage},
    {"ExportImage", native_mila_ExportImage},
    {"ExportImageToMemory", native_mila_ExportImageToMemory},
    {"ExportImageAsCode", native_mila_ExportImageAsCode},
    {"GenImageColor", native_mila_GenImageColor},
    {"GenImageGradientLinear", native_mila_GenImageGradientLinear},
    {"GenImageGradientRadial", native_mila_GenImageGradientRadial},
    {"GenImageGradientSquare", native_mila_GenImageGradientSquare},
    {"GenImageChecked", native_mila_GenImageChecked},
    {"GenImageWhiteNoise", native_mila_GenImageWhiteNoise},
    {"GenImagePerlinNoise", native_mila_GenImagePerlinNoise},
    {"GenImageCellular", native_mila_GenImageCellular},
    {"GenImageText", native_mila_GenImageText},
    {"ImageCopy", native_mila_ImageCopy},
    {"ImageFromImage", native_mila_ImageFromImage},
    {"ImageFromChannel", native_mila_ImageFromChannel},
    {"ImageText", native_mila_ImageText},
    {"ImageTextEx", native_mila_ImageTextEx},
    {"ImageFormat", native_mila_ImageFormat},
    {"ImageToPOT", native_mila_ImageToPOT},
    {"ImageCrop", native_mila_ImageCrop},
    {"ImageAlphaCrop", native_mila_ImageAlphaCrop},
    {"ImageAlphaClear", native_mila_ImageAlphaClear},
    {"ImageAlphaMask", native_mila_ImageAlphaMask},
    {"ImageAlphaPremultiply", native_mila_ImageAlphaPremultiply},
    {"ImageBlurGaussian", native_mila_ImageBlurGaussian},
    {"ImageKernelConvolution", native_mila_ImageKernelConvolution},
    {"ImageResize", native_mila_ImageResize},
    {"ImageResizeNN", native_mila_ImageResizeNN},
    {"ImageResizeCanvas", native_mila_ImageResizeCanvas},
    {"ImageMipmaps", native_mila_ImageMipmaps},
    {"ImageDither", native_mila_ImageDither},
    {"ImageFlipVertical", native_mila_ImageFlipVertical},
    {"ImageFlipHorizontal", native_mila_ImageFlipHorizontal},
    {"ImageRotate", native_mila_ImageRotate},
    {"ImageRotateCW", native_mila_ImageRotateCW},
    {"ImageRotateCCW", native_mila_ImageRotateCCW},
    {"ImageColorTint", native_mila_ImageColorTint},
    {"ImageColorInvert", native_mila_ImageColorInvert},
    {"ImageColorGrayscale", native_mila_ImageColorGrayscale},
    {"ImageColorContrast", native_mila_ImageColorContrast},
    {"ImageColorBrightness", native_mila_ImageColorBrightness},
    {"ImageColorReplace", native_mila_ImageColorReplace},
    {"LoadImageColors", native_mila_LoadImageColors},
    {"LoadImagePalette", native_mila_LoadImagePalette},
    {"UnloadImageColors", native_mila_UnloadImageColors},
    {"UnloadImagePalette", native_mila_UnloadImagePalette},
    {"GetImageAlphaBorder", native_mila_GetImageAlphaBorder},
    {"GetImageColor", native_mila_GetImageColor},
    {"ImageClearBackground", native_mila_ImageClearBackground},
    {"ImageDrawPixel", native_mila_ImageDrawPixel},
    {"ImageDrawPixelV", native_mila_ImageDrawPixelV},
    {"ImageDrawLine", native_mila_ImageDrawLine},
    {"ImageDrawLineV", native_mila_ImageDrawLineV},
    {"ImageDrawLineEx", native_mila_ImageDrawLineEx},
    {"ImageDrawCircle", native_mila_ImageDrawCircle},
    {"ImageDrawCircleV", native_mila_ImageDrawCircleV},
    {"ImageDrawCircleLines", native_mila_ImageDrawCircleLines},
    {"ImageDrawCircleLinesV", native_mila_ImageDrawCircleLinesV},
    {"ImageDrawRectangle", native_mila_ImageDrawRectangle},
    {"ImageDrawRectangleV", native_mila_ImageDrawRectangleV},
    {"ImageDrawRectangleRec", native_mila_ImageDrawRectangleRec},
    {"ImageDrawRectangleLines", native_mila_ImageDrawRectangleLines},
    {"ImageDrawTriangle", native_mila_ImageDrawTriangle},
    {"ImageDrawTriangleEx", native_mila_ImageDrawTriangleEx},
    {"ImageDrawTriangleLines", native_mila_ImageDrawTriangleLines},
    {"ImageDrawTriangleFan", native_mila_ImageDrawTriangleFan},
    {"ImageDrawTriangleStrip", native_mila_ImageDrawTriangleStrip},
    {"ImageDraw", native_mila_ImageDraw},
    {"ImageDrawText", native_mila_ImageDrawText},
    {"ImageDrawTextEx", native_mila_ImageDrawTextEx},
    {"LoadTexture", native_mila_LoadTexture},
    {"LoadTextureFromImage", native_mila_LoadTextureFromImage},
    {"LoadTextureCubemap", native_mila_LoadTextureCubemap},
    {"LoadRenderTexture", native_mila_LoadRenderTexture},
    {"IsTextureValid", native_mila_IsTextureValid},
    {"UnloadTexture", native_mila_UnloadTexture},
    {"IsRenderTextureValid", native_mila_IsRenderTextureValid},
    {"UnloadRenderTexture", native_mila_UnloadRenderTexture},
    {"UpdateTexture", native_mila_UpdateTexture},
    {"UpdateTextureRec", native_mila_UpdateTextureRec},
    {"GenTextureMipmaps", native_mila_GenTextureMipmaps},
    {"SetTextureFilter", native_mila_SetTextureFilter},
    {"SetTextureWrap", native_mila_SetTextureWrap},
    {"DrawTexture", native_mila_DrawTexture},
    {"DrawTextureV", native_mila_DrawTextureV},
    {"DrawTextureEx", native_mila_DrawTextureEx},
    {"DrawTextureRec", native_mila_DrawTextureRec},
    {"DrawTexturePro", native_mila_DrawTexturePro},
    {"DrawTextureNPatch", native_mila_DrawTextureNPatch},
    {"ColorIsEqual", native_mila_ColorIsEqual},
    {"Fade", native_mila_Fade},
    {"ColorToInt", native_mila_ColorToInt},
    {"ColorNormalize", native_mila_ColorNormalize},
    {"ColorFromNormalized", native_mila_ColorFromNormalized},
    {"ColorToHSV", native_mila_ColorToHSV},
    {"ColorFromHSV", native_mila_ColorFromHSV},
    {"ColorTint", native_mila_ColorTint},
    {"ColorBrightness", native_mila_ColorBrightness},
    {"ColorContrast", native_mila_ColorContrast},
    {"ColorAlpha", native_mila_ColorAlpha},
    {"ColorAlphaBlend", native_mila_ColorAlphaBlend},
    {"ColorLerp", native_mila_ColorLerp},
    {"GetColor", native_mila_GetColor},
    {"GetPixelColor", native_mila_GetPixelColor},
    {"SetPixelColor", native_mila_SetPixelColor},
    {"GetPixelDataSize", native_mila_GetPixelDataSize},
    {"GetFontDefault", native_mila_GetFontDefault},
    {"LoadFont", native_mila_LoadFont},
    {"LoadFontEx", native_mila_LoadFontEx},
    {"LoadFontFromImage", native_mila_LoadFontFromImage},
    {"LoadFontFromMemory", native_mila_LoadFontFromMemory},
    {"IsFontValid", native_mila_IsFontValid},
    {"LoadFontData", native_mila_LoadFontData},
    {"GenImageFontAtlas", native_mila_GenImageFontAtlas},
    {"UnloadFontData", native_mila_UnloadFontData},
    {"UnloadFont", native_mila_UnloadFont},
    {"ExportFontAsCode", native_mila_ExportFontAsCode},
    {"DrawFPS", native_mila_DrawFPS},
    {"DrawText", native_mila_DrawText},
    {"DrawTextEx", native_mila_DrawTextEx},
    {"DrawTextPro", native_mila_DrawTextPro},
    {"DrawTextCodepoint", native_mila_DrawTextCodepoint},
    {"DrawTextCodepoints", native_mila_DrawTextCodepoints},
    {"SetTextLineSpacing", native_mila_SetTextLineSpacing},
    {"MeasureText", native_mila_MeasureText},
    {"MeasureTextEx", native_mila_MeasureTextEx},
    {"GetGlyphIndex", native_mila_GetGlyphIndex},
    {"GetGlyphInfo", native_mila_GetGlyphInfo},
    {"GetGlyphAtlasRec", native_mila_GetGlyphAtlasRec},
    {"LoadUTF8", native_mila_LoadUTF8},
    {"UnloadUTF8", native_mila_UnloadUTF8},
    {"LoadCodepoints", native_mila_LoadCodepoints},
    {"UnloadCodepoints", native_mila_UnloadCodepoints},
    {"GetCodepointCount", native_mila_GetCodepointCount},
    {"GetCodepoint", native_mila_GetCodepoint},
    {"GetCodepointNext", native_mila_GetCodepointNext},
    {"GetCodepointPrevious", native_mila_GetCodepointPrevious},
    {"CodepointToUTF8", native_mila_CodepointToUTF8},
    {"TextCopy", native_mila_TextCopy},
    {"TextIsEqual", native_mila_TextIsEqual},
    {"TextLength", native_mila_TextLength},
    {"TextFormat", native_mila_TextFormat},
    {"TextSubtext", native_mila_TextSubtext},
    {"TextReplace", native_mila_TextReplace},
    {"TextInsert", native_mila_TextInsert},
    {"TextJoin", native_mila_TextJoin},
    {"TextSplit", native_mila_TextSplit},
    {"TextAppend", native_mila_TextAppend},
    {"TextFindIndex", native_mila_TextFindIndex},
    {"TextToUpper", native_mila_TextToUpper},
    {"TextToLower", native_mila_TextToLower},
    {"TextToPascal", native_mila_TextToPascal},
    {"TextToSnake", native_mila_TextToSnake},
    {"TextToCamel", native_mila_TextToCamel},
    {"TextToInteger", native_mila_TextToInteger},
    {"TextToFloat", native_mila_TextToFloat},
    {"DrawLine3D", native_mila_DrawLine3D},
    {"DrawPoint3D", native_mila_DrawPoint3D},
    {"DrawCircle3D", native_mila_DrawCircle3D},
    {"DrawTriangle3D", native_mila_DrawTriangle3D},
    {"DrawTriangleStrip3D", native_mila_DrawTriangleStrip3D},
    {"DrawCube", native_mila_DrawCube},
    {"DrawCubeV", native_mila_DrawCubeV},
    {"DrawCubeWires", native_mila_DrawCubeWires},
    {"DrawCubeWiresV", native_mila_DrawCubeWiresV},
    {"DrawSphere", native_mila_DrawSphere},
    {"DrawSphereEx", native_mila_DrawSphereEx},
    {"DrawSphereWires", native_mila_DrawSphereWires},
    {"DrawCylinder", native_mila_DrawCylinder},
    {"DrawCylinderEx", native_mila_DrawCylinderEx},
    {"DrawCylinderWires", native_mila_DrawCylinderWires},
    {"DrawCylinderWiresEx", native_mila_DrawCylinderWiresEx},
    {"DrawCapsule", native_mila_DrawCapsule},
    {"DrawCapsuleWires", native_mila_DrawCapsuleWires},
    {"DrawPlane", native_mila_DrawPlane},
    {"DrawRay", native_mila_DrawRay},
    {"DrawGrid", native_mila_DrawGrid},
    {"LoadModel", native_mila_LoadModel},
    {"LoadModelFromMesh", native_mila_LoadModelFromMesh},
    {"IsModelValid", native_mila_IsModelValid},
    {"UnloadModel", native_mila_UnloadModel},
    {"GetModelBoundingBox", native_mila_GetModelBoundingBox},
    {"DrawModel", native_mila_DrawModel},
    {"DrawModelEx", native_mila_DrawModelEx},
    {"DrawModelWires", native_mila_DrawModelWires},
    {"DrawModelWiresEx", native_mila_DrawModelWiresEx},
    {"DrawModelPoints", native_mila_DrawModelPoints},
    {"DrawModelPointsEx", native_mila_DrawModelPointsEx},
    {"DrawBoundingBox", native_mila_DrawBoundingBox},
    {"DrawBillboard", native_mila_DrawBillboard},
    {"DrawBillboardRec", native_mila_DrawBillboardRec},
    {"DrawBillboardPro", native_mila_DrawBillboardPro},
    {"UploadMesh", native_mila_UploadMesh},
    {"UpdateMeshBuffer", native_mila_UpdateMeshBuffer},
    {"UnloadMesh", native_mila_UnloadMesh},
    {"DrawMesh", native_mila_DrawMesh},
    {"DrawMeshInstanced", native_mila_DrawMeshInstanced},
    {"GetMeshBoundingBox", native_mila_GetMeshBoundingBox},
    {"GenMeshTangents", native_mila_GenMeshTangents},
    {"ExportMesh", native_mila_ExportMesh},
    {"ExportMeshAsCode", native_mila_ExportMeshAsCode},
    {"GenMeshPoly", native_mila_GenMeshPoly},
    {"GenMeshPlane", native_mila_GenMeshPlane},
    {"GenMeshCube", native_mila_GenMeshCube},
    {"GenMeshSphere", native_mila_GenMeshSphere},
    {"GenMeshHemiSphere", native_mila_GenMeshHemiSphere},
    {"GenMeshCylinder", native_mila_GenMeshCylinder},
    {"GenMeshCone", native_mila_GenMeshCone},
    {"GenMeshTorus", native_mila_GenMeshTorus},
    {"GenMeshKnot", native_mila_GenMeshKnot},
    {"GenMeshHeightmap", native_mila_GenMeshHeightmap},
    {"GenMeshCubicmap", native_mila_GenMeshCubicmap},
    {"LoadMaterials", native_mila_LoadMaterials},
    {"LoadMaterialDefault", native_mila_LoadMaterialDefault},
    {"IsMaterialValid", native_mila_IsMaterialValid},
    {"UnloadMaterial", native_mila_UnloadMaterial},
    {"SetMaterialTexture", native_mila_SetMaterialTexture},
    {"SetModelMeshMaterial", native_mila_SetModelMeshMaterial},
    {"LoadModelAnimations", native_mila_LoadModelAnimations},
    {"UpdateModelAnimation", native_mila_UpdateModelAnimation},
    {"UpdateModelAnimationBones", native_mila_UpdateModelAnimationBones},
    {"UnloadModelAnimation", native_mila_UnloadModelAnimation},
    {"UnloadModelAnimations", native_mila_UnloadModelAnimations},
    {"IsModelAnimationValid", native_mila_IsModelAnimationValid},
    {"CheckCollisionSpheres", native_mila_CheckCollisionSpheres},
    {"CheckCollisionBoxes", native_mila_CheckCollisionBoxes},
    {"CheckCollisionBoxSphere", native_mila_CheckCollisionBoxSphere},
    {"GetRayCollisionSphere", native_mila_GetRayCollisionSphere},
    {"GetRayCollisionBox", native_mila_GetRayCollisionBox},
    {"GetRayCollisionMesh", native_mila_GetRayCollisionMesh},
    {"GetRayCollisionTriangle", native_mila_GetRayCollisionTriangle},
    {"GetRayCollisionQuad", native_mila_GetRayCollisionQuad},
    {"InitAudioDevice", native_mila_InitAudioDevice},
    {"CloseAudioDevice", native_mila_CloseAudioDevice},
    {"IsAudioDeviceReady", native_mila_IsAudioDeviceReady},
    {"SetMasterVolume", native_mila_SetMasterVolume},
    {"GetMasterVolume", native_mila_GetMasterVolume},
    {"LoadWave", native_mila_LoadWave},
    {"LoadWaveFromMemory", native_mila_LoadWaveFromMemory},
    {"IsWaveValid", native_mila_IsWaveValid},
    {"LoadSound", native_mila_LoadSound},
    {"LoadSoundFromWave", native_mila_LoadSoundFromWave},
    {"LoadSoundAlias", native_mila_LoadSoundAlias},
    {"IsSoundValid", native_mila_IsSoundValid},
    {"UpdateSound", native_mila_UpdateSound},
    {"UnloadWave", native_mila_UnloadWave},
    {"UnloadSound", native_mila_UnloadSound},
    {"UnloadSoundAlias", native_mila_UnloadSoundAlias},
    {"ExportWave", native_mila_ExportWave},
    {"ExportWaveAsCode", native_mila_ExportWaveAsCode},
    {"PlaySound", native_mila_PlaySound},
    {"StopSound", native_mila_StopSound},
    {"PauseSound", native_mila_PauseSound},
    {"ResumeSound", native_mila_ResumeSound},
    {"IsSoundPlaying", native_mila_IsSoundPlaying},
    {"SetSoundVolume", native_mila_SetSoundVolume},
    {"SetSoundPitch", native_mila_SetSoundPitch},
    {"SetSoundPan", native_mila_SetSoundPan},
    {"WaveCopy", native_mila_WaveCopy},
    {"WaveCrop", native_mila_WaveCrop},
    {"WaveFormat", native_mila_WaveFormat},
    {"LoadWaveSamples", native_mila_LoadWaveSamples},
    {"UnloadWaveSamples", native_mila_UnloadWaveSamples},
    {"LoadMusicStream", native_mila_LoadMusicStream},
    {"LoadMusicStreamFromMemory", native_mila_LoadMusicStreamFromMemory},
    {"IsMusicValid", native_mila_IsMusicValid},
    {"UnloadMusicStream", native_mila_UnloadMusicStream},
    {"PlayMusicStream", native_mila_PlayMusicStream},
    {"IsMusicStreamPlaying", native_mila_IsMusicStreamPlaying},
    {"UpdateMusicStream", native_mila_UpdateMusicStream},
    {"StopMusicStream", native_mila_StopMusicStream},
    {"PauseMusicStream", native_mila_PauseMusicStream},
    {"ResumeMusicStream", native_mila_ResumeMusicStream},
    {"SeekMusicStream", native_mila_SeekMusicStream},
    {"SetMusicVolume", native_mila_SetMusicVolume},
    {"SetMusicPitch", native_mila_SetMusicPitch},
    {"SetMusicPan", native_mila_SetMusicPan},
    {"GetMusicTimeLength", native_mila_GetMusicTimeLength},
    {"GetMusicTimePlayed", native_mila_GetMusicTimePlayed},
    {"LoadAudioStream", native_mila_LoadAudioStream},
    {"IsAudioStreamValid", native_mila_IsAudioStreamValid},
    {"UnloadAudioStream", native_mila_UnloadAudioStream},
    {"UpdateAudioStream", native_mila_UpdateAudioStream},
    {"IsAudioStreamProcessed", native_mila_IsAudioStreamProcessed},
    {"PlayAudioStream", native_mila_PlayAudioStream},
    {"PauseAudioStream", native_mila_PauseAudioStream},
    {"ResumeAudioStream", native_mila_ResumeAudioStream},
    {"IsAudioStreamPlaying", native_mila_IsAudioStreamPlaying},
    {"StopAudioStream", native_mila_StopAudioStream},
    {"SetAudioStreamVolume", native_mila_SetAudioStreamVolume},
    {"SetAudioStreamPitch", native_mila_SetAudioStreamPitch},
    {"SetAudioStreamPan", native_mila_SetAudioStreamPan},
    {"SetAudioStreamBufferSizeDefault", native_mila_SetAudioStreamBufferSizeDefault},
    {"SetAudioStreamCallback", native_mila_SetAudioStreamCallback},
    {"AttachAudioStreamProcessor", native_mila_AttachAudioStreamProcessor},
    {"DetachAudioStreamProcessor", native_mila_DetachAudioStreamProcessor},
    {"AttachAudioMixedProcessor", native_mila_AttachAudioMixedProcessor},
    {"DetachAudioMixedProcessor", native_mila_DetachAudioMixedProcessor},
    {"Vector2", _type_mila_Vector2_new},
    {"Vector3", _type_mila_Vector3_new},
    {"Vector4", _type_mila_Vector4_new},
    {"Matrix", _type_mila_Matrix_new},
    {"Color", _type_mila_Color_new},
    {"Rectangle", _type_mila_Rectangle_new},
    {"Image", _type_mila_Image_new},
    {"Texture", _type_mila_Texture_new},
    {"RenderTexture", _type_mila_RenderTexture_new},
    {"NPatchInfo", _type_mila_NPatchInfo_new},
    {"GlyphInfo", _type_mila_GlyphInfo_new},
    {"Font", _type_mila_Font_new},
    {"Camera3D", _type_mila_Camera3D_new},
    {"Camera2D", _type_mila_Camera2D_new},
    {"Mesh", _type_mila_Mesh_new},
    {"Shader", _type_mila_Shader_new},
    {"MaterialMap", _type_mila_MaterialMap_new},
    {"Material", _type_mila_Material_new},
    {"Transform", _type_mila_Transform_new},
    {"BoneInfo", _type_mila_BoneInfo_new},
    {"Model", _type_mila_Model_new},
    {"ModelAnimation", _type_mila_ModelAnimation_new},
    {"Ray", _type_mila_Ray_new},
    {"RayCollision", _type_mila_RayCollision_new},
    {"BoundingBox", _type_mila_BoundingBox_new},
    {"Wave", _type_mila_Wave_new},
    {"rAudioBuffer", _type_mila_rAudioBuffer_new},
    {"rAudioProcessor", _type_mila_rAudioProcessor_new},
    {"AudioStream", _type_mila_AudioStream_new},
    {"Sound", _type_mila_Sound_new},
    {"Music", _type_mila_Music_new},
    {"VrDeviceInfo", _type_mila_VrDeviceInfo_new},
    {"VrStereoConfig", _type_mila_VrStereoConfig_new},
    {"FilePathList", _type_mila_FilePathList_new},
    {"AutomationEvent", _type_mila_AutomationEvent_new},
    {"AutomationEventList", _type_mila_AutomationEventList_new},
    {NULL, NULL}
};

void _mila_lib_init(Env* e) {
    env_set_raw(e, "ConfigFlags.FLAG_VSYNC_HINT", vint(64));
    env_set_raw(e, "ConfigFlags.FLAG_FULLSCREEN_MODE", vint(2));
    env_set_raw(e, "ConfigFlags.FLAG_WINDOW_RESIZABLE", vint(4));
    env_set_raw(e, "ConfigFlags.FLAG_WINDOW_UNDECORATED", vint(8));
    env_set_raw(e, "ConfigFlags.FLAG_WINDOW_HIDDEN", vint(128));
    env_set_raw(e, "ConfigFlags.FLAG_WINDOW_MINIMIZED", vint(512));
    env_set_raw(e, "ConfigFlags.FLAG_WINDOW_MAXIMIZED", vint(1024));
    env_set_raw(e, "ConfigFlags.FLAG_WINDOW_UNFOCUSED", vint(2048));
    env_set_raw(e, "ConfigFlags.FLAG_WINDOW_TOPMOST", vint(4096));
    env_set_raw(e, "ConfigFlags.FLAG_WINDOW_ALWAYS_RUN", vint(256));
    env_set_raw(e, "ConfigFlags.FLAG_WINDOW_TRANSPARENT", vint(16));
    env_set_raw(e, "ConfigFlags.FLAG_WINDOW_HIGHDPI", vint(8192));
    env_set_raw(e, "ConfigFlags.FLAG_WINDOW_MOUSE_PASSTHROUGH", vint(16384));
    env_set_raw(e, "ConfigFlags.FLAG_BORDERLESS_WINDOWED_MODE", vint(32768));
    env_set_raw(e, "ConfigFlags.FLAG_MSAA_4X_HINT", vint(32));
    env_set_raw(e, "ConfigFlags.FLAG_INTERLACED_HINT", vint(65536));
    env_set_raw(e, "TraceLogLevel.LOG_ALL", vint(0));
    env_set_raw(e, "TraceLogLevel.LOG_TRACE", vint(1));
    env_set_raw(e, "TraceLogLevel.LOG_DEBUG", vint(2));
    env_set_raw(e, "TraceLogLevel.LOG_INFO", vint(3));
    env_set_raw(e, "TraceLogLevel.LOG_WARNING", vint(4));
    env_set_raw(e, "TraceLogLevel.LOG_ERROR", vint(5));
    env_set_raw(e, "TraceLogLevel.LOG_FATAL", vint(6));
    env_set_raw(e, "TraceLogLevel.LOG_NONE", vint(7));
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
    env_set_raw(e, "MouseButton.MOUSE_BUTTON_LEFT", vint(0));
    env_set_raw(e, "MouseButton.MOUSE_BUTTON_RIGHT", vint(1));
    env_set_raw(e, "MouseButton.MOUSE_BUTTON_MIDDLE", vint(2));
    env_set_raw(e, "MouseButton.MOUSE_BUTTON_SIDE", vint(3));
    env_set_raw(e, "MouseButton.MOUSE_BUTTON_EXTRA", vint(4));
    env_set_raw(e, "MouseButton.MOUSE_BUTTON_FORWARD", vint(5));
    env_set_raw(e, "MouseButton.MOUSE_BUTTON_BACK", vint(6));
    env_set_raw(e, "MouseCursor.MOUSE_CURSOR_DEFAULT", vint(0));
    env_set_raw(e, "MouseCursor.MOUSE_CURSOR_ARROW", vint(1));
    env_set_raw(e, "MouseCursor.MOUSE_CURSOR_IBEAM", vint(2));
    env_set_raw(e, "MouseCursor.MOUSE_CURSOR_CROSSHAIR", vint(3));
    env_set_raw(e, "MouseCursor.MOUSE_CURSOR_POINTING_HAND", vint(4));
    env_set_raw(e, "MouseCursor.MOUSE_CURSOR_RESIZE_EW", vint(5));
    env_set_raw(e, "MouseCursor.MOUSE_CURSOR_RESIZE_NS", vint(6));
    env_set_raw(e, "MouseCursor.MOUSE_CURSOR_RESIZE_NWSE", vint(7));
    env_set_raw(e, "MouseCursor.MOUSE_CURSOR_RESIZE_NESW", vint(8));
    env_set_raw(e, "MouseCursor.MOUSE_CURSOR_RESIZE_ALL", vint(9));
    env_set_raw(e, "MouseCursor.MOUSE_CURSOR_NOT_ALLOWED", vint(10));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_UNKNOWN", vint(0));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_LEFT_FACE_UP", vint(1));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_LEFT_FACE_RIGHT", vint(2));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_LEFT_FACE_DOWN", vint(3));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_LEFT_FACE_LEFT", vint(4));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_RIGHT_FACE_UP", vint(5));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_RIGHT_FACE_RIGHT", vint(6));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_RIGHT_FACE_DOWN", vint(7));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_RIGHT_FACE_LEFT", vint(8));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_LEFT_TRIGGER_1", vint(9));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_LEFT_TRIGGER_2", vint(10));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_RIGHT_TRIGGER_1", vint(11));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_RIGHT_TRIGGER_2", vint(12));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_MIDDLE_LEFT", vint(13));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_MIDDLE", vint(14));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_MIDDLE_RIGHT", vint(15));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_LEFT_THUMB", vint(16));
    env_set_raw(e, "GamepadButton.GAMEPAD_BUTTON_RIGHT_THUMB", vint(17));
    env_set_raw(e, "GamepadAxis.GAMEPAD_AXIS_LEFT_X", vint(0));
    env_set_raw(e, "GamepadAxis.GAMEPAD_AXIS_LEFT_Y", vint(1));
    env_set_raw(e, "GamepadAxis.GAMEPAD_AXIS_RIGHT_X", vint(2));
    env_set_raw(e, "GamepadAxis.GAMEPAD_AXIS_RIGHT_Y", vint(3));
    env_set_raw(e, "GamepadAxis.GAMEPAD_AXIS_LEFT_TRIGGER", vint(4));
    env_set_raw(e, "GamepadAxis.GAMEPAD_AXIS_RIGHT_TRIGGER", vint(5));
    env_set_raw(e, "MaterialMapIndex.MATERIAL_MAP_ALBEDO", vint(0));
    env_set_raw(e, "MaterialMapIndex.MATERIAL_MAP_METALNESS", vint(1));
    env_set_raw(e, "MaterialMapIndex.MATERIAL_MAP_NORMAL", vint(2));
    env_set_raw(e, "MaterialMapIndex.MATERIAL_MAP_ROUGHNESS", vint(3));
    env_set_raw(e, "MaterialMapIndex.MATERIAL_MAP_OCCLUSION", vint(4));
    env_set_raw(e, "MaterialMapIndex.MATERIAL_MAP_EMISSION", vint(5));
    env_set_raw(e, "MaterialMapIndex.MATERIAL_MAP_HEIGHT", vint(6));
    env_set_raw(e, "MaterialMapIndex.MATERIAL_MAP_CUBEMAP", vint(7));
    env_set_raw(e, "MaterialMapIndex.MATERIAL_MAP_IRRADIANCE", vint(8));
    env_set_raw(e, "MaterialMapIndex.MATERIAL_MAP_PREFILTER", vint(9));
    env_set_raw(e, "MaterialMapIndex.MATERIAL_MAP_BRDF", vint(10));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_VERTEX_POSITION", vint(0));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_VERTEX_TEXCOORD01", vint(1));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_VERTEX_TEXCOORD02", vint(2));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_VERTEX_NORMAL", vint(3));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_VERTEX_TANGENT", vint(4));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_VERTEX_COLOR", vint(5));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MATRIX_MVP", vint(6));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MATRIX_VIEW", vint(7));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MATRIX_PROJECTION", vint(8));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MATRIX_MODEL", vint(9));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MATRIX_NORMAL", vint(10));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_VECTOR_VIEW", vint(11));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_COLOR_DIFFUSE", vint(12));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_COLOR_SPECULAR", vint(13));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_COLOR_AMBIENT", vint(14));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MAP_ALBEDO", vint(15));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MAP_METALNESS", vint(16));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MAP_NORMAL", vint(17));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MAP_ROUGHNESS", vint(18));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MAP_OCCLUSION", vint(19));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MAP_EMISSION", vint(20));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MAP_HEIGHT", vint(21));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MAP_CUBEMAP", vint(22));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MAP_IRRADIANCE", vint(23));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MAP_PREFILTER", vint(24));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_MAP_BRDF", vint(25));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_VERTEX_BONEIDS", vint(26));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_VERTEX_BONEWEIGHTS", vint(27));
    env_set_raw(e, "ShaderLocationIndex.SHADER_LOC_BONE_MATRICES", vint(28));
    env_set_raw(e, "ShaderUniformDataType.SHADER_UNIFORM_FLOAT", vint(0));
    env_set_raw(e, "ShaderUniformDataType.SHADER_UNIFORM_VEC2", vint(1));
    env_set_raw(e, "ShaderUniformDataType.SHADER_UNIFORM_VEC3", vint(2));
    env_set_raw(e, "ShaderUniformDataType.SHADER_UNIFORM_VEC4", vint(3));
    env_set_raw(e, "ShaderUniformDataType.SHADER_UNIFORM_INT", vint(4));
    env_set_raw(e, "ShaderUniformDataType.SHADER_UNIFORM_IVEC2", vint(5));
    env_set_raw(e, "ShaderUniformDataType.SHADER_UNIFORM_IVEC3", vint(6));
    env_set_raw(e, "ShaderUniformDataType.SHADER_UNIFORM_IVEC4", vint(7));
    env_set_raw(e, "ShaderUniformDataType.SHADER_UNIFORM_SAMPLER2D", vint(8));
    env_set_raw(e, "ShaderAttributeDataType.SHADER_ATTRIB_FLOAT", vint(0));
    env_set_raw(e, "ShaderAttributeDataType.SHADER_ATTRIB_VEC2", vint(1));
    env_set_raw(e, "ShaderAttributeDataType.SHADER_ATTRIB_VEC3", vint(2));
    env_set_raw(e, "ShaderAttributeDataType.SHADER_ATTRIB_VEC4", vint(3));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_GRAYSCALE", vint(1));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA", vint(2));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_R5G6B5", vint(3));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_R8G8B8", vint(4));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_R5G5B5A1", vint(5));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_R4G4B4A4", vint(6));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_R8G8B8A8", vint(7));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_R32", vint(8));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_R32G32B32", vint(9));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_R32G32B32A32", vint(10));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_R16", vint(11));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_R16G16B16", vint(12));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_UNCOMPRESSED_R16G16B16A16", vint(13));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_COMPRESSED_DXT1_RGB", vint(14));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_COMPRESSED_DXT1_RGBA", vint(15));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_COMPRESSED_DXT3_RGBA", vint(16));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_COMPRESSED_DXT5_RGBA", vint(17));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_COMPRESSED_ETC1_RGB", vint(18));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_COMPRESSED_ETC2_RGB", vint(19));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA", vint(20));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_COMPRESSED_PVRT_RGB", vint(21));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_COMPRESSED_PVRT_RGBA", vint(22));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA", vint(23));
    env_set_raw(e, "PixelFormat.PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA", vint(24));
    env_set_raw(e, "TextureFilter.TEXTURE_FILTER_POINT", vint(0));
    env_set_raw(e, "TextureFilter.TEXTURE_FILTER_BILINEAR", vint(1));
    env_set_raw(e, "TextureFilter.TEXTURE_FILTER_TRILINEAR", vint(2));
    env_set_raw(e, "TextureFilter.TEXTURE_FILTER_ANISOTROPIC_4X", vint(3));
    env_set_raw(e, "TextureFilter.TEXTURE_FILTER_ANISOTROPIC_8X", vint(4));
    env_set_raw(e, "TextureFilter.TEXTURE_FILTER_ANISOTROPIC_16X", vint(5));
    env_set_raw(e, "TextureWrap.TEXTURE_WRAP_REPEAT", vint(0));
    env_set_raw(e, "TextureWrap.TEXTURE_WRAP_CLAMP", vint(1));
    env_set_raw(e, "TextureWrap.TEXTURE_WRAP_MIRROR_REPEAT", vint(2));
    env_set_raw(e, "TextureWrap.TEXTURE_WRAP_MIRROR_CLAMP", vint(3));
    env_set_raw(e, "CubemapLayout.CUBEMAP_LAYOUT_AUTO_DETECT", vint(0));
    env_set_raw(e, "CubemapLayout.CUBEMAP_LAYOUT_LINE_VERTICAL", vint(1));
    env_set_raw(e, "CubemapLayout.CUBEMAP_LAYOUT_LINE_HORIZONTAL", vint(2));
    env_set_raw(e, "CubemapLayout.CUBEMAP_LAYOUT_CROSS_THREE_BY_FOUR", vint(3));
    env_set_raw(e, "CubemapLayout.CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE", vint(4));
    env_set_raw(e, "FontType.FONT_DEFAULT", vint(0));
    env_set_raw(e, "FontType.FONT_BITMAP", vint(1));
    env_set_raw(e, "FontType.FONT_SDF", vint(2));
    env_set_raw(e, "BlendMode.BLEND_ALPHA", vint(0));
    env_set_raw(e, "BlendMode.BLEND_ADDITIVE", vint(1));
    env_set_raw(e, "BlendMode.BLEND_MULTIPLIED", vint(2));
    env_set_raw(e, "BlendMode.BLEND_ADD_COLORS", vint(3));
    env_set_raw(e, "BlendMode.BLEND_SUBTRACT_COLORS", vint(4));
    env_set_raw(e, "BlendMode.BLEND_ALPHA_PREMULTIPLY", vint(5));
    env_set_raw(e, "BlendMode.BLEND_CUSTOM", vint(6));
    env_set_raw(e, "BlendMode.BLEND_CUSTOM_SEPARATE", vint(7));
    env_set_raw(e, "Gesture.GESTURE_NONE", vint(0));
    env_set_raw(e, "Gesture.GESTURE_TAP", vint(1));
    env_set_raw(e, "Gesture.GESTURE_DOUBLETAP", vint(2));
    env_set_raw(e, "Gesture.GESTURE_HOLD", vint(4));
    env_set_raw(e, "Gesture.GESTURE_DRAG", vint(8));
    env_set_raw(e, "Gesture.GESTURE_SWIPE_RIGHT", vint(16));
    env_set_raw(e, "Gesture.GESTURE_SWIPE_LEFT", vint(32));
    env_set_raw(e, "Gesture.GESTURE_SWIPE_UP", vint(64));
    env_set_raw(e, "Gesture.GESTURE_SWIPE_DOWN", vint(128));
    env_set_raw(e, "Gesture.GESTURE_PINCH_IN", vint(256));
    env_set_raw(e, "Gesture.GESTURE_PINCH_OUT", vint(512));
    env_set_raw(e, "CameraMode.CAMERA_CUSTOM", vint(0));
    env_set_raw(e, "CameraMode.CAMERA_FREE", vint(1));
    env_set_raw(e, "CameraMode.CAMERA_ORBITAL", vint(2));
    env_set_raw(e, "CameraMode.CAMERA_FIRST_PERSON", vint(3));
    env_set_raw(e, "CameraMode.CAMERA_THIRD_PERSON", vint(4));
    env_set_raw(e, "CameraProjection.CAMERA_PERSPECTIVE", vint(0));
    env_set_raw(e, "CameraProjection.CAMERA_ORTHOGRAPHIC", vint(1));
    env_set_raw(e, "NPatchLayout.NPATCH_NINE_PATCH", vint(0));
    env_set_raw(e, "NPatchLayout.NPATCH_THREE_PATCH_VERTICAL", vint(1));
    env_set_raw(e, "NPatchLayout.NPATCH_THREE_PATCH_HORIZONTAL", vint(2));
}
