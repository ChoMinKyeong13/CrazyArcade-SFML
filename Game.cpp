#include "Game.h"
#include "WaterBalloon.cpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <cmath>

using namespace sf;
using namespace std;

Game::Game()
    : window(VideoMode({ WINDOW_WIDTH, WINDOW_HEIGHT }), "CrazyArcade"),
    state(GameState::Start),
    waterBalloonSound(waterBalloonSoundBuffer),
    popSound(popSoundBuffer),
    selectedCharacterP1(0),
    selectedCharacterP2(1),
    player1Confirmed(false),
    player2Confirmed(false),
    secondsRemaining(180.f),
    winner(0),
    startBackground(startBackgroundTexture),
    resultBackground(resultBackgroundTexture),
    startText(font),
    startExitText(font),
    selectTitleText(font),
    player1HintText(font),
    player2HintText(font),
    winnerText(font),
    restartText(font),
    exitText(font)
{
    window.setFramerateLimit(60);

    waterBalloonSoundLoaded = waterBalloonSoundBuffer.loadFromFile("waterBalloon.mp3");
    popSoundLoaded = popSoundBuffer.loadFromFile("pop.mp3");
    villageBgmLoaded = villageBgm.openFromFile("vilagebgm.mp3");
    loginBgmLoaded = loginBgm.openFromFile("login.mp3");

    if (!waterBalloonSoundLoaded)
        cout << "waterBalloon.mp3 load failed!" << endl;
    if (!popSoundLoaded)
        cout << "pop.mp3 load failed!" << endl;
    if (!villageBgmLoaded)
        cout << "vilagebgm.mp3 load failed!" << endl;
    if (!loginBgmLoaded)
        cout << "login.mp3 load failed!" << endl;

    waterBalloonSound.setVolume(80.f);
    popSound.setVolume(80.f);
    villageBgm.setVolume(45.f);
    villageBgm.setLooping(true);
    loginBgm.setVolume(45.f);
    loginBgm.setLooping(true);

    if (!font.openFromFile("assets/fonts/neodgm.ttf"))
        cout << "font load failed!" << endl;

    font.setSmooth(false);
    hud.loadFont("assets/fonts/neodgm.ttf");

    for (int index = 0; index < CharacterCount; ++index)
    {
        const string path = "assets/characters/char" + to_string(index + 1) + ".png";

        if (!characterTextures[index].loadFromFile(path))
        {
            cout << path << " load failed!" << endl;
        }
        else
        {
            Image variantImage = characterTextures[index].copyToImage();
            applySecondPlayerPalette(variantImage, index);

            if (!secondPlayerCharacterTextures[index].loadFromImage(variantImage))
                cout << path << " player 2 palette load failed!" << endl;
            else
                secondPlayerCharacterTextures[index].setSmooth(false);
        }
    }

    player1.keyUp = Keyboard::Scancode::W;
    player1.keyDown = Keyboard::Scancode::S;
    player1.keyLeft = Keyboard::Scancode::A;
    player1.keyRight = Keyboard::Scancode::D;

    player2.keyUp = Keyboard::Scancode::Up;
    player2.keyDown = Keyboard::Scancode::Down;
    player2.keyLeft = Keyboard::Scancode::Left;
    player2.keyRight = Keyboard::Scancode::Right;

    selectTitleText.setString(sf::String(U"캐릭터를 선택하세요"));
    selectTitleText.setCharacterSize(30);
    selectTitleText.setFillColor(Color::White);
    centerTextX(selectTitleText, 70.f);

    player1HintText.setString("P1: A / D   SPACE confirm");
    player1HintText.setCharacterSize(18);
    player1HintText.setFillColor(Color(255, 90, 90));
    player1HintText.setPosition({ 75.f, 410.f });

    player2HintText.setString("P2: LEFT / RIGHT   ENTER confirm");
    player2HintText.setCharacterSize(18);
    player2HintText.setFillColor(Color(90, 160, 255));
    player2HintText.setPosition({ 395.f, 410.f });

    float windowWidth = static_cast<float>(WINDOW_WIDTH);
    float windowHeight = static_cast<float>(WINDOW_HEIGHT);

    float mapWidth = MAP_COLS * TILE_SIZE;
    float mapHeight = MAP_ROWS * TILE_SIZE;
    float uiWidth = windowWidth - mapWidth;

    if (!startBackgroundTexture.loadFromFile("assets/images/start_background.png"))
    {
        cout << "start_background.png load failed!" << endl;
    }
    else
    {
        cout << "start_background.png load success!" << endl;
        startBackground.setTexture(startBackgroundTexture, true);

        Vector2u windowSize = window.getSize();
        Vector2u textureSize = startBackgroundTexture.getSize();

        float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(textureSize.x);
        float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(textureSize.y);
        float scale = std::max(scaleX, scaleY);

        startBackground.setScale({ scale, scale });

        float backgroundWidth = static_cast<float>(textureSize.x) * scale;
        float backgroundHeight = static_cast<float>(textureSize.y) * scale;
        float backgroundX = (static_cast<float>(windowSize.x) - backgroundWidth) / 2.f;
        float backgroundY = (static_cast<float>(windowSize.y) - backgroundHeight) / 2.f;

        startBackground.setPosition({ backgroundX, backgroundY });
    }

    if (!resultBackgroundTexture.loadFromFile("assets/images/result_background.png"))
    {
        cout << "result_background.png load failed!" << endl;
    }
    else
    {
        cout << "result_background.png load success!" << endl;
        resultBackground.setTexture(resultBackgroundTexture, true);

        Vector2u windowSize = window.getSize();
        Vector2u textureSize = resultBackgroundTexture.getSize();

        float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(textureSize.x);
        float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(textureSize.y);
        float scale = std::max(scaleX, scaleY);

        resultBackground.setScale({ scale, scale });

        float backgroundWidth = static_cast<float>(textureSize.x) * scale;
        float backgroundHeight = static_cast<float>(textureSize.y) * scale;
        float backgroundX = (static_cast<float>(windowSize.x) - backgroundWidth) / 2.f;
        float backgroundY = (static_cast<float>(windowSize.y) - backgroundHeight) / 2.f;

        resultBackground.setPosition({ backgroundX, backgroundY });
    }

    startButton.setSize({ 240.f, 60.f });
    float startButtonX = (windowWidth - startButton.getSize().x) / 2.f;
    startButton.setPosition({ startButtonX, 390.f });
    startButton.setFillColor(Color(30, 130, 230));
    startButton.setOutlineColor(Color::White);
    startButton.setOutlineThickness(4.f);

    startButtonShadow.setSize(startButton.getSize());
    startButtonShadow.setPosition({ startButtonX, startButton.getPosition().y + 7.f });
    startButtonShadow.setFillColor(Color(0, 70, 150));

    startText.setString(sf::String(U"게임 시작"));
    startText.setCharacterSize(32);
    centerTextOnButton(startText, startButton);

    startExitButton.setSize({ 240.f, 60.f });
    float startExitButtonX = (windowWidth - startExitButton.getSize().x) / 2.f;
    startExitButton.setPosition({ startExitButtonX, 470.f });
    startExitButton.setFillColor(Color(200, 70, 70));
    startExitButton.setOutlineColor(Color::White);
    startExitButton.setOutlineThickness(4.f);

    startExitButtonShadow.setSize(startExitButton.getSize());
    startExitButtonShadow.setPosition({ startExitButtonX, startExitButton.getPosition().y + 7.f });
    startExitButtonShadow.setFillColor(Color(130, 35, 35));

    startExitText.setString(sf::String(U"게임 종료"));
    startExitText.setCharacterSize(32);
    centerTextOnButton(startExitText, startExitButton);

    uiArea.setSize({ uiWidth, windowHeight });
    uiArea.setFillColor(Color(230, 230, 230));
    uiArea.setPosition({ mapWidth, 0.f });

    resultPanel.setSize({ 300.f, 250.f });
    float resultPanelX = (windowWidth - resultPanel.getSize().x) / 2.f;
    resultPanel.setPosition({ resultPanelX, 300.f });
    resultPanel.setFillColor(Color(0, 0, 0, 140));

    winnerText.setString(sf::String(U"승자 없음"));
    winnerText.setCharacterSize(36);
    centerTextX(winnerText, 315.f);

    restartButton.setSize({ 240.f, 70.f });
    float restartButtonX = (windowWidth - restartButton.getSize().x) / 2.f;
    restartButton.setPosition({ restartButtonX, 370.f });
    restartButton.setFillColor(Color(30, 130, 230));
    restartButton.setOutlineColor(Color::White);
    restartButton.setOutlineThickness(4.f);

    restartText.setString(sf::String(U"다시 시작"));
    restartText.setCharacterSize(36);
    centerTextOnButton(restartText, restartButton);

    exitButton.setSize({ 240.f, 70.f });
    float exitButtonX = (windowWidth - exitButton.getSize().x) / 2.f;
    exitButton.setPosition({ exitButtonX, 460.f });
    exitButton.setFillColor(Color(200, 70, 70));
    exitButton.setOutlineColor(Color::White);
    exitButton.setOutlineThickness(4.f);

    exitText.setString(sf::String(U"게임 종료"));
    exitText.setCharacterSize(36);
    centerTextOnButton(exitText, exitButton);

    resetPlayer(player1, 1, 1);
    resetPlayer(player2, TileMap::Rows - 2, TileMap::Columns - 2);
}

Game::~Game() = default;

void Game::run()
{
    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();
        deltaTime = std::min(deltaTime, 0.1f);

        processEvents();
        update(deltaTime);
        updateBackgroundMusic();
        render();
    }
}

void Game::processEvents()
{
    while (const auto event = window.pollEvent())
    {
        if (event->is<Event::Closed>())
            window.close();

        if (const auto* mousePressed = event->getIf<Event::MouseButtonPressed>())
        {
            if (mousePressed->button == Mouse::Button::Left)
            {
                Vector2f mousePosition(mousePressed->position);

                if (state == GameState::Playing && waitingForResultClick)
                {
                    state = GameState::GameOver;
                    continue;
                }

                if (state == GameState::Start)
                {
                    if (startButton.getGlobalBounds().contains(mousePosition))
                        beginCharacterSelect();
                    else if (startExitButton.getGlobalBounds().contains(mousePosition))
                        window.close();
                }
                else if (state == GameState::GameOver)
                {
                    if (restartButton.getGlobalBounds().contains(mousePosition))
                        beginCharacterSelect();
                    else if (exitButton.getGlobalBounds().contains(mousePosition))
                        window.close();
                }
            }
        }

        if (const auto* keyPressed = event->getIf<Event::KeyPressed>())
        {
            if (state == GameState::CharacterSelect)
            {
                if (keyPressed->scancode == Keyboard::Scancode::Escape)
                {
                    state = GameState::Start;
                    continue;
                }

                if (!player1Confirmed)
                {
                    if (keyPressed->scancode == Keyboard::Scancode::A)
                        selectedCharacterP1 = (selectedCharacterP1 - 1 + CharacterCount) % CharacterCount;
                    else if (keyPressed->scancode == Keyboard::Scancode::D)
                        selectedCharacterP1 = (selectedCharacterP1 + 1) % CharacterCount;
                    else if (keyPressed->scancode == Keyboard::Scancode::Space)
                        player1Confirmed = true;
                }

                if (!player2Confirmed)
                {
                    if (keyPressed->scancode == Keyboard::Scancode::Left)
                        selectedCharacterP2 = (selectedCharacterP2 - 1 + CharacterCount) % CharacterCount;
                    else if (keyPressed->scancode == Keyboard::Scancode::Right)
                        selectedCharacterP2 = (selectedCharacterP2 + 1) % CharacterCount;
                    else if (keyPressed->scancode == Keyboard::Scancode::Enter)
                        player2Confirmed = true;
                }

                if (keyPressed->scancode == Keyboard::Scancode::Q)
                    player1Confirmed = false;
                if (keyPressed->scancode == Keyboard::Scancode::RShift)
                    player2Confirmed = false;

                if (player1Confirmed && player2Confirmed)
                    beginPlaying();
            }
            else if (state == GameState::Playing)
            {
                if (keyPressed->scancode == Keyboard::Scancode::Escape)
                {
                    setWinner(0);
                    state = GameState::GameOver;
                }
                else if (keyPressed->scancode == Keyboard::Scancode::Space)
                {
                    placeWaterBalloon(0);
                }
                else if (keyPressed->scancode == Keyboard::Scancode::Enter)
                {
                    placeWaterBalloon(1);
                }
                else if (keyPressed->scancode == Keyboard::Scancode::F)
                {
                    if (tryUseNeedle(itemStats[0], player1.trapped, player1.alive))
                    {
                        trappedSeconds[0] = 0.f;
                        syncPlayerStats(0);
                    }
                }
                else if (keyPressed->scancode == Keyboard::Scancode::RShift)
                {
                    if (tryUseNeedle(itemStats[1], player2.trapped, player2.alive))
                    {
                        trappedSeconds[1] = 0.f;
                        syncPlayerStats(1);
                    }
                }
            }
        }
    }
}

void Game::update(float deltaTime)
{
    if (state == GameState::Start)
    {
        Vector2i mousePixel = Mouse::getPosition(window);
        Vector2f mousePosition = window.mapPixelToCoords(mousePixel);

        if (startButton.getGlobalBounds().contains(mousePosition))
            startButton.setFillColor(Color(70, 170, 255));
        else
            startButton.setFillColor(Color(30, 130, 230));
    }
    else if (state == GameState::CharacterSelect)
    {
        // Character selection is driven by key press events.
    }
    else if (state == GameState::Playing)
    {
        if (waitingForResultClick)
            return;

        bushShakeTimer += deltaTime;

        updatePlayer(player1, tileMap, deltaTime,
            [&](sf::Vector2i tile)
            {
                bool blocked = false;
                if (player1Balloons) blocked = blocked || player1Balloons->hasBalloonAt(tile);
                if (player2Balloons) blocked = blocked || player2Balloons->hasBalloonAt(tile);
                return blocked;
            },
            [&](sf::Vector2i tile)
            {
                if (player2.col == tile.x && player2.row == tile.y)
                    return true;

                if (player2.moving &&
                    player2.targetCol == tile.x &&
                    player2.targetRow == tile.y)
                    return true;

                return false;
            });

        updatePlayer(player2, tileMap, deltaTime,
            [&](sf::Vector2i tile)
            {
                bool blocked = false;
                if (player1Balloons) blocked = blocked || player1Balloons->hasBalloonAt(tile);
                if (player2Balloons) blocked = blocked || player2Balloons->hasBalloonAt(tile);
                return blocked;
            },
            [&](sf::Vector2i tile)
            {
                if (player1.col == tile.x && player1.row == tile.y)
                    return true;

                if (player1.moving &&
                    player1.targetCol == tile.x &&
                    player1.targetRow == tile.y)
                    return true;

                return false;
            });

        sf::Vector2f p1Pos = getRenderPos(player1);
        sf::Vector2f p2Pos = getRenderPos(player2);

        float dx = p1Pos.x - p2Pos.x;
        float dy = p1Pos.y - p2Pos.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        const float touchDistance = 25.f;

        if (player1.alive && player2.alive && distance <= touchDistance)
        {
            if (player1.trapped && !player2.trapped && !player1.trapBursting)
            {
                player1.trapped = false;
                player1.trapBursting = true;
                player1.trapBurstTimer = 0.f;
                trappedSeconds[0] = 0.f;
            }
            else if (player2.trapped && !player1.trapped && !player2.trapBursting)
            {
                player2.trapped = false;
                player2.trapBursting = true;
                player2.trapBurstTimer = 0.f;
                trappedSeconds[1] = 0.f;
            }
        }

        if (player1.trapBursting)
        {
            player1.trapBurstTimer += deltaTime;

            if (player1.trapBurstTimer >= Player::TrapBurstDuration)
            {
                player1.trapBursting = false;

                if (popSoundLoaded)
                    popSound.play();

                killPlayer(player1);
            }
        }

        if (player2.trapBursting)
        {
            player2.trapBurstTimer += deltaTime;

            if (player2.trapBurstTimer >= Player::TrapBurstDuration)
            {
                player2.trapBursting = false;

                if (popSoundLoaded)
                    popSound.play();

                killPlayer(player2);
            }
        }

        updateWaterBalloons(deltaTime);
        updateItems();

        secondsRemaining = std::max(0.f, secondsRemaining - deltaTime);

        Player* players[] = { &player1, &player2 };

        for (int index = 0; index < 2; ++index)
        {
            if (players[index]->alive && players[index]->trapped)
            {
                trappedSeconds[index] += deltaTime;

                if (trappedSeconds[index] >= Player::TrapDuration && !players[index]->trapBursting)
                {
                    players[index]->trapped = false;
                    players[index]->trapBursting = true;
                    players[index]->trapBurstTimer = 0.f;
                    trappedSeconds[index] = 0.f;
                }
            }
        }

        checkGameResult();
    }
    else if (state == GameState::GameOver)
    {
        Vector2i mousePixel = Mouse::getPosition(window);
        Vector2f mousePosition = window.mapPixelToCoords(mousePixel);

        if (restartButton.getGlobalBounds().contains(mousePosition))
            restartButton.setFillColor(Color(70, 170, 255));
        else
            restartButton.setFillColor(Color(30, 130, 230));

        if (exitButton.getGlobalBounds().contains(mousePosition))
            exitButton.setFillColor(Color(235, 90, 90));
        else
            exitButton.setFillColor(Color(200, 70, 70));
    }
}

void Game::render()
{
    if (state == GameState::Start)
    {
        window.clear();
        window.draw(startBackground);

        window.draw(startButtonShadow);
        window.draw(startButton);
        window.draw(startText);

        window.draw(startExitButtonShadow);
        window.draw(startExitButton);
        window.draw(startExitText);
    }
    else if (state == GameState::CharacterSelect)
    {
        renderCharacterSelect();
    }
    else if (state == GameState::Playing)
    {
        window.clear();

        tileMap.draw(window);
        window.draw(uiArea);

        itemManager.draw(window);

        if (player1Balloons)
            player1Balloons->drawBalloons(window);
        if (player2Balloons)
            player2Balloons->drawBalloons(window);

        sf::Vector2i p1Tile = getTilePosition(player1);
        sf::Vector2i p2Tile = getTilePosition(player2);

        bool p1InBush = tileMap.getTile(p1Tile.x, p1Tile.y) == TileType::HideableBush;
        bool p2InBush = tileMap.getTile(p2Tile.x, p2Tile.y) == TileType::HideableBush;

        if (!p1InBush)
            player1.draw(window);
        if (!p2InBush)
            player2.draw(window);

        float shake = std::sin(bushShakeTimer * 14.f) * 3.f;

        if (p1InBush)
            tileMap.drawBushShake(window, p1Tile.x, p1Tile.y, shake);
        if (p2InBush)
            tileMap.drawBushShake(window, p2Tile.x, p2Tile.y, -shake);

        if (player1Balloons)
            player1Balloons->drawExplosions(window);
        if (player2Balloons)
            player2Balloons->drawExplosions(window);

        const array<PlayerHudData, 2> players = { {
            { itemStats[0], player1.alive, player1.trapped },
            { itemStats[1], player2.alive, player2.trapped }
        } };


        hud.draw(window, players, secondsRemaining, "P1 SPACE / F\nP2 ENTER / RSHIFT");

        if (waitingForResultClick)
        {
            RectangleShape dark({
                static_cast<float>(WINDOW_WIDTH),
                static_cast<float>(WINDOW_HEIGHT)
                });

            dark.setFillColor(Color(0, 0, 0, 110));
            window.draw(dark);

            Text loseText(
                font,
                loser == 1 ? "P1 LOSE" : loser == 2 ? "P2 LOSE" : "DRAW",
                64);

            loseText.setStyle(Text::Bold);
            loseText.setFillColor(Color(255, 80, 80));

            FloatRect bounds = loseText.getLocalBounds();

            loseText.setPosition({
                TileMap::OriginX + TileMap::PixelWidth / 2.f - bounds.position.x - bounds.size.x / 2.f,
                TileMap::OriginY + TileMap::PixelHeight / 2.f - bounds.position.y - bounds.size.y / 2.f - 20.f
                });

            window.draw(loseText);

            Text clickText(font, sf::String(U"마우스를 클릭하세요"), 20);
            clickText.setFillColor(Color::White);

            bounds = clickText.getLocalBounds();

            clickText.setPosition({
                TileMap::OriginX + TileMap::PixelWidth / 2.f - bounds.position.x - bounds.size.x / 2.f,
                TileMap::OriginY + TileMap::PixelHeight / 2.f + 65.f
                });

            window.draw(clickText);
        }
    }
    else if (state == GameState::GameOver)
    {
        window.clear();
        window.draw(resultBackground);
        window.draw(resultPanel);
        window.draw(winnerText);
        window.draw(restartButton);
        window.draw(restartText);
        window.draw(exitButton);
        window.draw(exitText);
    }

    window.display();
}

void Game::resetGame()
{
    winner = 0;

    waitingForResultClick = false;
    loser = 0;

    tileMap.reset();
    resetPlayer(player1, 1, 0);
    resetPlayer(player2, 12, 14);

    itemManager.clear();
    itemStats = {};
    trappedSeconds = {};
    secondsRemaining = 180.f;

    createWaterBalloonSystems();
    syncPlayerStats(0);
    syncPlayerStats(1);

    state = GameState::Playing;

    waitingForGameOver = false;
    gameOverDelayTimer = 0.f;
    pendingWinner = 0;
}

void Game::updateBackgroundMusic()
{
    const bool showMap = window.isOpen() && state == GameState::Playing;

    if (showMap)
    {
        if (loginBgm.getStatus() != SoundSource::Status::Stopped)
            loginBgm.stop();

        if (villageBgmLoaded && villageBgm.getStatus() != SoundSource::Status::Playing)
            villageBgm.play();
    }
    else
    {
        if (villageBgm.getStatus() != SoundSource::Status::Stopped)
            villageBgm.stop();

        if (window.isOpen() && loginBgmLoaded && loginBgm.getStatus() != SoundSource::Status::Playing)
            loginBgm.play();
        else if (!window.isOpen())
            loginBgm.stop();
    }
}

void Game::beginCharacterSelect()
{
    selectedCharacterP1 = 0;
    selectedCharacterP2 = 1;
    player1Confirmed = false;
    player2Confirmed = false;
    state = GameState::CharacterSelect;
}

void Game::beginPlaying()
{
    const string player1Path =
        "assets/characters/char" + to_string(selectedCharacterP1 + 1) + ".png";

    const string player2Path =
        "assets/characters/char" + to_string(selectedCharacterP2 + 1) + ".png";

    player1.playerNumber = 1;
    player2.playerNumber = 2;

    player1.characterIndex = selectedCharacterP1;
    player2.characterIndex = selectedCharacterP2;

    player1.loadTexture(player1Path);
    player2.loadTexture(player2Path, selectedCharacterP1 == selectedCharacterP2);

    resetGame();
}

void Game::createWaterBalloonSystems()
{
    auto createSystem = []()
        {
            return make_unique<WaterBalloonSystem>(
                "assets/water_balloon_animation/water_balloon_sheet_10fps.png",
                "assets/water_balloon_animation/water_center.png",
                "assets/water_balloon_animation/water_body.png",
                "assets/water_balloon_animation/water_end.png",
                static_cast<float>(TileMap::TileSize),
                TileMap::Columns,
                TileMap::Rows,
                Vector2f(
                    static_cast<float>(TileMap::OriginX),
                    static_cast<float>(TileMap::OriginY)));
        };

    player1Balloons = createSystem();
    player2Balloons = createSystem();
}

void Game::placeWaterBalloon(int playerIndex)
{
    Player& player = playerIndex == 0 ? player1 : player2;

    WaterBalloonSystem* system =
        playerIndex == 0 ? player1Balloons.get() : player2Balloons.get();

    if (!system || !player.alive || player.trapped)
        return;

    const Vector2i tile = getTilePosition(player);

    if ((player1Balloons && player1Balloons->hasBalloonAt(tile)) ||
        (player2Balloons && player2Balloons->hasBalloonAt(tile)))
        return;

    system->place(tile);
}

void Game::updateWaterBalloons(float deltaTime)
{
    int explodedBalloonCount = 0;

    auto isBlocked = [this](Vector2i tile)
        {
            return tileMap.getTile(tile.x, tile.y) != TileType::Empty;
        };

    auto handleExplosionTile = [this](Vector2i tile)
        {
            if (!tileMap.destroyBlock(tile.x, tile.y))
                return;

            const Vector2f center = {
                static_cast<float>(TileMap::OriginX) +
                    (static_cast<float>(tile.x) + 0.5f) * TileMap::TileSize,
                static_cast<float>(TileMap::OriginY) +
                    (static_cast<float>(tile.y) + 0.5f) * TileMap::TileSize
            };

            itemManager.trySpawn(center, 0.3f, 28.f);
        };

    if (player1Balloons)
        explodedBalloonCount +=
        player1Balloons->update(deltaTime, isBlocked, handleExplosionTile);

    if (player2Balloons)
        explodedBalloonCount +=
        player2Balloons->update(deltaTime, isBlocked, handleExplosionTile);

    constexpr int MaxChainReactionSteps = 32;

    for (int step = 0; step < MaxChainReactionSteps; ++step)
    {
        bool triggered = false;

        if (player1Balloons)
        {
            triggered =
                player1Balloons->triggerBalloonsHitBy(*player1Balloons) ||
                triggered;

            if (player2Balloons)
            {
                triggered =
                    player1Balloons->triggerBalloonsHitBy(*player2Balloons) ||
                    triggered;
            }
        }

        if (player2Balloons)
        {
            if (player1Balloons)
            {
                triggered =
                    player2Balloons->triggerBalloonsHitBy(*player1Balloons) ||
                    triggered;
            }

            triggered =
                player2Balloons->triggerBalloonsHitBy(*player2Balloons) ||
                triggered;
        }

        if (!triggered)
            break;

        if (player1Balloons)
            explodedBalloonCount +=
            player1Balloons->update(0.f, isBlocked, handleExplosionTile);

        if (player2Balloons)
            explodedBalloonCount +=
            player2Balloons->update(0.f, isBlocked, handleExplosionTile);
    }

    if (explodedBalloonCount > 0 && waterBalloonSoundLoaded)
        waterBalloonSound.play();

    Player* players[] = { &player1, &player2 };

    for (int index = 0; index < 2; ++index)
    {
        Player& player = *players[index];

        if (!player.alive || player.trapped)
            continue;

        const Vector2f position = getRenderPos(player);

        const bool hitByPlayer1 =
            player1Balloons && player1Balloons->hitsPoint(position);

        const bool hitByPlayer2 =
            player2Balloons && player2Balloons->hitsPoint(position);

        if (hitByPlayer1 || hitByPlayer2)
        {
            player.trapped = true;
            player.moving = false;
            trappedSeconds[index] = 0.f;
        }
    }
}

void Game::updateItems()
{
    Player* players[] = { &player1, &player2 };
    WaterBalloonSystem* systems[] = {
        player1Balloons.get(),
        player2Balloons.get()
    };

    for (int index = 0; index < 2; ++index)
    {
        Player& player = *players[index];

        const optional<ItemType> collected =
            itemManager.collectAt(
                getPlayerBounds(player),
                player.alive && !player.trapped);

        if (!collected)
            continue;

        const ItemStats before = itemStats[index];

        applyItemEffect(itemStats[index], *collected);

        if (systems[index])
        {
            if (itemStats[index].bombCapacity > before.bombCapacity)
                systems[index]->increaseBalloonCapacity();

            if (itemStats[index].blastRange > before.blastRange)
                systems[index]->increaseExplosionRange();
        }

        syncPlayerStats(index);
    }
}

void Game::syncPlayerStats(int playerIndex)
{
    Player& player = playerIndex == 0 ? player1 : player2;
    const ItemStats& stats = itemStats[playerIndex];

    player.bombCapacity = stats.bombCapacity;
    player.blastRange = stats.blastRange;
    player.needles = stats.needles;
    player.speed = stats.speed / static_cast<float>(TileMap::TileSize);
}

void Game::checkGameResult()
{
    if (state != GameState::Playing || waitingForResultClick)
        return;

    if (!player1.alive && !player2.alive)
    {
        setWinner(0);
        loser = 0;
        waitingForResultClick = true;
    }
    else if (!player1.alive)
    {
        setWinner(2);
        loser = 1;
        waitingForResultClick = true;
    }
    else if (!player2.alive)
    {
        setWinner(1);
        loser = 2;
        waitingForResultClick = true;
    }
    else if (secondsRemaining <= 0.f)
    {
        setWinner(0);
        loser = 0;
        waitingForResultClick = true;
    }
}

FloatRect Game::getPlayerBounds(const Player& player) const
{
    const Vector2f center = getRenderPos(player);

    return FloatRect(
        { center.x - 14.f, center.y - 14.f },
        { 28.f, 28.f });
}

void Game::renderCharacterSelect()
{
    window.clear(Color(19, 21, 38));

    RectangleShape topBar({ 800.f, 7.f });
    topBar.setFillColor(Color(86, 112, 255));
    window.draw(topBar);

    Text logo(font, "CRAZY ARCADE", 34);
    logo.setStyle(Text::Bold);
    logo.setFillColor(Color(245, 247, 255));
    centerTextX(logo, 42.f);
    window.draw(logo);

    Text subtitle(
        font,
        sf::String(U"플레이할 캐릭터를 선택하세요"),
        18);

    subtitle.setFillColor(Color(165, 171, 200));
    centerTextX(subtitle, 91.f);
    window.draw(subtitle);

    RectangleShape characterPanel({ 700.f, 184.f });
    characterPanel.setPosition({ 50.f, 186.f });
    characterPanel.setFillColor(Color(28, 31, 52));
    characterPanel.setOutlineThickness(1.f);
    characterPanel.setOutlineColor(Color(60, 65, 95));
    window.draw(characterPanel);

    auto drawMarker =
        [this](const string& label, float x, float y, const Color& color)
        {
            RectangleShape marker({ 38.f, 22.f });
            marker.setOrigin({ 19.f, 11.f });
            marker.setPosition({ x, y });
            marker.setFillColor(color);
            window.draw(marker);

            Text markerText(font, label, 13);
            markerText.setStyle(Text::Bold);
            markerText.setFillColor(Color::White);

            const FloatRect bounds = markerText.getLocalBounds();

            markerText.setPosition({
                x - bounds.position.x - bounds.size.x / 2.f,
                y - bounds.position.y - bounds.size.y / 2.f - 1.f
                });

            window.draw(markerText);
        };

    constexpr float cardStartX = 135.f;
    constexpr float cardGap = 106.f;
    constexpr float cardY = 285.f;

    for (int index = 0; index < CharacterCount; ++index)
    {
        const bool isPlayer1 = index == selectedCharacterP1;
        const bool isPlayer2 = index == selectedCharacterP2;

        const float cardWidth =
            (isPlayer1 || isPlayer2) ? 94.f : 86.f;

        const float cardHeight =
            (isPlayer1 || isPlayer2) ? 124.f : 116.f;

        const float centerX =
            cardStartX + static_cast<float>(index) * cardGap;

        RectangleShape shadow({ cardWidth, cardHeight });
        shadow.setOrigin({ cardWidth / 2.f, cardHeight / 2.f });
        shadow.setPosition({ centerX + 4.f, cardY + 6.f });
        shadow.setFillColor(Color(8, 9, 20, 150));
        window.draw(shadow);

        RectangleShape cardBody({ cardWidth, cardHeight });
        cardBody.setOrigin({ cardWidth / 2.f, cardHeight / 2.f });
        cardBody.setPosition({ centerX, cardY });
        cardBody.setFillColor(Color(42, 45, 69));
        cardBody.setOutlineThickness(2.f);
        cardBody.setOutlineColor(Color(75, 80, 110));

        if (isPlayer1 && isPlayer2)
        {
            cardBody.setFillColor(Color(60, 42, 75));
            cardBody.setOutlineThickness(5.f);
            cardBody.setOutlineColor(Color(205, 100, 255));
        }
        else if (isPlayer1)
        {
            cardBody.setFillColor(Color(67, 38, 51));
            cardBody.setOutlineThickness(4.f);
            cardBody.setOutlineColor(
                player1Confirmed
                ? Color(90, 230, 130)
                : Color(255, 75, 90));
        }
        else if (isPlayer2)
        {
            cardBody.setFillColor(Color(35, 49, 76));
            cardBody.setOutlineThickness(4.f);
            cardBody.setOutlineColor(
                player2Confirmed
                ? Color(90, 230, 130)
                : Color(70, 150, 255));
        }

        window.draw(cardBody);

        const bool useSecondPlayerPalette = isPlayer1 && isPlayer2;

        const Texture& previewTexture =
            useSecondPlayerPalette
            ? secondPlayerCharacterTextures[index]
            : characterTextures[index];

        const Vector2u textureSize = previewTexture.getSize();
        const int frameWidth = static_cast<int>(textureSize.x) / 4;
        const int frameHeight = static_cast<int>(textureSize.y) / 4;

        if (frameWidth <= 0 || frameHeight <= 0)
            continue;

        Sprite character(previewTexture);

        character.setTextureRect(
            IntRect({ 0, 0 }, { frameWidth, frameHeight }));

        character.setOrigin({
            frameWidth / 2.f,
            frameHeight / 2.f
            });

        const float imageSize =
            (isPlayer1 || isPlayer2) ? 69.f : 62.f;

        const float scale =
            imageSize / static_cast<float>(frameHeight);

        character.setScale({ scale, scale });

        float previewOffsetX = 0.f;

        if (index == 0)
            previewOffsetX = 54.5f;
        else if (index == 5)
            previewOffsetX = 43.5f;

        character.setPosition({
            centerX - previewOffsetX * scale,
            cardY - 12.f
            });

        window.draw(character);

        Color platformTop(70, 74, 100);
        Color platformFront(34, 37, 60);

        if (isPlayer1 && isPlayer2)
        {
            platformTop = Color(174, 91, 211);
            platformFront = Color(91, 48, 116);
        }
        else if (isPlayer1)
        {
            platformTop = Color(213, 79, 92);
            platformFront = Color(112, 43, 54);
        }
        else if (isPlayer2)
        {
            platformTop = Color(70, 139, 226);
            platformFront = Color(38, 72, 124);
        }

        constexpr float platformTopY = cardY + 27.f;

        RectangleShape platformShadow({ 64.f, 21.f });
        platformShadow.setOrigin({ 32.f, 0.f });
        platformShadow.setPosition({
            centerX + 3.f,
            platformTopY + 8.f
            });
        platformShadow.setFillColor(Color(5, 7, 15, 170));
        window.draw(platformShadow);

        ConvexShape platformSurface(4);
        platformSurface.setPoint(0, { 7.f, 0.f });
        platformSurface.setPoint(1, { 61.f, 0.f });
        platformSurface.setPoint(2, { 68.f, 9.f });
        platformSurface.setPoint(3, { 0.f, 9.f });
        platformSurface.setOrigin({ 34.f, 0.f });
        platformSurface.setPosition({ centerX, platformTopY });
        platformSurface.setFillColor(platformTop);
        window.draw(platformSurface);

        RectangleShape platformFrontShape({ 68.f, 17.f });
        platformFrontShape.setOrigin({ 34.f, 0.f });
        platformFrontShape.setPosition({
            centerX,
            platformTopY + 9.f
            });
        platformFrontShape.setFillColor(platformFront);
        platformFrontShape.setOutlineColor(Color(12, 15, 28));
        platformFrontShape.setOutlineThickness(1.f);
        window.draw(platformFrontShape);

        RectangleShape platformHighlight({ 64.f, 2.f });
        platformHighlight.setOrigin({ 32.f, 0.f });
        platformHighlight.setPosition({
            centerX,
            platformTopY + 10.f
            });
        platformHighlight.setFillColor(Color(255, 255, 255, 55));
        window.draw(platformHighlight);

        Text characterNumber(font, to_string(index + 1), 12);
        characterNumber.setStyle(Text::Bold);
        characterNumber.setFillColor(Color(235, 238, 250));

        const FloatRect numberBounds =
            characterNumber.getLocalBounds();

        characterNumber.setPosition({
            centerX - numberBounds.position.x -
                numberBounds.size.x / 2.f,
            platformTopY + 11.f - numberBounds.position.y
            });

        window.draw(characterNumber);

        if (isPlayer1)
        {
            drawMarker(
                player1Confirmed ? "P1 OK" : "P1",
                centerX - 20.f,
                205.f,
                player1Confirmed
                ? Color(55, 190, 105)
                : Color(230, 65, 80));
        }

        if (isPlayer2)
        {
            drawMarker(
                player2Confirmed ? "P2 OK" : "P2",
                centerX + 20.f,
                205.f,
                player2Confirmed
                ? Color(55, 190, 105)
                : Color(60, 125, 235));
        }
    }

    auto drawPlayerPanel =
        [this](bool firstPlayer, bool confirmed)
        {
            const float x = firstPlayer ? 55.f : 420.f;

            const Color normalColor =
                firstPlayer
                ? Color(220, 60, 75)
                : Color(65, 135, 245);

            const Color readyColor(65, 210, 115);

            RectangleShape panel({ 325.f, 115.f });
            panel.setPosition({ x, 410.f });

            panel.setFillColor(
                firstPlayer
                ? Color(48, 29, 40)
                : Color(27, 38, 61));

            panel.setOutlineThickness(2.f);
            panel.setOutlineColor(
                confirmed ? readyColor : normalColor);

            window.draw(panel);

            Text title(
                font,
                confirmed
                ? (firstPlayer ? "P1  READY!" : "P2  READY!")
                : (firstPlayer ? "PLAYER 1" : "PLAYER 2"),
                19);

            title.setStyle(Text::Bold);

            title.setFillColor(
                confirmed
                ? Color(90, 235, 135)
                : (firstPlayer
                    ? Color(255, 90, 100)
                    : Color(80, 155, 255)));

            title.setPosition({ x + 22.f, 427.f });
            window.draw(title);

            const sf::String selectMessage =
                firstPlayer
                ? sf::String(U"A / D  캐릭터 선택")
                : sf::String(U"LEFT / RIGHT  캐릭터 선택");

            Text controls(font, selectMessage, 15);
            controls.setFillColor(Color(220, 220, 232));
            controls.setPosition({ x + 22.f, 460.f });
            window.draw(controls);

            const sf::String confirmMessage =
                firstPlayer
                ? sf::String(U"SPACE  선택 완료   /   Q  취소")
                : sf::String(U"ENTER  선택 완료   /   RSHIFT  취소");

            Text confirmText(font, confirmMessage, 14);
            confirmText.setFillColor(Color(165, 167, 190));
            confirmText.setPosition({ x + 22.f, 487.f });
            window.draw(confirmText);
        };

    drawPlayerPanel(true, player1Confirmed);
    drawPlayerPanel(false, player2Confirmed);

    Text bottomText(
        font,
        sf::String(U"두 플레이어가 모두 선택하면 게임이 시작됩니다"),
        13);

    bottomText.setFillColor(Color(115, 120, 150));
    centerTextX(bottomText, 557.f);
    window.draw(bottomText);
}

void Game::centerTextOnButton(
    Text& text,
    const RectangleShape& button)
{
    FloatRect textBounds = text.getLocalBounds();
    FloatRect buttonBounds = button.getGlobalBounds();

    text.setPosition({
        buttonBounds.position.x +
            (buttonBounds.size.x - textBounds.size.x) / 2.f -
            textBounds.position.x,
        buttonBounds.position.y +
            (buttonBounds.size.y - textBounds.size.y) / 2.f -
            textBounds.position.y
        });
}

void Game::centerTextX(Text& text, float y)
{
    FloatRect textBounds = text.getLocalBounds();

    float windowWidth =
        static_cast<float>(window.getSize().x);

    float x =
        (windowWidth - textBounds.size.x) / 2.f -
        textBounds.position.x;

    text.setPosition({ x, y });
}

void Game::setWinner(int playerNumber)
{
    winner = playerNumber;

    if (winner == 1)
        winnerText.setString(sf::String(U"1P 승리!"));
    else if (winner == 2)
        winnerText.setString(sf::String(U"2P 승리!"));
    else
        winnerText.setString(sf::String(U"승자 없음"));

    centerTextX(winnerText, 315.f);
}