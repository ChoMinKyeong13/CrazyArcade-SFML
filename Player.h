#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
class TileMap;

constexpr int PlayerFrameCount = 4;

struct Player
{
    int playerNumber = 1;
    int characterIndex = -1;

    int row = 1;
    int col = 1;
    int targetRow = 1;
    int targetCol = 1;

    bool moving = false;
    float progress = 0.f;
    float speed = 3.f;

    bool alive = true;
    bool trapped = false;
    float trappedAnimTimer = 0.f;

    static constexpr float TrapDuration = 5.f;

    int bombCapacity = 1;
    int blastRange = 1;
    int needles = 0;

    std::string dir = "down";
    int currentFrame = 0;
    float animTimer = 0.f;
    float animSpeed = 0.12f;

    sf::Keyboard::Scancode keyUp = sf::Keyboard::Scancode::Unknown;
    sf::Keyboard::Scancode keyDown = sf::Keyboard::Scancode::Unknown;
    sf::Keyboard::Scancode keyLeft = sf::Keyboard::Scancode::Unknown;
    sf::Keyboard::Scancode keyRight = sf::Keyboard::Scancode::Unknown;

    sf::Texture texture;
    bool textureLoaded = false;
    float drawSize = 36.f;

    bool loadTexture(const std::string& path, bool useSecondPlayerPalette = false);
    void draw(sf::RenderWindow& window) const;

    bool trapBursting = false;
    float trapBurstTimer = 0.f;
    static constexpr float TrapBurstDuration = 0.35f;

    bool cratePushLocked = false;
};

int dirRow(const std::string& dir);
void applySecondPlayerPalette(sf::Image& image, int characterIndex);
void resetPlayer(Player& player, int row, int col);
void updatePlayer(
    Player& player,
    TileMap& map,
    float deltaTime,
    const std::function<bool(sf::Vector2i)>& isBalloonAt = {},
    const std::function<bool(sf::Vector2i)>& isPlayerAt = {});
sf::Vector2f getRenderPos(const Player& player);
sf::Vector2i getTilePosition(const Player& player);
void killPlayer(Player& player);
bool isAlive(const Player& player);