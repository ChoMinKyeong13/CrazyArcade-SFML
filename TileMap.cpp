#include "TileMap.h"
#include <SFML/Graphics/Sprite.hpp>
#include <string_view>
#include <utility>

namespace
{
// Five village columns | trees | three road columns | trees | five village columns.
// . empty, H/U/Y red/blue/yellow house, T tall tree,
// B destructible shrub, C wooden crate, R/O red/orange destructible brick.
constexpr std::array<std::string_view, TileMap::Rows> Layout = {
    ".OROR" "T" "..C" "T" "YRY.Y",
    ".HCHC" "T" "C.." "T" "RO...",
    ".OROR" "T" ".CC" "T" "YCYCY",
    "CHCHC" "T" "C.." "T" "ORORO",
    "ROROR" "T" "..C" "T" "YCYCY",
    "OHOHO" "B" "CC." "B" "ROROR", // Shrub gates connect both upper villages to the road.
    "TBTBT" "B" "..C" "B" "TBTBT",
    "ROROR" "." "C.." "B" "RHRHR", // Destructible gate into the right village.
    "UCUCU" "T" ".CC" "T" "ORORO",
    "ROROR" "T" "C.." "T" "CHCHC",
    "U.UCU" "T" "..C" "T" "RORO.",
    "..ROR" "T" "CC." "T" "CHCH.",
    "U.URU" "B" "..C" "T" "ORO.."
};

constexpr bool validLayout()
{
    for (auto row : Layout)
    {
        if (row.size() != TileMap::Columns) return false;
        for (char c : row)
            if (std::string_view(".HUYTBCRO").find(c) == std::string_view::npos)
                return false;
    }
    return Layout[TileMap::SpawnY][TileMap::SpawnX] == '.';
}
static_assert(validLayout(), "Map rows must contain exactly 15 valid cells and an empty spawn.");

ObjectStyle objectFromSymbol(char c)
{
    switch (c)
    {
    case 'H': return ObjectStyle::HouseRed;
    case 'U': return ObjectStyle::HouseBlue;
    case 'Y': return ObjectStyle::HouseYellow;
    case 'T': return ObjectStyle::Tree;
    case 'B': return ObjectStyle::Bush;
    case 'C': return ObjectStyle::Crate;
    case 'R': return ObjectStyle::BrickRed;
    case 'O': return ObjectStyle::BrickOrange;
    default: return ObjectStyle::None;
    }
}

sf::Vector2f cellPosition(int x, int y)
{
    return {static_cast<float>(TileMap::OriginX + x * TileMap::TileSize),
            static_cast<float>(TileMap::OriginY + y * TileMap::TileSize)};
}

void drawTexture(sf::RenderTarget& target, const sf::Texture& texture,
                 sf::Vector2f position, bool isGround)
{
    sf::Sprite sprite(texture);
    const auto dimensions = texture.getSize();
    const float scale = static_cast<float>(TileMap::TileSize) / dimensions.x;
    if (isGround)
    {
        sprite.setScale({scale, static_cast<float>(TileMap::TileSize) / dimensions.y});
        sprite.setPosition(position);
    }
    else
    {
        // A tall PNG occupies one collision cell. Its feet sit on the cell bottom.
        sprite.setScale({scale, scale});
        sprite.setOrigin({0.f, static_cast<float>(dimensions.y)});
        sprite.setPosition({position.x, position.y + TileMap::TileSize});
    }
    target.draw(sprite);
}
}

TileMap::TileMap(const std::filesystem::path& assetDirectory)
{
    reset();
    loadTextures(assetDirectory);
}

void TileMap::loadTextures(const std::filesystem::path& directory)
{
    constexpr std::array<const char*, 4> groundNames = {
        "grass.png", "road.png", "road_line.png", "crosswalk.png"};
    constexpr std::array<const char*, 9> objectNames = {
        "", "house_red.png", "house_blue.png", "house_yellow.png",
        "tree.png", "bush.png", "crate.png", "brick_red.png", "brick_orange.png"};
    auto load = [&](auto& textures, const auto& names)
    {
        for (std::size_t i = 0; i < names.size(); ++i)
        {
            if (names[i][0] == '\0') continue;
            const auto path = directory / names[i];
            if (!std::filesystem::is_regular_file(path)) continue;
            sf::Texture texture;
            if (texture.loadFromFile(path))
            {
                texture.setSmooth(false);
                textures[i] = std::move(texture);
            }
        }
    };
    load(m_groundTextures, groundNames);
    load(m_objectTextures, objectNames);
}

void TileMap::reset()
{
    for (int y = 0; y < Rows; ++y)
        for (int x = 0; x < Columns; ++x)
        {
            auto& tile = m_tiles[y][x];
            tile.ground = GroundStyle::Grass;
            if (x >= 6 && x <= 8)
                tile.ground = (y == 2 || y == 10) ? GroundStyle::Crosswalk :
                    (x == 7 ? GroundStyle::RoadLine : GroundStyle::Road);
            tile.object = objectFromSymbol(Layout[y][x]);
        }
}

void TileMap::drawGround(sf::RenderTarget& target) const
{
    for (int y = 0; y < Rows; ++y)
        for (int x = 0; x < Columns; ++x)
        {
            const auto style = m_tiles[y][x].ground;
            const auto& texture = m_groundTextures[static_cast<std::size_t>(style)];
            if (texture) drawTexture(target, *texture, cellPosition(x, y), true);
            else TileArt::drawGround(target, style, cellPosition(x, y), TileSize);
        }
}

void TileMap::drawObjectRow(sf::RenderTarget& target, int row) const
{
    if (row < 0 || row >= Rows) return;
    for (int x = 0; x < Columns; ++x)
    {
        const auto style = m_tiles[row][x].object;
        if (style == ObjectStyle::None) continue;
        const auto& texture = m_objectTextures[static_cast<std::size_t>(style)];
        if (texture) drawTexture(target, *texture, cellPosition(x, row), false);
        else TileArt::drawObject(target, style, cellPosition(x, row), TileSize);
    }
}

void TileMap::draw(sf::RenderTarget& target) const
{
    drawGround(target);
    for (int y = 0; y < Rows; ++y) drawObjectRow(target, y);
}

bool TileMap::isInside(int x, int y) const
{
    return x >= 0 && x < Columns && y >= 0 && y < Rows;
}

ObjectStyle TileMap::getObject(int x, int y) const
{
    return isInside(x, y) ? m_tiles[y][x].object : ObjectStyle::None;
}

GroundStyle TileMap::getGround(int x, int y) const
{
    return isInside(x, y) ? m_tiles[y][x].ground : GroundStyle::Grass;
}

TileType TileMap::getTile(int x, int y) const
{
    if (!isInside(x, y)) return TileType::IndestructibleWall;

    switch (m_tiles[y][x].object)
    {
    case ObjectStyle::None:
        return TileType::Empty;

    case ObjectStyle::HouseRed:
    case ObjectStyle::HouseBlue:
    case ObjectStyle::HouseYellow:
    case ObjectStyle::Tree:
        return TileType::IndestructibleWall;

    case ObjectStyle::Bush:
        return TileType::HideableBush;

    case ObjectStyle::Crate:
        return TileType::PushableCrate;

    case ObjectStyle::BrickRed:
    case ObjectStyle::BrickOrange:
        return TileType::DestructibleBlock;
    }

    return TileType::IndestructibleWall;
}
bool TileMap::tryPushCrate(int x, int y, int dx, int dy)
{
    if (getTile(x, y) != TileType::PushableCrate) return false;

    int nextX = x + dx;
    int nextY = y + dy;

    if (getTile(nextX, nextY) != TileType::Empty) return false;

    m_tiles[nextY][nextX].object = ObjectStyle::Crate;
    m_tiles[y][x].object = ObjectStyle::None;
    return true;
}
bool TileMap::isWalkable(int x, int y) const
{
    TileType type = getTile(x, y);
    return type == TileType::Empty || type == TileType::HideableBush;
}

std::optional<sf::Vector2i> TileMap::worldToTile(sf::Vector2f position) const
{
    const float x = position.x - OriginX;
    const float y = position.y - OriginY;
    if (!(x >= 0.f && y >= 0.f && x < PixelWidth && y < PixelHeight))
        return std::nullopt;
    return sf::Vector2i{static_cast<int>(x) / TileSize, static_cast<int>(y) / TileSize};
}

bool TileMap::destroyBlock(int x, int y)
{
    if (!isInside(x, y)) return false;

    TileType type = getTile(x, y);

    if (type != TileType::DestructibleBlock
        && type != TileType::PushableCrate
        && type != TileType::HideableBush)
        return false;

    m_tiles[y][x].object = ObjectStyle::None;
    return true;
}

void TileMap::drawBushShake(sf::RenderTarget& target, int x, int y, float offsetX) const
{
    if (!isInside(x, y)) return;
    if (m_tiles[y][x].object != ObjectStyle::Bush) return;

    sf::Vector2f pos = cellPosition(x, y);
    pos.x += offsetX;

    const auto& texture = m_objectTextures[static_cast<std::size_t>(ObjectStyle::Bush)];

    if (texture)
        drawTexture(target, *texture, pos, false);
    else
        TileArt::drawObject(target, ObjectStyle::Bush, pos, TileSize);
}
