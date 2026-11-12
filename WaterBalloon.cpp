#ifndef WATER_BALLOON_MODULE_CPP_INCLUDED
#define WATER_BALLOON_MODULE_CPP_INCLUDED

// -----------------------------------------------------------------------------
// 물풍선 단일 파일 모듈
//
// 다른 프로젝트에서 사용하는 방법
// 1. 이 파일과 assets/water_balloon_animation 폴더를 복사한다.
// 2. 물풍선 클래스를 사용할 게임 cpp 상단에서 이 파일을 한 번만 include 한다.
// 3. update()에 맵 충돌 검사 함수와 오브젝트 피격 처리 함수를 전달한다.
// 4. 캐릭터 좌표를 hitsPoint()에 전달해 물줄기 피격 여부를 확인한다.
//
// 이 모듈은 특정 Player, Map 클래스에 의존하지 않는다.
// 타일 좌표는 sf::Vector2i, 화면 좌표는 sf::Vector2f만 사용한다.
// -----------------------------------------------------------------------------

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <array>
#include <filesystem>
#include <functional>
#include <vector>

namespace waterballoon_detail
{
// 순서: 오른쪽, 아래, 왼쪽, 위
constexpr std::array<sf::Vector2i, 4> ExplosionOffsets = {{
    {1, 0},
    {0, 1},
    {-1, 0},
    {0, -1},
}};
}

// 설치된 물풍선 하나의 위치, 타이머, 물줄기 길이를 관리한다.
class WaterBalloon
{
public:
    WaterBalloon(sf::Vector2i tile, int explosionRange)
        : m_tile(tile), m_explosionRange(explosionRange)
    {
    }

    void update(float deltaSeconds)
    {
        m_elapsedSeconds += deltaSeconds;
    }

    sf::Vector2i tile() const
    {
        return m_tile;
    }

    int animationFrame() const
    {
        return static_cast<int>(m_elapsedSeconds / SecondsPerFrame) %
               AnimationFrameCount;
    }

    int explosionRange() const
    {
        return m_explosionRange;
    }

    bool isReadyToExplode() const
    {
        return m_elapsedSeconds >= FuseSeconds;
    }

    bool trigger()
    {
        if (isReadyToExplode())
            return false;

        m_elapsedSeconds = FuseSeconds;
        return true;
    }

private:
    static constexpr int AnimationFrameCount = 10;
    static constexpr float SecondsPerFrame = 0.1f; // 초당 10프레임
    static constexpr float FuseSeconds = 3.f;      // 설치 3초 후 폭발

    sf::Vector2i m_tile;
    int m_explosionRange;
    float m_elapsedSeconds = 0.f;
};

// 여러 물풍선과 폭발 효과를 관리한다.
// 맵과 캐릭터는 콜백 및 좌표를 통해서만 연결되므로 다른 프로젝트에 이식하기 쉽다.
class WaterBalloonSystem
{
public:
    static constexpr int DefaultGridSize = 16;

    WaterBalloonSystem(const std::filesystem::path& balloonTexturePath,
                       const std::filesystem::path& centerTexturePath,
                       const std::filesystem::path& bodyTexturePath,
                       const std::filesystem::path& endTexturePath,
                       float tileSize,
                       int gridColumns = DefaultGridSize,
                       int gridRows = DefaultGridSize,
                       sf::Vector2f worldOrigin = {0.f, 0.f})
        : m_tileSize(tileSize),
          m_gridColumns(gridColumns),
          m_gridRows(gridRows),
          m_worldOrigin(worldOrigin),
          m_balloonSprite(m_balloonTexture),
          m_centerSprite(m_centerTexture),
          m_bodySprite(m_bodyTexture),
          m_endSprite(m_endTexture)
    {
        const bool balloonLoaded = m_balloonTexture.loadFromFile(balloonTexturePath);
        const bool centerLoaded = m_centerTexture.loadFromFile(centerTexturePath);
        const bool bodyLoaded = m_bodyTexture.loadFromFile(bodyTexturePath);
        const bool endLoaded = m_endTexture.loadFromFile(endTexturePath);
        m_loaded = balloonLoaded && centerLoaded && bodyLoaded && endLoaded;

        if (!m_loaded)
            return;

        m_balloonTexture.setSmooth(false);
        m_centerTexture.setSmooth(false);
        m_bodyTexture.setSmooth(false);
        m_endTexture.setSmooth(false);

        const sf::Vector2f origin(FrameSize / 2.f, FrameSize / 2.f);
        m_balloonSprite.setOrigin(origin);
        m_centerSprite.setOrigin(origin);
        m_bodySprite.setOrigin(origin);
        m_endSprite.setOrigin(origin);

        // 텍스처를 스프라이트 생성 후 불러왔으므로 표시 영역을 직접 지정한다.
        m_centerSprite.setTextureRect({{0, 0}, {FrameSize, FrameSize}});
        m_bodySprite.setTextureRect({{0, 0}, {FrameSize, FrameSize}});
        m_endSprite.setTextureRect({{0, 0}, {FrameSize, FrameSize}});

        const float effectScale = m_tileSize / static_cast<float>(FrameSize);
        m_balloonSprite.setScale({effectScale * 0.9f, effectScale * 0.9f});
        m_centerSprite.setScale({effectScale, effectScale});
        m_bodySprite.setScale({effectScale, effectScale});
        m_endSprite.setScale({effectScale, effectScale});
        applyBalloonFrame(0);
    }

    bool isReady() const
    {
        return m_loaded;
    }

    // 현재 설치 한도와 동일 타일 중복 여부를 확인한 뒤 물풍선을 설치한다.
    bool place(sf::Vector2i tile)
    {
        if (!isInsideGrid(tile))
            return false;

        if (static_cast<int>(m_balloons.size()) >= m_maxBalloonCount)
            return false;

        if (hasBalloonAt(tile))
            return false;

        // 설치 시점의 물줄기 길이를 각 물풍선에 저장한다.
        m_balloons.emplace_back(tile, m_explosionRange);
        return true;
    }

    bool hasBalloonAt(sf::Vector2i tile) const
    {
        return std::any_of(
            m_balloons.begin(), m_balloons.end(),
            [tile](const WaterBalloon& balloon)
            {
                return balloon.tile() == tile;
            });
    }

    bool hasExplosionAt(sf::Vector2i tile) const
    {
        for (const Explosion& explosion : m_explosions)
        {
            if (explosion.centerTile == tile)
                return true;

            for (std::size_t direction = 0;
                 direction < waterballoon_detail::ExplosionOffsets.size();
                 ++direction)
            {
                for (int distance = 1; distance <= explosion.reach[direction]; ++distance)
                {
                    if (explosion.centerTile +
                            waterballoon_detail::ExplosionOffsets[direction] * distance ==
                        tile)
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    // 현재 물줄기에 닿은 대기 중인 물풍선의 타이머를 즉시 끝낸다.
    // 하나라도 새로 점화했다면 true를 반환한다.
    bool triggerBalloonsHitBy(const WaterBalloonSystem& source)
    {
        bool triggered = false;
        for (WaterBalloon& balloon : m_balloons)
        {
            if (source.hasExplosionAt(balloon.tile()))
                triggered = balloon.trigger() || triggered;
        }
        return triggered;
    }

    // 캐릭터의 화면 좌표가 실제 피격 영역 안에 있는지 반환한다.
    // 피격 영역은 이미지보다 상하좌우 5픽셀씩 작다.
    bool hitsPoint(sf::Vector2f worldPosition) const
    {
        for (const Explosion& explosion : m_explosions)
        {
            if (pointInsideExplosionTile(worldPosition, explosion.centerTile))
                return true;

            for (std::size_t direction = 0;
                 direction < waterballoon_detail::ExplosionOffsets.size();
                 ++direction)
            {
                for (int distance = 1; distance <= explosion.reach[direction]; ++distance)
                {
                    const sf::Vector2i tile =
                        explosion.centerTile +
                        waterballoon_detail::ExplosionOffsets[direction] * distance;

                    if (pointInsideExplosionTile(worldPosition, tile))
                        return true;
                }
            }
        }

        return false;
    }

    void increaseExplosionRange()
    {
        ++m_explosionRange;
    }

    void increaseBalloonCapacity()
    {
        ++m_maxBalloonCount;
    }

    int explosionRange() const
    {
        return m_explosionRange;
    }

    int balloonCapacity() const
    {
        return m_maxBalloonCount;
    }

    // isBlocked(tile): 해당 타일에서 물줄기가 멈춰야 하면 true
    // onExplosionHit(tile): 폭발이 닿은 오브젝트 처리용 콜백
    // 차단 여부를 먼저 저장하므로 파괴 가능 상자를 콜백에서 제거해도
    // 그 타일에는 물줄기 이미지와 캐릭터 피격 판정이 생성되지 않는다.
    int update(float deltaSeconds,
               const std::function<bool(sf::Vector2i)>& isBlocked,
               const std::function<void(sf::Vector2i)>& onExplosionHit)
    {
        if (!m_loaded)
            return 0;

        int explodedBalloonCount = 0;

        for (Explosion& explosion : m_explosions)
            explosion.elapsedSeconds += deltaSeconds;

        m_explosions.erase(
            std::remove_if(m_explosions.begin(), m_explosions.end(),
                           [](const Explosion& explosion)
                           {
                               return explosion.elapsedSeconds >= ExplosionSeconds;
                           }),
            m_explosions.end());

        for (WaterBalloon& balloon : m_balloons)
            balloon.update(deltaSeconds);

        for (const WaterBalloon& balloon : m_balloons)
        {
            if (!balloon.isReadyToExplode())
                continue;

            Explosion explosion;
            explosion.centerTile = balloon.tile();
            onExplosionHit(balloon.tile());

            for (std::size_t direction = 0;
                 direction < waterballoon_detail::ExplosionOffsets.size();
                 ++direction)
            {
                for (int distance = 1; distance <= balloon.explosionRange(); ++distance)
                {
                    const sf::Vector2i targetTile =
                        balloon.tile() +
                        waterballoon_detail::ExplosionOffsets[direction] * distance;

                    if (!isInsideGrid(targetTile))
                        break;

                    // 오브젝트 제거 전에 차단 여부를 먼저 판정한다.
                    const bool blocked = isBlocked(targetTile);
                    onExplosionHit(targetTile);

                    if (blocked)
                        break;

                    explosion.reach[direction] = distance;
                }
            }

            m_explosions.push_back(explosion);
            ++explodedBalloonCount;
        }

        m_balloons.erase(
            std::remove_if(m_balloons.begin(), m_balloons.end(),
                           [](const WaterBalloon& balloon)
                           {
                               return balloon.isReadyToExplode();
                           }),
            m_balloons.end());

        return explodedBalloonCount;
    }

    void drawBalloons(sf::RenderTarget& target)
    {
        if (!m_loaded)
            return;

        for (const WaterBalloon& balloon : m_balloons)
        {
            applyBalloonFrame(balloon.animationFrame());
            m_balloonSprite.setPosition(tileCenter(balloon.tile()));
            target.draw(m_balloonSprite);
        }
    }

    void drawExplosions(sf::RenderTarget& target)
    {
        if (!m_loaded)
            return;

        for (const Explosion& explosion : m_explosions)
            drawExplosion(target, explosion);
    }

private:
    static constexpr int FrameSize = 128;
    static constexpr int FramesPerRow = 5;
    static constexpr float ExplosionSeconds = 0.5f;
    static constexpr float HitboxInset = 5.f;

    struct Explosion
    {
        sf::Vector2i centerTile;
        float elapsedSeconds = 0.f;
        std::array<int, 4> reach{};
    };

    void applyBalloonFrame(int frame)
    {
        const int column = frame % FramesPerRow;
        const int row = frame / FramesPerRow;
        m_balloonSprite.setTextureRect({{column * FrameSize, row * FrameSize},
                                        {FrameSize, FrameSize}});
    }

    void drawExplosion(sf::RenderTarget& target, const Explosion& explosion)
    {
        m_centerSprite.setPosition(tileCenter(explosion.centerTile));
        target.draw(m_centerSprite);

        struct Direction
        {
            sf::Vector2i offset;
            sf::Angle rotation;
        };

        // water_end.png와 water_body.png의 원본 방향은 오른쪽이다.
        const std::array<Direction, 4> directions = {{
            {waterballoon_detail::ExplosionOffsets[0], sf::degrees(0.f)},
            {waterballoon_detail::ExplosionOffsets[1], sf::degrees(90.f)},
            {waterballoon_detail::ExplosionOffsets[2], sf::degrees(180.f)},
            {waterballoon_detail::ExplosionOffsets[3], sf::degrees(270.f)},
        }};

        for (std::size_t direction = 0; direction < directions.size(); ++direction)
        {
            for (int distance = 1; distance <= explosion.reach[direction]; ++distance)
            {
                const sf::Vector2i tile =
                    explosion.centerTile + directions[direction].offset * distance;

                if (distance == explosion.reach[direction])
                {
                    m_endSprite.setRotation(directions[direction].rotation);
                    m_endSprite.setPosition(tileCenter(tile));
                    target.draw(m_endSprite);
                }
                else
                {
                    m_bodySprite.setRotation(directions[direction].rotation);
                    m_bodySprite.setPosition(tileCenter(tile));
                    target.draw(m_bodySprite);
                }
            }
        }
    }

    sf::Vector2f tileCenter(sf::Vector2i tile) const
    {
        return {m_worldOrigin.x + (static_cast<float>(tile.x) + 0.5f) * m_tileSize,
                m_worldOrigin.y + (static_cast<float>(tile.y) + 0.5f) * m_tileSize};
    }

    bool pointInsideExplosionTile(sf::Vector2f point, sf::Vector2i tile) const
    {
        const sf::Vector2f center = tileCenter(tile);
        const float halfExtent = m_tileSize * 0.5f - HitboxInset;

        return point.x >= center.x - halfExtent &&
               point.x <= center.x + halfExtent &&
               point.y >= center.y - halfExtent &&
               point.y <= center.y + halfExtent;
    }

    bool isInsideGrid(sf::Vector2i tile) const
    {
        return tile.x >= 0 && tile.y >= 0 &&
               tile.x < m_gridColumns && tile.y < m_gridRows;
    }

    float m_tileSize;
    int m_gridColumns;
    int m_gridRows;
    sf::Vector2f m_worldOrigin;
    int m_explosionRange = 1;
    int m_maxBalloonCount = 1;
    bool m_loaded = false;
    std::vector<WaterBalloon> m_balloons;
    std::vector<Explosion> m_explosions;

    sf::Texture m_balloonTexture;
    sf::Texture m_centerTexture;
    sf::Texture m_bodyTexture;
    sf::Texture m_endTexture;
    sf::Sprite m_balloonSprite;
    sf::Sprite m_centerSprite;
    sf::Sprite m_bodySprite;
    sf::Sprite m_endSprite;
};

#endif // WATER_BALLOON_MODULE_CPP_INCLUDED
