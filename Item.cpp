#include "Item.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
using namespace std;

ItemEffect getItemEffect(ItemType type)
{
    ItemEffect effect{};
    switch (type)
    {
    case ItemType::Needle: effect.needleDelta = 1; break;
    case ItemType::BlastRange: effect.blastRangeDelta = 1; break;
    case ItemType::WaterBalloon: effect.bombCapacityDelta = 1; break;
    case ItemType::SpeedIncrease: effect.speedMultiplier = 1.15f; break;
    }
    return effect;
}
void applyItemEffect(ItemStats& s, ItemType type, const ItemLimits& limits)
{
    const auto e = getItemEffect(type);
    if (e.bombCapacityDelta && s.bombCapacity < limits.bombCapacity) ++s.bombCapacity;
    if (e.blastRangeDelta && s.blastRange < limits.blastRange) ++s.blastRange;
    if (e.needleDelta && s.needles < limits.needles) ++s.needles;
    if (e.speedMultiplier > 1.f && s.speed < limits.speed)
        s.speed = min(s.speed * e.speedMultiplier, limits.speed);
}
bool tryUseNeedle(ItemStats& stats, bool& trapped, bool alive)
{
    if (!alive || !trapped || stats.needles <= 0) return false;
    --stats.needles;
    trapped = false;
    return true;
}
Item::Item(ItemType type, sf::Vector2f position)
    : m_type(type), m_position(position), m_sprite(m_texture)
{
    m_sprite.setPosition(position);
}
ItemType Item::getType() const { return m_type; }
sf::Vector2f Item::getPosition() const { return m_position; }
bool Item::loadTexture(const string& folder)
{
    string name;
    switch (m_type)
    {
    case ItemType::Needle: name = "needle.png"; break;
    case ItemType::BlastRange: name = "watercourse.png"; break;
    case ItemType::WaterBalloon: name = "waterballoon.png"; break;
    case ItemType::SpeedIncrease: name = "speedincrease.png"; break;
    default: return false;
    }
    if (!m_texture.loadFromFile(filesystem::path(folder) / name)) return false;
    m_texture.setSmooth(false);
    m_sprite.setTexture(m_texture, true);
    const auto size = m_texture.getSize();
    m_sprite.setOrigin({size.x / 2.f, size.y / 2.f});
    m_loaded = true;
    setSize(m_size);
    return true;
}
void Item::draw(sf::RenderTarget& target) const
{
    if (m_loaded && !m_collected) target.draw(m_sprite);
}
sf::FloatRect Item::getBounds() const
{
    if (!m_loaded || m_collected) return {};
    return m_sprite.getGlobalBounds();
}
void Item::setSize(float size)
{
    if (!isfinite(size) || size <= 0.f) return;
    m_size = size;
    if (!m_loaded) return;
    const auto dimensions = m_texture.getSize();
    m_sprite.setScale({size / dimensions.x, size / dimensions.y});
}
void Item::collect() { m_collected = true; }
bool Item::isCollected() const { return m_collected; }
ItemManager::ItemManager(unsigned seed) : m_random(seed) {}
bool ItemManager::spawn(ItemType type, sf::Vector2f center, float size, const string& folder)
{
    if (!isfinite(center.x) || !isfinite(center.y) || !isfinite(size) || size <= 0) return false;
    for (const auto& item : m_items)
        if (item->getPosition() == center) return false;
    auto item = make_unique<Item>(type, center);
    item->setSize(size);
    if (!item->loadTexture(folder)) return false;
    m_items.push_back(move(item));
    return true;
}
bool ItemManager::trySpawn(sf::Vector2f center, float probability, float size, const string& folder)
{
    if (!isfinite(probability)) return false;
    if (!bernoulli_distribution(clamp(probability, 0.f, 1.f))(m_random)) return false;
    const auto type = static_cast<ItemType>(uniform_int_distribution<int>(0, 3)(m_random));
    return spawn(type, center, size, folder);
}
optional<ItemType> ItemManager::collectAt(const sf::FloatRect& bounds, bool canCollect)
{
    if (!canCollect) return nullopt;
    for (auto it = m_items.begin(); it != m_items.end(); ++it)
    {
        if ((*it)->getBounds().findIntersection(bounds))
        {
            const auto type = (*it)->getType();
            (*it)->collect();
            m_items.erase(it);
            return type;
        }
    }
    return nullopt;
}
void ItemManager::draw(sf::RenderTarget& target) const
{
    for (const auto& item : m_items) item->draw(target);
}
void ItemManager::clear() { m_items.clear(); }
size_t ItemManager::getItemCount() const { return m_items.size(); }
