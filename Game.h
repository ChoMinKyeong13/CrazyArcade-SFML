#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <array>
#include <memory>
#include "Item.h"
#include "Player.h"
#include "TileMap.h"
#include "UI.h"

class WaterBalloonSystem;

enum class GameState
{
    Start,
    CharacterSelect,
    Playing,
    GameOver
};

class Game
{
private:
    // 게임 화면 크기
    static constexpr unsigned int WINDOW_WIDTH = 800;
    static constexpr unsigned int WINDOW_HEIGHT = 600;

    // 맵 설정
    static constexpr int MAP_COLS = 15;
    static constexpr int MAP_ROWS = 13;
    static constexpr float TILE_SIZE = 40.f;

    sf::RenderWindow window;
    GameState state;
    sf::Clock clock;

    // 게임 음향
    sf::SoundBuffer waterBalloonSoundBuffer;
    sf::SoundBuffer popSoundBuffer;
    sf::Sound waterBalloonSound;
    sf::Sound popSound;
    sf::Music villageBgm;
    sf::Music loginBgm;
    bool waterBalloonSoundLoaded = false;
    bool popSoundLoaded = false;
    bool villageBgmLoaded = false;
    bool loginBgmLoaded = false;

    sf::Font font;

    // 시작 화면
    sf::Texture startBackgroundTexture;
    sf::Sprite startBackground;

    sf::RectangleShape startButtonShadow;
    sf::RectangleShape startButton;
    sf::Text startText;

    sf::RectangleShape startExitButtonShadow;
    sf::RectangleShape startExitButton;
    sf::Text startExitText;

    // 캐릭터 선택 화면
    static constexpr int CharacterCount = 6;
    std::array<sf::Texture, CharacterCount> characterTextures;
    std::array<sf::Texture, CharacterCount> secondPlayerCharacterTextures;
    int selectedCharacterP1;
    int selectedCharacterP2;
    bool player1Confirmed;
    bool player2Confirmed;

    sf::Text selectTitleText;
    sf::Text player1HintText;
    sf::Text player2HintText;

    // 게임 진행 화면
    TileMap tileMap;
    sf::RectangleShape uiArea;
    Player player1;
    Player player2;
    std::unique_ptr<WaterBalloonSystem> player1Balloons;
    std::unique_ptr<WaterBalloonSystem> player2Balloons;

    ItemManager itemManager;
    std::array<ItemStats, 2> itemStats;
    std::array<float, 2> trappedSeconds{};
    UI hud;
    float secondsRemaining;

    // 승자 정보
    int winner;

    // 게임 종료 화면 배경
    sf::Texture resultBackgroundTexture;
    sf::Sprite resultBackground;

    // 결과 화면 반투명 패널
    sf::RectangleShape resultPanel;

    sf::Text winnerText;

    sf::RectangleShape restartButton;
    sf::Text restartText;

    sf::RectangleShape exitButton;
    sf::Text exitText;

    void processEvents();
    void update(float deltaTime);
    void updateBackgroundMusic();
    void render();
    void resetGame();
    void setWinner(int playerNumber);
    void beginCharacterSelect();
    void beginPlaying();
    void renderCharacterSelect();
    void createWaterBalloonSystems();
    void placeWaterBalloon(int playerIndex);
    void updateWaterBalloons(float deltaTime);
    void updateItems();
    void syncPlayerStats(int playerIndex);
    void checkGameResult();
    [[nodiscard]] sf::FloatRect getPlayerBounds(const Player& player) const;

    void centerTextOnButton(
        sf::Text& text,
        const sf::RectangleShape& button);

    void centerTextX(
        sf::Text& text,
        float y);

    // 결과화면 지연용
    bool waitingForGameOver = false;
    float gameOverDelayTimer = 0.f;
    int pendingWinner = 0;
    static constexpr float GameOverDelay = 0.8f;

    bool waitingForResultClick = false;
    int loser = 0;

    float bushShakeTimer = 0.f;

public:
    Game();
    ~Game();
    void run();
};