#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>

enum class GroundStyle { Grass, Road, RoadLine, Crosswalk };
enum class ObjectStyle
{
    None, HouseRed, HouseBlue, HouseYellow, Tree, Bush, Crate, BrickRed, BrickOrange
};

namespace TileArt
{
// Coordinates describe the one-tile collision footprint. Tall decorations may
// extend 0.3 tiles upwards, so render objects in order of their foot positions.
void drawGround(sf::RenderTarget& target, GroundStyle style, sf::Vector2f topLeft, float size);
void drawObject(sf::RenderTarget& target, ObjectStyle style, sf::Vector2f topLeft, float size);
}
