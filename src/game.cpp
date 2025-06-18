#include "game.hpp"
#include <random>

// ENTROPY
static std::random_device rd;
static std::mt19937 gen(rd());

Game::Game g_Game;

void Game::InitGame() {
  std::uniform_int_distribution<int> num_of_s_meteors(g_Game.meteors.min_meteors,
                                                      g_Game.meteors.max_meteors);
  g_Game.meteors.count = num_of_s_meteors(gen);
}

void Game::LoadLevel(int level) {
  g_Game.meteors.max_meteors = MAX_METEORS + g_Game.level - 1;
  g_Game.meteors.max_meteor_size = MAX_METEOR_SIZE + g_Game.level - 1;
  // TODO: change min_meteors too

  std::uniform_int_distribution<int> num_of_s_meteors(g_Game.meteors.min_meteors,
                                                      g_Game.meteors.max_meteors);
  g_Game.meteors.count = num_of_s_meteors(gen);

  // TODO: should be calculated per upgrade
  g_Game.fuel_loss_rate = SPACESHIP_INITIAL_FUEL_LOSS_RATE;

  // Reset
  g_Game.level_cores = 0;
  g_Game.health = SPACESHIP_INITIAL_HEALTH;

  // Reset lives
  if (g_Game.lives == 0) {
    g_Game.lives = SPACESHIP_INITIAL_LIVES;
  }
}

void Game::NextLevel() {
  ++g_Game.level;
  LoadLevel(g_Game.level);
}

void Game::ResetSpaceship() { g_Game.health = SPACESHIP_INITIAL_HEALTH; }

void Game::MineMeteor() {
  // Let's earn 1 point for every hit for now....
  ++g_Game.score;
}

void Game::DmgSpaceship(float dmg) {
  g_Game.health -= dmg;
  if (g_Game.health <= 0.f) {
    --g_Game.lives;
  }
}

void Game::GatherCore() {
  ++g_Game.total_cores;
  ++g_Game.level_cores;
}

void Game::LoseFuel() {
  g_Game.fuel -= g_Game.fuel_loss_rate;
  if (g_Game.fuel < 0.f) {
    g_Game.fuel = 0.f;
  }
}

bool Game::IsGameWon() { return g_Game.meteors.count == g_Game.level_cores; }
bool Game::IsGameLost() { return g_Game.lives == 0; }
