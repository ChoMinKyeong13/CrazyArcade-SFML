#include "UI.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
using namespace std;

bool UI::loadFont(const string& path)
{
    m_ready = m_font.openFromFile(path);
    return m_ready;
}

void UI::draw(
    sf::RenderTarget& target,
    const array<PlayerHudData, 2>& players,
    float secondsRemaining,
    const string& status) const
{
    if (!m_ready) return;

    const auto oldView = target.getView();
    target.setView(target.getDefaultView());

    const float width = target.getDefaultView().getSize().x;
    const float height = target.getDefaultView().getSize().y;

    constexpr float panelWidth = 200.f;
    const float panelX = width - panelWidth;

    sf::RectangleShape panel({ panelWidth, height });
    panel.setPosition({ panelX, 0.f });
    panel.setFillColor(sf::Color(20, 29, 45, 235));
    target.draw(panel);

    for (size_t i = 0; i < players.size(); ++i)
    {
        const auto& p = players[i];

        ostringstream line;

        line << (i + 1) << "P  "
            << (!p.alive ? "OUT" : p.trapped ? "TRAPPED" : "READY")
            << "\n\nBomb      " << p.stats.bombCapacity
            << "\nRange     " << p.stats.blastRange
            << "\nSpeed     " << fixed << setprecision(0) << p.stats.speed
            << "\nNeedle    " << p.stats.needles;

        sf::Text text(m_font, line.str(), 17);

        text.setPosition({
            panelX + 18.f,
            35.f + static_cast<float>(i) * 210.f
            });

        text.setFillColor(
            i == 0
            ? sf::Color(110, 220, 255)
            : sf::Color(255, 205, 100)
        );

        target.draw(text);
    }

    const int time =
        isfinite(secondsRemaining)
        ? static_cast<int>(
            clamp(secondsRemaining, 0.f, 359999.f))
        : 0;

    ostringstream label;

    label << "TIME "
        << time / 60
        << ':'
        << setw(2)
        << setfill('0')
        << time % 60;

    sf::Text timeText(
        m_font,
        label.str(),
        21
    );

    timeText.setStyle(sf::Text::Bold);

    timeText.setPosition({
        panelX + 18.f,
        465.f
        });

    target.draw(timeText);

    if (!status.empty())
    {
        sf::Text statusText(
            m_font,
            sf::String::fromUtf8(
                status.begin(),
                status.end()),
            13
        );

        statusText.setFillColor(
            sf::Color(165, 171, 200));

        statusText.setPosition({
            panelX + 14.f,
            510.f
            });

        target.draw(statusText);
    }

    target.setView(oldView);
}