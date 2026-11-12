#include "Player.h"
#include "TileMap.h"
#include <cmath>
#include <string>
#include <unordered_map>
#include <memory>
#include <algorithm>

static std::unordered_map<const Player*, std::unique_ptr<sf::Texture>> trappedTextures;

static std::string makeTrappedPath(const std::string& path)
{
    std::string result = path;
    size_t pos = result.rfind(".png");
    if (pos != std::string::npos) result.insert(pos, "_trapped");
    return result;
}

int dirRow(const std::string& dir)
{
    if (dir == "down") return 0;
    if (dir == "up") return 1;
    if (dir == "left") return 2;
    if (dir == "right") return 3;
    return 0;
}

namespace
{
    struct HsvColor
    {
        float hue;
        float saturation;
        float value;
    };

    HsvColor rgbToHsv(const sf::Color& color)
    {
        const float red = color.r / 255.f;
        const float green = color.g / 255.f;
        const float blue = color.b / 255.f;
        const float maximum = std::max({ red, green, blue });
        const float minimum = std::min({ red, green, blue });
        const float difference = maximum - minimum;

        float hue = 0.f;

        if (difference > 0.0001f)
        {
            if (maximum == red)
                hue = 60.f * std::fmod((green - blue) / difference, 6.f);
            else if (maximum == green)
                hue = 60.f * (((blue - red) / difference) + 2.f);
            else
                hue = 60.f * (((red - green) / difference) + 4.f);
        }

        if (hue < 0.f)
            hue += 360.f;

        return { hue, maximum <= 0.f ? 0.f : difference / maximum, maximum };
    }

    sf::Color hsvToRgb(const HsvColor& hsv, std::uint8_t alpha)
    {
        const float chroma = hsv.value * hsv.saturation;
        const float section = hsv.hue / 60.f;
        const float x = chroma * (1.f - std::fabs(std::fmod(section, 2.f) - 1.f));
        const float match = hsv.value - chroma;

        float red = 0.f;
        float green = 0.f;
        float blue = 0.f;

        if (section < 1.f)
        {
            red = chroma;
            green = x;
        }
        else if (section < 2.f)
        {
            red = x;
            green = chroma;
        }
        else if (section < 3.f)
        {
            green = chroma;
            blue = x;
        }
        else if (section < 4.f)
        {
            green = x;
            blue = chroma;
        }
        else if (section < 5.f)
        {
            red = x;
            blue = chroma;
        }
        else
        {
            red = chroma;
            blue = x;
        }

        return sf::Color(
            static_cast<std::uint8_t>(std::clamp((red + match) * 255.f, 0.f, 255.f)),
            static_cast<std::uint8_t>(std::clamp((green + match) * 255.f, 0.f, 255.f)),
            static_cast<std::uint8_t>(std::clamp((blue + match) * 255.f, 0.f, 255.f)),
            alpha);
    }
}

void applySecondPlayerPalette(sf::Image& image, int characterIndex)
{
    const sf::Vector2u size = image.getSize();

    for (unsigned int y = 0; y < size.y; ++y)
    {
        for (unsigned int x = 0; x < size.x; ++x)
        {
            const sf::Color original = image.getPixel({ x, y });

            if (original.a == 0)
                continue;

            HsvColor color = rgbToHsv(original);
            bool replace = false;

            switch (characterIndex)
            {
            case 0:
                replace = color.saturation > 0.12f && color.value > 0.12f;
                color.hue = 352.f;
                color.saturation = std::max(color.saturation, 0.48f);
                break;

            case 1:
                replace = color.saturation > 0.12f && color.value > 0.12f;
                color.hue = 212.f;
                color.saturation = std::max(color.saturation, 0.48f);
                break;

            case 2:
                if (color.saturation > 0.22f &&
                    color.hue >= 65.f &&
                    color.hue <= 170.f)
                {
                    color.hue = 285.f;
                    color.saturation = std::max(color.saturation, 0.52f);
                    replace = true;
                }
                else if (color.saturation > 0.28f &&
                    (color.hue <= 65.f || color.hue >= 340.f) &&
                    color.value < 0.9f)
                {
                    color.hue = 215.f;
                    color.saturation = std::max(color.saturation, 0.55f);
                    replace = true;
                }
                break;

            case 3:
                replace = color.value > 0.18f;
                color.hue = 350.f;
                color.saturation = std::max(
                    color.saturation,
                    color.value > 0.55f ? 0.34f : 0.48f);
                break;

            case 4:
                replace = color.value > 0.15f;
                color.hue = 28.f;
                color.saturation = std::clamp(
                    std::max(color.saturation, 0.48f),
                    0.f,
                    0.78f);
                color.value *= 0.72f;
                break;

            case 5:
                replace =
                    color.saturation > 0.34f &&
                    (color.hue <= 70.f || color.hue >= 340.f) &&
                    color.value > 0.18f;

                color.hue = color.value > 0.72f ? 190.f : 218.f;
                color.saturation = std::max(color.saturation, 0.68f);
                break;

            default:
                break;
            }

            if (replace)
                image.setPixel({ x, y }, hsvToRgb(color, original.a));
        }
    }
}

bool Player::loadTexture(
    const std::string& path,
    bool useSecondPlayerPalette)
{
    textureLoaded = texture.loadFromFile(path);

    if (textureLoaded)
    {
        if (useSecondPlayerPalette)
        {
            sf::Image image = texture.copyToImage();
            applySecondPlayerPalette(image, characterIndex);
            textureLoaded = texture.loadFromImage(image);
        }

        texture.setSmooth(false);
    }

    std::string trappedPath = makeTrappedPath(path);
    auto trappedTexture = std::make_unique<sf::Texture>();

    if (trappedTexture->loadFromFile(trappedPath))
    {
        bool paletteLoaded = true;

        if (useSecondPlayerPalette)
        {
            sf::Image image = trappedTexture->copyToImage();
            applySecondPlayerPalette(image, characterIndex);
            paletteLoaded = trappedTexture->loadFromImage(image);
        }

        if (paletteLoaded)
        {
            trappedTexture->setSmooth(false);
            trappedTextures[this] = std::move(trappedTexture);
        }
        else
        {
            trappedTextures.erase(this);
        }
    }
    else
    {
        trappedTextures.erase(this);
    }

    return textureLoaded;
}

static sf::Color getPlayerColor(const Player& player)
{
    if (player.playerNumber == 1)
        return sf::Color(255, 70, 80, 210);

    return sf::Color(70, 140, 255, 210);
}

static void drawPlayerIndicator(
    const Player& player,
    sf::RenderWindow& window)
{
    sf::Vector2f pos = getRenderPos(player);
    sf::Color color = getPlayerColor(player);

    sf::CircleShape ground(13.f);
    ground.setOrigin({ 13.f, 13.f });
    ground.setScale({ 1.f, 0.35f });
    ground.setPosition({ pos.x, pos.y + 16.f });
    ground.setFillColor(
        sf::Color(color.r, color.g, color.b, 65));
    ground.setOutlineThickness(2.f);
    ground.setOutlineColor(
        sf::Color(color.r, color.g, color.b, 180));

    window.draw(ground);

    sf::ConvexShape arrow;
    arrow.setPointCount(3);
    arrow.setPoint(0, { 0.f, 0.f });
    arrow.setPoint(1, { 10.f, 0.f });
    arrow.setPoint(2, { 5.f, 7.f });
    arrow.setOrigin({ 5.f, 3.5f });
    arrow.setPosition({ pos.x, pos.y - 27.f });
    arrow.setFillColor(color);

    window.draw(arrow);
}

static void drawNormalPlayer(
    const Player& player,
    sf::RenderWindow& window)
{
    const sf::Vector2u textureSize = player.texture.getSize();

    const int frameWidth =
        static_cast<int>(textureSize.x) / PlayerFrameCount;

    const int frameHeight =
        static_cast<int>(textureSize.y) / 4;

    if (frameWidth <= 0 || frameHeight <= 0)
        return;

    sf::Sprite sprite(player.texture);

    sprite.setTextureRect(sf::IntRect(
        {
            player.currentFrame * frameWidth,
            dirRow(player.dir) * frameHeight
        },
        {
            frameWidth,
            frameHeight
        }));

        sprite.setOrigin({
            frameWidth / 2.f,
            frameHeight / 2.f
            });

        float scale =
            player.drawSize /
            static_cast<float>(frameHeight);

        sprite.setScale({ scale, scale });

        static constexpr float Character1Offsets[4][4] = {
            {54.5f, 20.5f, -18.5f, -54.5f},
            {53.5f, 19.5f, -18.5f, -53.0f},
            {28.0f, 0.5f, 10.5f, -65.0f},
            {59.5f, -12.5f, -4.5f, -39.0f}
        };

        static constexpr float Character6Offsets[4][4] = {
            {43.5f, 21.0f, -15.0f, -58.5f},
            {45.0f, 29.5f, 3.0f, -30.0f},
            {48.0f, 26.0f, -18.5f, -48.5f},
            {46.0f, 25.5f, -8.0f, -50.0f}
        };

        const int row = dirRow(player.dir);
        const int frame =
            player.currentFrame % PlayerFrameCount;

        float sourceOffsetX = 0.f;

        if (player.characterIndex == 0)
            sourceOffsetX =
            Character1Offsets[row][frame];
        else if (player.characterIndex == 5)
            sourceOffsetX =
            Character6Offsets[row][frame];

        sf::Vector2f renderPosition =
            getRenderPos(player);

        renderPosition.x -=
            sourceOffsetX * scale;

        sprite.setPosition(renderPosition);

        drawPlayerIndicator(player, window);
        window.draw(sprite);
}

static void drawBubbleBack(
    const Player& player,
    sf::RenderWindow& window)
{
    sf::Vector2f pos = getRenderPos(player);

    float ratio =
        player.trappedAnimTimer /
        Player::TrapDuration;

    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;

    float appear =
        player.trappedAnimTimer / 0.25f;

    if (appear > 1.f)
        appear = 1.f;

    float pulseSpeed = 6.f;
    float pulsePower = 1.2f;

    if (ratio > 0.7f)
    {
        pulseSpeed = 10.f;
        pulsePower = 2.f;
    }

    if (ratio > 0.88f)
    {
        pulseSpeed = 22.f;
        pulsePower = 3.f;
    }

    float radius =
        (25.f +
            std::sin(
                player.trappedAnimTimer *
                pulseSpeed) *
            pulsePower) *
        appear;

    int alpha =
        static_cast<int>(
            45.f + 125.f * ratio);

    if (alpha > 180)
        alpha = 180;

    sf::CircleShape bubble(radius);

    bubble.setOrigin({ radius, radius });
    bubble.setPosition(pos);

    bubble.setFillColor(
        sf::Color(
            60,
            180,
            250,
            static_cast<unsigned char>(alpha)));

    window.draw(bubble);
}

static void drawTrappedPlayer(
    const Player& player,
    sf::RenderWindow& window)
{
    auto it =
        trappedTextures.find(&player);

    if (it == trappedTextures.end())
    {
        drawNormalPlayer(player, window);
        return;
    }

    const sf::Texture& trappedTexture =
        *it->second;

    const sf::Vector2u textureSize =
        trappedTexture.getSize();

    const int frameWidth =
        static_cast<int>(textureSize.x) /
        PlayerFrameCount;

    const int frameHeight =
        static_cast<int>(textureSize.y);

    if (frameWidth <= 0 ||
        frameHeight <= 0)
    {
        drawNormalPlayer(player, window);
        return;
    }

    float ratio =
        player.trappedAnimTimer /
        Player::TrapDuration;

    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;

    sf::Vector2f pos =
        getRenderPos(player);

    float shakePower = 1.f;
    float shakeSpeed = 13.f;

    if (ratio > 0.65f)
    {
        shakePower = 2.f;
        shakeSpeed = 18.f;
    }

    if (ratio > 0.88f)
    {
        shakePower = 3.5f;
        shakeSpeed = 28.f;
    }

    pos.x +=
        std::sin(
            player.trappedAnimTimer *
            shakeSpeed) *
        shakePower;

    pos.y +=
        std::sin(
            player.trappedAnimTimer *
            11.f);

    sf::Sprite sprite(trappedTexture);

    sprite.setTextureRect(
        sf::IntRect(
            {
                player.currentFrame *
                frameWidth,
                0
            },
            {
                frameWidth,
                frameHeight
            }));

            sprite.setOrigin({
                frameWidth / 2.f,
                frameHeight / 2.f
                });

            float scale =
                52.f /
                static_cast<float>(frameHeight);

            scale *=
                1.f +
                std::sin(
                    player.trappedAnimTimer *
                    8.f) *
                0.015f;

            sprite.setScale({ scale, scale });
            sprite.setPosition(pos);

            drawPlayerIndicator(player, window);
            window.draw(sprite);
}

static void drawBubbleFront(
    const Player& player,
    sf::RenderWindow& window)
{
    sf::Vector2f pos =
        getRenderPos(player);

    float ratio =
        player.trappedAnimTimer /
        Player::TrapDuration;

    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;

    float appear =
        player.trappedAnimTimer / 0.25f;

    if (appear > 1.f)
        appear = 1.f;

    float pulseSpeed = 6.f;
    float pulsePower = 1.2f;

    if (ratio > 0.7f)
    {
        pulseSpeed = 10.f;
        pulsePower = 2.f;
    }

    if (ratio > 0.88f)
    {
        pulseSpeed = 22.f;
        pulsePower = 3.f;
    }

    float radius =
        (25.f +
            std::sin(
                player.trappedAnimTimer *
                pulseSpeed) *
            pulsePower) *
        appear;

    int outlineAlpha =
        static_cast<int>(
            140.f + 100.f * ratio);

    if (outlineAlpha > 255)
        outlineAlpha = 255;

    sf::CircleShape outline(radius);

    outline.setOrigin({
        radius,
        radius
        });

    outline.setPosition(pos);

    outline.setFillColor(
        sf::Color::Transparent);

    outline.setOutlineThickness(2.f);

    outline.setOutlineColor(
        sf::Color(
            170,
            235,
            255,
            static_cast<unsigned char>(
                outlineAlpha)));

    window.draw(outline);

    sf::CircleShape highlight(
        radius * 0.17f);

    highlight.setOrigin({
        radius * 0.17f,
        radius * 0.17f
        });

    highlight.setPosition({
        pos.x - radius * 0.37f,
        pos.y - radius * 0.39f
        });

    highlight.setFillColor(
        sf::Color(
            255,
            255,
            255,
            180));

    window.draw(highlight);

    for (int i = 0; i < 3; ++i)
    {
        float smallRadius =
            1.8f + i * 0.7f;

        float phase =
            player.trappedAnimTimer *
            (1.5f + i * 0.4f) +
            i * 2.f;

        float rise =
            std::fmod(
                player.trappedAnimTimer *
                (9.f + i * 3.f) +
                i * 9.f,
                radius * 1.25f);

        sf::CircleShape smallBubble(
            smallRadius);

        smallBubble.setOrigin({
            smallRadius,
            smallRadius
            });

        smallBubble.setPosition({
            pos.x +
                std::sin(phase) *
                radius * 0.35f,

            pos.y +
                radius * 0.38f -
                rise
            });

        smallBubble.setFillColor(
            sf::Color(
                215,
                245,
                255,
                150));

        window.draw(smallBubble);
    }

    if (ratio > 0.88f)
    {
        float warning =
            (ratio - 0.88f) /
            0.12f;

        if (warning > 1.f)
            warning = 1.f;

        int warningAlpha =
            static_cast<int>(
                warning * 130.f);

        sf::CircleShape warningRing(
            radius + 3.f);

        warningRing.setOrigin({
            radius + 3.f,
            radius + 3.f
            });

        warningRing.setPosition(pos);

        warningRing.setFillColor(
            sf::Color::Transparent);

        warningRing.setOutlineThickness(2.f);

        warningRing.setOutlineColor(
            sf::Color(
                255,
                255,
                255,
                static_cast<unsigned char>(
                    warningAlpha)));

        window.draw(warningRing);
    }
}

void Player::draw(
    sf::RenderWindow& window) const
{
    if (!textureLoaded)
        return;

    if (trapBursting)
    {
        sf::Vector2f pos =
            getRenderPos(*this);

        float t =
            trapBurstTimer /
            TrapBurstDuration;

        if (t > 1.f)
            t = 1.f;

        float radius =
            20.f + 30.f * t;

        int alpha =
            static_cast<int>(
                220.f * (1.f - t));

        sf::CircleShape burst(radius);

        burst.setOrigin({
            radius,
            radius
            });

        burst.setPosition(pos);

        burst.setFillColor(
            sf::Color(
                80,
                200,
                255,
                static_cast<unsigned char>(
                    alpha)));

        burst.setOutlineThickness(3.f);

        burst.setOutlineColor(
            sf::Color(
                220,
                250,
                255,
                static_cast<unsigned char>(
                    alpha)));

        window.draw(burst);

        for (int i = 0; i < 8; i++)
        {
            float angle =
                static_cast<float>(i) *
                3.141592f /
                4.f;

            float distance =
                10.f + 30.f * t;

            float r =
                4.f * (1.f - t) +
                1.f;

            sf::CircleShape drop(r);

            drop.setOrigin({ r, r });

            drop.setPosition({
                pos.x +
                    std::cos(angle) *
                    distance,

                pos.y +
                    std::sin(angle) *
                    distance
                });

            drop.setFillColor(
                sf::Color(
                    100,
                    210,
                    255,
                    static_cast<unsigned char>(
                        alpha)));

            window.draw(drop);
        }

        return;
    }

    if (!alive)
    {
        drawTrappedPlayer(*this, window);
        return;
    }

    if (!trapped)
    {
        drawNormalPlayer(*this, window);
        return;
    }

    drawBubbleBack(*this, window);
    drawTrappedPlayer(*this, window);
    drawBubbleFront(*this, window);
}

void resetPlayer(
    Player& player,
    int row,
    int col)
{
    player.row = row;
    player.col = col;

    player.targetRow = row;
    player.targetCol = col;

    player.moving = false;
    player.progress = 0.f;

    player.alive = true;
    player.trapped = false;
    player.trappedAnimTimer = 0.f;

    player.bombCapacity = 1;
    player.blastRange = 1;
    player.needles = 0;

    player.dir = "down";
    player.currentFrame = 0;
    player.animTimer = 0.f;

    player.trapBursting = false;
    player.trapBurstTimer = 0.f;

    player.cratePushLocked = false;
}

void updatePlayer(
    Player& player,
    TileMap& map,
    float deltaTime,
    const std::function<bool(sf::Vector2i)>& isBalloonAt,
    const std::function<bool(sf::Vector2i)>& isPlayerAt)
{
    if (!player.alive)
    {
        player.moving = false;
        player.currentFrame = 2;
        player.animTimer = 0.f;
        return;
    }

    if (player.trapped)
    {
        player.moving = false;
        player.trappedAnimTimer += deltaTime;
        player.animTimer += deltaTime;

        float trappedAnimSpeed = 0.11f;

        if (player.trappedAnimTimer >
            Player::TrapDuration * 0.7f)
        {
            trappedAnimSpeed = 0.085f;
        }

        if (player.trappedAnimTimer >
            Player::TrapDuration * 0.88f)
        {
            trappedAnimSpeed = 0.06f;
        }

        if (player.animTimer >=
            trappedAnimSpeed)
        {
            player.animTimer -=
                trappedAnimSpeed;

            player.currentFrame =
                (player.currentFrame + 1) %
                PlayerFrameCount;
        }

        return;
    }

    player.trappedAnimTimer = 0.f;

    if (player.moving)
    {
        player.progress +=
            player.speed * deltaTime;

        player.animTimer += deltaTime;

        if (player.animTimer >=
            player.animSpeed)
        {
            player.animTimer -=
                player.animSpeed;

            player.currentFrame =
                (player.currentFrame + 1) %
                PlayerFrameCount;
        }

        if (player.progress >= 1.f)
        {
            player.row =
                player.targetRow;

            player.col =
                player.targetCol;

            player.moving = false;
            player.progress = 0.f;

            player.currentFrame = 0;
            player.animTimer = 0.f;
        }

        return;
    }

    int deltaColumn = 0;
    int deltaRow = 0;

    if (sf::Keyboard::isKeyPressed(
        player.keyUp))
    {
        deltaRow = -1;
        player.dir = "up";
    }
    else if (sf::Keyboard::isKeyPressed(
        player.keyDown))
    {
        deltaRow = 1;
        player.dir = "down";
    }
    else if (sf::Keyboard::isKeyPressed(
        player.keyLeft))
    {
        deltaColumn = -1;
        player.dir = "left";
    }
    else if (sf::Keyboard::isKeyPressed(
        player.keyRight))
    {
        deltaColumn = 1;
        player.dir = "right";
    }

    if (deltaColumn == 0 &&
        deltaRow == 0)
    {
        player.cratePushLocked = false;
        player.currentFrame = 0;
        player.animTimer = 0.f;
        return;
    }

    const int nextColumn =
        player.col + deltaColumn;

    const int nextRow =
        player.row + deltaRow;

    sf::Vector2i nextTile = {
        nextColumn,
        nextRow
    };

    bool balloonBlocked = false;

    if (isBalloonAt)
        balloonBlocked =
        isBalloonAt(nextTile);

    TileType nextType =
        map.getTile(
            nextColumn,
            nextRow);

    bool canMove = false;

    if (nextType == TileType::Empty ||
        nextType == TileType::HideableBush)
    {
        canMove = true;
    }
    else if (
        nextType ==
        TileType::PushableCrate)
    {
        int crateNextColumn =
            nextColumn + deltaColumn;

        int crateNextRow =
            nextRow + deltaRow;

        sf::Vector2i crateNextTile = {
            crateNextColumn,
            crateNextRow
        };

        bool playerBlocked = false;
        bool balloonBlockedBehind = false;

        if (isPlayerAt)
            playerBlocked =
            isPlayerAt(crateNextTile);

        if (isBalloonAt)
            balloonBlockedBehind =
            isBalloonAt(crateNextTile);

        if (!player.cratePushLocked &&
            !playerBlocked &&
            !balloonBlockedBehind)
        {
            canMove =
                map.tryPushCrate(
                    nextColumn,
                    nextRow,
                    deltaColumn,
                    deltaRow);

            if (canMove)
                player.cratePushLocked = true;
        }
    }

    if (canMove &&
        !balloonBlocked)
    {
        player.targetCol =
            nextColumn;

        player.targetRow =
            nextRow;

        player.moving = true;
        player.progress = 0.f;

        player.currentFrame = 0;
        player.animTimer = 0.f;
    }
}

sf::Vector2f getRenderPos(
    const Player& player)
{
    float renderRow =
        static_cast<float>(
            player.row);

    float renderColumn =
        static_cast<float>(
            player.col);

    if (player.moving)
    {
        renderRow +=
            static_cast<float>(
                player.targetRow -
                player.row) *
            player.progress;

        renderColumn +=
            static_cast<float>(
                player.targetCol -
                player.col) *
            player.progress;
    }

    return {
        static_cast<float>(
            TileMap::OriginX) +
            (renderColumn + 0.5f) *
            TileMap::TileSize,

        static_cast<float>(
            TileMap::OriginY) +
            (renderRow + 0.5f) *
            TileMap::TileSize
    };
}

sf::Vector2i getTilePosition(
    const Player& player)
{
    return {
        player.col,
        player.row
    };
}

void killPlayer(Player& player)
{
    player.alive = false;
    player.trapped = false;

    player.moving = false;
    player.progress = 0.f;

    player.trappedAnimTimer =
        Player::TrapDuration;

    player.currentFrame = 2;
    player.animTimer = 0.f;

    player.trapBursting = false;
    player.trapBurstTimer = 0.f;
}

bool isAlive(
    const Player& player)
{
    return player.alive;
}