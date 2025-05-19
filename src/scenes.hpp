#ifndef SCENES_H
#define SCENES_H

enum class Scene {
  NONE,
  INTRO,
  MENU,
  GAME,
};

enum class SceneEvent {
  NONE,
  BACK,
  NEXT,
  PAUSE,
  EXIT,
};

extern Scene g_currentScene;

void LoadIntro();
void UpdateIntro(float delta);
void DrawIntro();
void UnloadIntro();
SceneEvent OnIntroEvent();

void LoadMenu();
void UpdateMenu(float delta);
void DrawMenu();
void UnloadMenu();
SceneEvent OnMenuEvent();
bool IsMenuUnloaded();

void LoadGame();
void UpdateGame(float delta);
void DrawGame();
void UnloadGame();
SceneEvent OnGameEvent();
void SetGameFocus(bool focus);

#endif
