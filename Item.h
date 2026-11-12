#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

enum class ItemType { Needle, BlastRange, WaterBalloon, SpeedIncrease };
struct ItemEffect
{
    int bombCapacityDelta = 0;
    int blastRangeDelta = 0;
    float speedMultiplier = 1.f;
    int needleDelta = 0;
};
ItemEffect getItemEffect(ItemType type);

// B 담당의 실제 Player와 분리된 전달용 능력치입니다.
struct ItemStats
{
    int bombCapacity = 1;
    int blastRange = 1;
    float speed = 120.f;
    int needles = 0;
};
struct ItemLimits
{
    int bombCapacity = 8;
    int blastRange = 8;
    float speed = 320.f;
    int needles = 3;
};
void applyItemEffect(ItemStats& stats, ItemType type, const ItemLimits& limits = {});
bool tryUseNeedle(ItemStats& stats, bool& trapped, bool alive);

class Item
{
public:
    Item(ItemType type, sf::Vector2f position);
    // Sprite가 자신의 Texture를 참조하므로 객체 자체의 복사/이동을 금지합니다.
    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;
    Item(Item&&) = delete;
    Item& operator=(Item&&) = delete;
    ItemType getType() const;
    sf::Vector2f getPosition() const;
    bool loadTexture(const std::string& folder = "assets/items");
    void draw(sf::RenderTarget& target) const;
    sf::FloatRect getBounds() const;
    void setSize(float size);
    void collect();
    bool isCollected() const;
private:
    ItemType m_type;
    sf::Vector2f m_position;
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    bool m_loaded = false;
    bool m_collected = false;
    float m_size = 40.f;
};

class ItemManager
{
public:
    explicit ItemManager(unsigned seed = std::random_device{}());
    bool spawn(ItemType type, sf::Vector2f center, float size = 40.f,
               const std::string& folder = "assets/items");
    // 블록 하나가 파괴될 때 딱 한 번 호출. 확률 0~1, 기본 30%.
    bool trySpawn(sf::Vector2f center, float probability = 0.3f, float size = 40.f,
                  const std::string& folder = "assets/items");
    std::optional<ItemType> collectAt(const sf::FloatRect& playerBounds,
                                     bool canCollect = true);
    void draw(sf::RenderTarget& target) const;
    void clear();
    std::size_t getItemCount() const;
private:
    std::vector<std::unique_ptr<Item>> m_items;
    std::mt19937 m_random;
};
