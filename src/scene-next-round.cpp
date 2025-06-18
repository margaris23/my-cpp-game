#include "ecs.hpp"
#include "fmt/core.h"
#include "game.hpp"
#include "raylib.h"
#include "scenes.hpp"
#include <memory>

static int s_counter;
static SceneEvent s_Event = SceneEvent::NONE;

static std::unique_ptr<ECS::Registry> s_Registry;

using ECS::PositionComponent, ECS::TextComponent, ECS::RenderComponent, ECS::Shape, ECS::Layer,
    ECS::GameStateComponent, ECS::SpriteComponent, ECS::UIComponent, ECS::UIElement, ECS::Entity;

extern Game::Game g_Game;

constexpr float GOLDEN_RATIO = 1.618033989f;
constexpr float SELECTED_CARD_OFFSET = 10.f;
static int selected = 1; // state, 1-indexed

constexpr float cardW = 154.5f;
constexpr float cardH = cardW * GOLDEN_RATIO;
constexpr float BAR_SCALE = 0.6f;

// Costs related
constexpr int EARN_HEALTH_COST = 3; // cores

Entity s_selectionBottom;
Entity s_selectionTop;
Entity s_coresCount;

void LoadNextRound() {
  s_Registry = std::make_unique<ECS::Registry>();

  float centerX = (float)GetScreenWidth() / 2.f;
  float centerY = (float)GetScreenHeight() / 2.f;
  float screenX_1_4 = (float)GetScreenWidth() / 4.f;

  // SHOP
  float posX = screenX_1_4 - cardW / 2.f;
  float posY = centerY - cardH / 2.f;

  // # Cores (UI Entity)
  s_coresCount = s_Registry->CreateEntity();
  s_Registry->Add<GameStateComponent>(s_coresCount, g_Game.total_cores);
  s_Registry->Add<TextComponent>(s_coresCount, fmt::format("{} Cores", g_Game.total_cores),
                                 DARKGREEN);
  s_Registry->Add<PositionComponent>(s_coresCount, 10.f, 10.f);

  // Selected Effect
  s_selectionBottom = s_Registry->CreateEntity();
  s_Registry->Add<RenderComponent>(s_selectionBottom, Layer::GROUND, Shape::RECTANGLE_SOLID, BLACK,
                                   cardW, cardH);
  s_Registry->Add<PositionComponent>(s_selectionBottom, posX + SELECTED_CARD_OFFSET,
                                     posY + SELECTED_CARD_OFFSET);
  s_selectionTop = s_Registry->CreateEntity();
  s_Registry->Add<RenderComponent>(s_selectionTop, Layer::GROUND, Shape::RECTANGLE_SOLID, RAYWHITE,
                                   cardW - 2.f, cardH - 2.f);
  s_Registry->Add<PositionComponent>(s_selectionTop, posX + 1.f, posY + 1.f);

  // CARD - EARN PARTIAL HEALTH
  const Color statusColor = g_Game.total_cores < EARN_HEALTH_COST ? GRAY : BLACK;

  Entity card1 = s_Registry->CreateEntity();
  s_Registry->Add<RenderComponent>(card1, Layer::SKY, Shape::RECTANGLE, statusColor, cardW, cardH);
  s_Registry->Add<PositionComponent>(card1, posX, posY);

  Entity card1Icon = s_Registry->CreateEntity();
  s_Registry->Add<SpriteComponent>(card1Icon, Layer::SKY, "ufo.png");
  s_Registry->Add<PositionComponent>(card1Icon, screenX_1_4 - 30.f, centerY - 15.f);

  Entity healthText = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(healthText, screenX_1_4 - 60.f, centerY - cardH / 2.f + 30.f);
  s_Registry->Add<TextComponent>(healthText, "+10% Health");

  Entity card1Cost = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(card1Cost, screenX_1_4 - 50.f, centerY + cardH / 2.f - 30.f);
  s_Registry->Add<TextComponent>(card1Cost, "$ 3 Cores");

  Entity healthBarInc = s_Registry->CreateEntity();
  float healthInc = 1.1f; // +10%
  s_Registry->Add<GameStateComponent>(healthBarInc, g_Game.health * healthInc * BAR_SCALE);
  s_Registry->Add<UIComponent>(healthBarInc, UIElement::BAR, ORANGE);
  s_Registry->Add<PositionComponent>(healthBarInc, screenX_1_4 - 30.f, centerY + 30.f);

  Entity healthBar = s_Registry->CreateEntity();
  s_Registry->Add<GameStateComponent>(healthBar, g_Game.health * BAR_SCALE);
  s_Registry->Add<UIComponent>(healthBar, UIElement::BAR, DARKGREEN);
  s_Registry->Add<PositionComponent>(healthBar, screenX_1_4 - 30.f, centerY + 30.f);

  // CARD - EARN LIFE
  Entity card2 = s_Registry->CreateEntity();
  s_Registry->Add<RenderComponent>(card2, Layer::SKY, Shape::RECTANGLE, BLACK, cardW, cardH);
  s_Registry->Add<PositionComponent>(card2, posX + screenX_1_4, posY);

  Entity card2Icon = s_Registry->CreateEntity();
  s_Registry->Add<SpriteComponent>(card2Icon, Layer::SKY, "ufo.png");
  s_Registry->Add<PositionComponent>(card2Icon, 2.f * screenX_1_4 - 30.f, centerY - 15.f);

  Entity lifeText = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(lifeText, 2.f * screenX_1_4 - 30.f,
                                     centerY - cardH / 2.f + 30.f);
  s_Registry->Add<TextComponent>(lifeText, "+1 Life");

  Entity card2Cost = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(card2Cost, 2.f * screenX_1_4 - 50.f,
                                     centerY + cardH / 2.f - 30.f);
  s_Registry->Add<TextComponent>(card2Cost, "$ 10 Cores");

  Entity lifeIcon = s_Registry->CreateEntity();
  s_Registry->Add<SpriteComponent>(lifeIcon, Layer::SKY, "life.png");
  s_Registry->Add<PositionComponent>(lifeIcon, 2.f * screenX_1_4 - 50.f, centerY + 25.f);

  Entity lifeIncText = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(lifeIncText, 2.f * screenX_1_4 - 15.f, centerY + 33.f);
  s_Registry->Add<TextComponent>(lifeIncText, fmt::format("{} ->", g_Game.lives), BLACK);
  Entity lifeIncAdded = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(lifeIncAdded, 2.f * screenX_1_4 + 30.f, centerY + 33.f);
  s_Registry->Add<TextComponent>(lifeIncAdded, fmt::format("{}", g_Game.lives + 1), RED);

  // CARD - ???
  Entity card3 = s_Registry->CreateEntity();
  s_Registry->Add<RenderComponent>(card3, Layer::SKY, Shape::RECTANGLE, BLACK, cardW, cardH);
  s_Registry->Add<PositionComponent>(card3, posX + 2.f * screenX_1_4, posY);

  // Messages & Status
  Entity title = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(title, centerX - 30.f, (centerY - cardH / 2.f) / 2.f);
  s_Registry->Add<TextComponent>(title, "SHOP");

  // Entity status = s_Registry->CreateEntity();
  posY = centerY + cardH / 2.f;
  posX = centerX - 60.f;
  // s_Registry->Add<PositionComponent>(status, posX, posY);

  Entity message = s_Registry->CreateEntity();
  s_Registry->Add<PositionComponent>(message, posX - 120.f, posY + 50.f);
  s_Registry->Add<TextComponent>(message, "Press SPACE or ENTER to Buy");

  if (Game::IsGameWon()) {
    // s_Registry->Add<TextComponent>(status, "YOU WIN!");
  } else {
    // s_Registry->Add<TextComponent>(status, "YOU LOSE!");
  }
}

void UpdateNextRound(float delta) {
  if (IsKeyPressed(KEY_LEFT)) {
    --selected;
  } else if (IsKeyPressed(KEY_RIGHT)) {
    ++selected;
  }

  if (selected < 1) {
    selected = 3;
  } else if (selected > 3) {
    selected = 1;
  }

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE) ||
      IsKeyPressed(KEY_ENTER)) {

    if (Game::IsGameWon()) {
      Game::NextLevel();
    } else {
      Game::LoadLevel(g_Game.level);
    }
    s_Event = SceneEvent::NEXT;
    return;
  }

  s_Registry->PositionSystem();
}

void DrawNextRound() {
  float screenX_1_4 = (float)GetScreenWidth() / 4.f;

  auto posBottom = s_Registry->Get<PositionComponent>(s_selectionBottom);
  posBottom->value.x = (float)selected * screenX_1_4 - cardW / 2.f + SELECTED_CARD_OFFSET;
  auto posTop = s_Registry->Get<PositionComponent>(s_selectionTop);
  posTop->value.x = posBottom->value.x - SELECTED_CARD_OFFSET;

  s_Registry->RenderSystem();
  s_Registry->UISystem();
}

void UnloadNextRound() {
  s_Registry.reset();
  s_Event = SceneEvent::NONE;
}

SceneEvent OnNextRoundEvent() { return s_Event; }
