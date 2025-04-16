#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static const int screenWidth = 800;
static const int screenHeight = 450;

static void UpdateDrawFrame(void);

int main(void) {
  InitWindow(screenWidth, screenHeight, "My Game");

  InitAudioDevice();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(60);

  // Main game loop
  while (!WindowShouldClose()) {
    UpdateDrawFrame();
  }
#endif

  CloseAudioDevice();
  CloseWindow();

  return 0;
}

// Update and draw game frame
static void UpdateDrawFrame(void) {
  BeginDrawing();

  ClearBackground(RAYWHITE);

  EndDrawing();
}
