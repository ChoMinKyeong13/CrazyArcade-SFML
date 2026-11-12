#include "TileArt.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <initializer_list>

namespace
{
// Every drawing is expressed in tile units so the same artwork scales with
// the map. Shapes are short-lived; there are no texture ownership requirements.
struct Canvas
{
    sf::RenderTarget& target;
    sf::Vector2f origin;
    float size;

    void rect(float x, float y, float w, float h, sf::Color color) const
    {
        sf::RectangleShape shape({w * size, h * size});
        shape.setPosition({origin.x + x * size, origin.y + y * size});
        shape.setFillColor(color);
        target.draw(shape);
    }

    void ellipse(float x, float y, float w, float h, sf::Color color) const
    {
        sf::CircleShape shape(0.5f, 24);
        shape.setScale({w * size, h * size});
        shape.setPosition({origin.x + x * size, origin.y + y * size});
        shape.setFillColor(color);
        target.draw(shape);
    }

    void poly(std::initializer_list<sf::Vector2f> points, sf::Color color) const
    {
        sf::ConvexShape shape(points.size());
        std::size_t index = 0;
        for (const auto point : points)
            shape.setPoint(index++, {point.x * size, point.y * size});
        shape.setPosition(origin);
        shape.setFillColor(color);
        target.draw(shape);
    }

    void shadow() const
    {
        ellipse(0.08f, 0.79f, 0.87f, 0.20f, {35, 61, 18, 85});
    }
};

struct Palette
{
    sf::Color base;
    sf::Color dark;
    sf::Color light;
    sf::Color highlight;
};

void drawStud(const Canvas& c, float x, float y, float w, float h, const Palette& p)
{
    c.ellipse(x, y + 0.035f, w, h, p.dark);
    c.ellipse(x, y, w, h, p.light);
    c.ellipse(x + w * 0.13f, y + h * 0.08f, w * 0.64f, h * 0.42f, p.highlight);
}

void grass(const Canvas& c)
{
    const Palette p{{121, 199, 38}, {106, 179, 33}, {147, 218, 65}, {174, 232, 93}};
    c.rect(0.f, 0.f, 1.f, 1.f, p.base);
    c.rect(0.f, 0.f, 1.f, 0.025f, {161, 219, 61});
    c.rect(0.f, 0.975f, 1.f, 0.025f, {108, 183, 32});
    drawStud(c, 0.14f, 0.13f, 0.24f, 0.17f, p);
    drawStud(c, 0.62f, 0.13f, 0.24f, 0.17f, p);
    drawStud(c, 0.14f, 0.61f, 0.24f, 0.17f, p);
    drawStud(c, 0.62f, 0.61f, 0.24f, 0.17f, p);
}

void house(const Canvas& c, const Palette& roof)
{
    c.shadow();
    // Cream facade, darker right wall, and a small stone plinth.
    c.poly({{0.82f, 0.48f}, {0.94f, 0.39f}, {0.94f, 0.93f}, {0.82f, 0.99f}}, {131, 130, 111});
    c.poly({{0.10f, 0.51f}, {0.49f, 0.18f}, {0.87f, 0.51f}, {0.87f, 0.96f}, {0.10f, 0.96f}}, {226, 221, 185});
    c.poly({{0.17f, 0.54f}, {0.49f, 0.26f}, {0.79f, 0.54f}}, {249, 240, 199});
    c.rect(0.14f, 0.57f, 0.69f, 0.34f, {239, 230, 196});
    c.rect(0.11f, 0.93f, 0.77f, 0.055f, {165, 163, 147});
    c.rect(0.15f, 0.91f, 0.67f, 0.025f, {253, 241, 209});

    // The roof ridge runs from the back of the house to the front gable.
    c.poly({{0.16f, 0.04f}, {0.54f, -0.23f}, {0.49f, 0.20f}, {0.05f, 0.55f}}, roof.base);
    c.poly({{0.54f, -0.23f}, {0.91f, 0.03f}, {0.95f, 0.55f}, {0.49f, 0.20f}}, roof.dark);
    c.poly({{0.19f, 0.045f}, {0.52f, -0.19f}, {0.48f, 0.14f}, {0.10f, 0.45f}}, roof.light);
    c.poly({{0.55f, -0.18f}, {0.87f, 0.06f}, {0.89f, 0.43f}, {0.53f, 0.16f}}, roof.base);
    c.poly({{0.05f, 0.51f}, {0.49f, 0.15f}, {0.49f, 0.23f}, {0.05f, 0.59f}}, roof.highlight);
    c.poly({{0.49f, 0.15f}, {0.95f, 0.51f}, {0.95f, 0.59f}, {0.49f, 0.23f}}, roof.light);
    c.poly({{0.28f, -0.015f}, {0.36f, -0.075f}, {0.29f, 0.28f}, {0.21f, 0.35f}}, roof.highlight);
    c.poly({{0.53f, -0.23f}, {0.56f, -0.20f}, {0.51f, 0.18f}, {0.48f, 0.17f}}, roof.highlight);

    // Raised chimney; its dark opening remains visible on the bright roof.
    c.poly({{0.70f, -0.08f}, {0.85f, -0.04f}, {0.85f, 0.20f}, {0.70f, 0.15f}}, roof.light);
    c.poly({{0.85f, -0.04f}, {0.90f, -0.08f}, {0.90f, 0.16f}, {0.85f, 0.20f}}, roof.dark);
    c.poly({{0.68f, -0.09f}, {0.74f, -0.14f}, {0.91f, -0.09f}, {0.85f, -0.02f}}, roof.highlight);
    c.poly({{0.73f, -0.085f}, {0.76f, -0.11f}, {0.86f, -0.08f}, {0.83f, -0.045f}}, roof.dark);

    // Four-pane window with a chunky toy-like frame and a sloped lintel.
    c.poly({{0.28f, 0.67f}, {0.35f, 0.59f}, {0.62f, 0.59f}, {0.69f, 0.67f}}, {154, 156, 144});
    c.rect(0.28f, 0.66f, 0.41f, 0.26f, {151, 154, 145});
    c.rect(0.32f, 0.67f, 0.33f, 0.21f, {43, 64, 76});
    c.rect(0.34f, 0.69f, 0.11f, 0.065f, {104, 145, 152});
    c.rect(0.50f, 0.69f, 0.11f, 0.065f, {87, 126, 137});
    c.rect(0.46f, 0.66f, 0.04f, 0.23f, {226, 224, 203});
    c.rect(0.31f, 0.765f, 0.35f, 0.04f, {226, 224, 203});
    c.rect(0.26f, 0.895f, 0.45f, 0.035f, {246, 240, 216});
}

void tree(const Canvas& c)
{
    c.shadow();
    c.rect(0.43f, 0.67f, 0.17f, 0.28f, {137, 76, 32});
    c.rect(0.43f, 0.72f, 0.065f, 0.22f, {202, 132, 57});
    c.ellipse(0.10f, 0.39f, 0.82f, 0.47f, {13, 115, 25});
    c.ellipse(0.11f, 0.33f, 0.77f, 0.46f, {18, 157, 30});
    c.ellipse(0.18f, 0.16f, 0.66f, 0.52f, {9, 137, 27});
    c.ellipse(0.20f, 0.10f, 0.60f, 0.49f, {27, 178, 33});
    c.ellipse(0.28f, -0.06f, 0.46f, 0.48f, {11, 149, 26});
    c.ellipse(0.30f, -0.10f, 0.40f, 0.43f, {39, 192, 37});
    c.ellipse(0.41f, -0.27f, 0.20f, 0.32f, {31, 178, 30});
    c.ellipse(0.43f, -0.25f, 0.10f, 0.19f, {83, 210, 40});
    c.ellipse(0.30f, 0.12f, 0.12f, 0.21f, {74, 204, 43});
    c.ellipse(0.21f, 0.41f, 0.13f, 0.22f, {64, 196, 35});
    c.ellipse(0.42f, 0.37f, 0.12f, 0.15f, {52, 190, 37});
    c.ellipse(0.67f, 0.51f, 0.12f, 0.15f, {17, 143, 28});
    c.ellipse(0.48f, 0.66f, 0.11f, 0.10f, {35, 178, 31});
}

void bush(const Canvas& c)
{
    c.shadow();
    c.ellipse(0.07f, 0.49f, 0.88f, 0.43f, {13, 110, 24});
    c.ellipse(0.05f, 0.22f, 0.39f, 0.49f, {10, 137, 25});
    c.ellipse(0.31f, 0.06f, 0.42f, 0.56f, {17, 154, 26});
    c.ellipse(0.63f, 0.19f, 0.32f, 0.50f, {12, 137, 26});
    c.ellipse(0.21f, 0.39f, 0.44f, 0.44f, {20, 155, 29});
    c.ellipse(0.51f, 0.48f, 0.37f, 0.35f, {10, 140, 24});
    c.ellipse(0.22f, 0.08f, 0.27f, 0.30f, {36, 172, 31});
    c.ellipse(0.55f, 0.10f, 0.25f, 0.31f, {26, 163, 27});
    c.ellipse(0.07f, 0.58f, 0.28f, 0.28f, {29, 158, 28});
    c.ellipse(0.20f, 0.25f, 0.12f, 0.15f, {62, 188, 37});
    c.ellipse(0.42f, 0.13f, 0.10f, 0.15f, {58, 184, 35});
    c.ellipse(0.70f, 0.40f, 0.11f, 0.15f, {39, 174, 32});
    c.ellipse(0.32f, 0.59f, 0.12f, 0.13f, {53, 186, 35});
    // Small golden buds echo the garden trees in the reference map.
    for (const auto bud : {sf::Vector2f{0.28f, 0.28f}, {0.59f, 0.21f}, {0.72f, 0.58f}, {0.43f, 0.52f}, {0.22f, 0.70f}})
    {
        c.ellipse(bud.x, bud.y, 0.065f, 0.095f, {216, 191, 19});
        c.ellipse(bud.x, bud.y, 0.045f, 0.055f, {255, 232, 48});
    }
}

void crate(const Canvas& c)
{
    c.shadow();
    c.poly({{0.05f, 0.08f}, {0.14f, 0.015f}, {0.95f, 0.015f}, {0.89f, 0.08f}}, {253, 212, 103});
    c.rect(0.055f, 0.075f, 0.84f, 0.89f, {167, 111, 24});
    c.poly({{0.895f, 0.075f}, {0.96f, 0.02f}, {0.96f, 0.90f}, {0.895f, 0.965f}}, {132, 89, 21});
    c.rect(0.085f, 0.105f, 0.77f, 0.62f, {201, 147, 40});
    c.rect(0.14f, 0.16f, 0.67f, 0.51f, {176, 120, 28});
    c.rect(0.18f, 0.18f, 0.26f, 0.46f, {205, 152, 45});
    c.rect(0.455f, 0.18f, 0.16f, 0.46f, {194, 137, 34});
    c.rect(0.63f, 0.18f, 0.14f, 0.46f, {209, 155, 47});
    // Two broad diagonal slats, each with a narrow sunlit edge.
    c.poly({{0.13f, 0.17f}, {0.23f, 0.15f}, {0.82f, 0.62f}, {0.75f, 0.71f}}, {244, 198, 77});
    c.poly({{0.15f, 0.16f}, {0.20f, 0.15f}, {0.81f, 0.63f}, {0.79f, 0.66f}}, {255, 224, 123});
    c.poly({{0.75f, 0.14f}, {0.84f, 0.22f}, {0.22f, 0.72f}, {0.13f, 0.65f}}, {239, 189, 66});
    c.poly({{0.75f, 0.14f}, {0.78f, 0.17f}, {0.16f, 0.68f}, {0.13f, 0.65f}}, {255, 224, 123});
    c.rect(0.07f, 0.09f, 0.82f, 0.085f, {255, 216, 107});
    c.rect(0.07f, 0.12f, 0.08f, 0.62f, {250, 207, 92});
    c.rect(0.80f, 0.12f, 0.085f, 0.62f, {235, 183, 57});
    c.rect(0.075f, 0.67f, 0.815f, 0.09f, {247, 199, 76});
    c.rect(0.075f, 0.755f, 0.815f, 0.025f, {139, 92, 18});
    c.rect(0.09f, 0.79f, 0.18f, 0.14f, {185, 128, 25});
    c.rect(0.30f, 0.79f, 0.23f, 0.14f, {195, 138, 30});
    c.rect(0.56f, 0.79f, 0.29f, 0.14f, {184, 125, 24});
}

void brick(const Canvas& c, const Palette& p)
{
    c.shadow();
    c.rect(0.015f, 0.10f, 0.97f, 0.87f, p.dark);
    c.rect(0.025f, 0.02f, 0.95f, 0.74f, p.base);
    c.rect(0.025f, 0.02f, 0.95f, 0.035f, p.light);
    c.rect(0.025f, 0.045f, 0.035f, 0.70f, p.light);
    c.rect(0.94f, 0.055f, 0.035f, 0.70f, p.dark);
    c.rect(0.045f, 0.775f, 0.89f, 0.14f, p.base);
    c.rect(0.045f, 0.78f, 0.055f, 0.12f, p.light);
    c.rect(0.02f, 0.94f, 0.955f, 0.03f, {87, 42, 18, 65});
    drawStud(c, 0.13f, 0.11f, 0.28f, 0.21f, p);
    drawStud(c, 0.59f, 0.11f, 0.28f, 0.21f, p);
    drawStud(c, 0.13f, 0.43f, 0.28f, 0.21f, p);
    drawStud(c, 0.59f, 0.43f, 0.28f, 0.21f, p);
}
}

namespace TileArt
{
void drawGround(sf::RenderTarget& target, GroundStyle style, sf::Vector2f topLeft, float size)
{
    const Canvas c{target, topLeft, size};
    if (style == GroundStyle::Grass)
    {
        grass(c);
        return;
    }
    c.rect(0.f, 0.f, 1.f, 1.f, {156, 160, 162});
    if (style == GroundStyle::RoadLine)
    {
        c.rect(0.455f, 0.22f, 0.10f, 0.51f, {225, 228, 222});
        c.rect(0.455f, 0.22f, 0.025f, 0.51f, {238, 239, 232});
    }
    else if (style == GroundStyle::Crosswalk)
    {
        c.rect(0.f, 0.16f, 1.f, 0.68f, {188, 190, 188});
        for (int stripe = 0; stripe < 3; ++stripe)
            c.rect(0.10f + stripe * 0.32f, 0.20f, 0.18f, 0.59f, {231, 233, 224});
    }
}

void drawObject(sf::RenderTarget& target, ObjectStyle style, sf::Vector2f topLeft, float size)
{
    const Canvas c{target, topLeft, size};
    switch (style)
    {
    case ObjectStyle::None: break;
    case ObjectStyle::HouseRed:
        house(c, {{224, 68, 37}, {163, 47, 28}, {255, 118, 75}, {255, 181, 130}}); break;
    case ObjectStyle::HouseBlue:
        house(c, {{12, 133, 220}, {10, 74, 151}, {37, 189, 248}, {165, 237, 251}}); break;
    case ObjectStyle::HouseYellow:
        house(c, {{229, 189, 37}, {168, 136, 25}, {255, 225, 88}, {255, 249, 176}}); break;
    case ObjectStyle::Tree: tree(c); break;
    case ObjectStyle::Bush: bush(c); break;
    case ObjectStyle::Crate: crate(c); break;
    case ObjectStyle::BrickRed:
        brick(c, {{236, 58, 31}, {165, 41, 17}, {255, 92, 52}, {255, 144, 99}}); break;
    case ObjectStyle::BrickOrange:
        brick(c, {{248, 147, 48}, {184, 91, 23}, {255, 177, 81}, {255, 209, 135}}); break;
    }
}
}
