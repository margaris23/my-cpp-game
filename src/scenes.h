#ifndef SCENES_H
#define SCENES_H

enum class Scene {
  NONE,
  INTRO,
  MENU,
  GAME,
};

extern Scene g_currentScene;

void LoadIntro();
void UpdateIntro(float delta);
void DrawIntro();
void UnloadIntro();

#endif
