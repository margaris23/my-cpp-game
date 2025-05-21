#include "game.hpp"

Game::Game g_Game;

void Game::InitGame() {
  // TBD
}

void Game::NextLevel() {
  ++g_Game.level;

  g_Game.meteors.max_meteors = MAX_METEORS + g_Game.level - 1;
  g_Game.meteors.max_meteor_size = MAX_METEOR_SIZE + g_Game.level - 1;
  g_Game.cores = 0;
  g_Game.health = SPACESHIP_INITIAL_HEALTH;
}

void Game::ResetSpaceship() { g_Game.health = SPACESHIP_INITIAL_HEALTH; }

void Game::MineMeteor() {
  // Let's earn 1 point for every hit for now....
  ++g_Game.score;
}

void Game::DmgSpaceship(float dmg) { g_Game.health -= dmg; }

void Game::LoseLife() { --g_Game.lives; }

void Game::GatherCore() { ++g_Game.cores; }
