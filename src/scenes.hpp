#ifndef SCENES_H
#define SCENES_H
#include <string>
#include <vector>

enum class Scene {
  NONE,
  INTRO,
  GAME,
  NEXT_ROUND,
};

enum class SceneEvent {
  NONE,
  NEXT,
  NEW_GAME,
  SETTINGS,
  HELP,
  PAUSE,
  CONTINUE,
  RESTART,
  EXIT,
};

extern Scene g_currentScene;

void LoadIntro();
void UpdateIntro(float delta);
void DrawIntro();
void UnloadIntro();
SceneEvent OnIntroEvent();
void SetIntroFocus();

using ButtonConfig = std::pair<std::string, SceneEvent>;

void LoadMenu(std::vector<ButtonConfig> &&config);
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

void LoadNextRound();
void UpdateNextRound(float delta);
void DrawNextRound();
void UnloadNextRound();
SceneEvent OnNextRoundEvent();
#endif
