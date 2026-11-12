#pragma once
#include "Item.h"
#include <array>
#include <string>

struct PlayerHudData
{
    ItemStats stats;
    bool alive = true;
    bool trapped = false;
};
// 생존/시간/승패는 Game과 Player에서 받아 표시만 합니다.
class UI
{
public:
    bool loadFont(const std::string& path);
    void draw(sf::RenderTarget& target, const std::array<PlayerHudData, 2>& players,
              float secondsRemaining, const std::string& status = "") const;
private:
    sf::Font m_font;
    bool m_ready = false;
};
