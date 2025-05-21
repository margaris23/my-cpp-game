#ifndef GAME_H
#define GAME_H

namespace Game {

constexpr int MIN_METEORS = 3;
constexpr int MAX_METEORS = 5;
constexpr int MIN_METEOR_SIZE = 20;
constexpr int MAX_METEOR_SIZE = 50;
constexpr float METEOR_DMG = 0.1f;
constexpr float METEOR_CORE_SIZE = 10.f;
constexpr float METEOR_CORE_HEALTH = 0.1f;

constexpr float SPACESHIP_INITIAL_HEALTH = 10.f;
constexpr float SPACESHIP_INITIAL_LIVES = 3.f;
constexpr float WEAPON_SIZE = 10.f;
constexpr float WEAPON_DMG = 1.f;
constexpr float WEAPON_MAX_DISTANCE = 60.f;

struct Game {
  int level;

  // Player ???
  int cores;
  int score;
  int max_score;

  // Spaceship ???
  int lives = SPACESHIP_INITIAL_LIVES;
  float health = SPACESHIP_INITIAL_HEALTH;

  // Per Level Meteor Data
  struct {
    int min_meteors = MIN_METEORS;
    int max_meteors = MAX_METEORS;
    int min_meteor_size = MIN_METEOR_SIZE;
    int max_meteor_size = MAX_METEOR_SIZE;
    float meteor_dmg = METEOR_DMG;
    float meteor_core_size = METEOR_CORE_SIZE;
    float meteor_core_health = METEOR_CORE_HEALTH;
  } meteors;

  // Weapon Data, per level ???
  struct {
    float dmg = WEAPON_DMG;
    float size = WEAPON_SIZE;
    float max_distance = WEAPON_MAX_DISTANCE;
  } weapon;
};

void InitGame();
void NextLevel();
void LoseLife();
void MineMeteor();
void DmgSpaceship(float dmg);
void ResetSpaceship();
void GatherCore();

// WIP...
void LoadLevel();
bool IsGameWon();
bool IsGameLost();

} // namespace Game

#endif
