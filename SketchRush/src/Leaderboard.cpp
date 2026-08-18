#include "Leaderboard.h"
#include <algorithm>

Leaderboard::Leaderboard(sf::Vector2f size, sf::Vector2f position, sf::Font& font, sf::Texture& placeholderAvatarTexture)
    : m_background(size, 15.f)
    , m_font(font)
    , m_size(size)
    , m_localAvatarSprite(placeholderAvatarTexture)
    , m_localNameText(font, "", 16)
    , m_localScoreText(font, "", 15)
{
    m_background.setFillColor(sf::Color(255, 255, 255, 160));
    m_background.setPosition(position);

    m_localSectionHeight = size.y * m_localSectionRatio;

    m_localNameText.setFillColor(sf::Color(46, 39, 64));
    m_localScoreText.setFillColor(sf::Color(106, 90, 160));

    m_localAvatarFrame.setFillColor(sf::Color::Transparent);
    m_localAvatarFrame.setOutlineThickness(2.f);
    m_localAvatarFrame.setOutlineColor(sf::Color(106, 90, 160));

    layoutLocalSection();
}

void Leaderboard::layoutLocalSection()
{
    sf::Vector2f pos = m_background.getPosition();

    // Avatar circle sized to fit comfortably within the reserved 10% strip
    m_avatarDiameter = std::min(m_localSectionHeight - 12.f, 54.f);
    float avatarX = pos.x + 14.f;
    float avatarY = pos.y + (m_localSectionHeight - m_avatarDiameter) / 2.f;
    m_avatarCenter = { avatarX + m_avatarDiameter / 2.f, avatarY + m_avatarDiameter / 2.f };

    float ringRadius = m_avatarDiameter / 2.f + 2.f;
    m_localAvatarFrame.setRadius(ringRadius);
    m_localAvatarFrame.setOrigin({ ringRadius, ringRadius });
    m_localAvatarFrame.setPosition(m_avatarCenter);

    float textX = avatarX + m_avatarDiameter + 12.f;
    m_localNameText.setPosition({ textX, avatarY + 2.f });
    m_localScoreText.setPosition({ textX, avatarY + m_avatarDiameter / 2.f + 6.f });
}

void Leaderboard::setPlayers(const std::vector<PlayerEntry>& players)
{
    m_players = players;
}

// Updates the reserved top strip: real name + randomized avatar during gameplay,
// or a fixed placeholder ("[NAME]" + silhouette) while in guide mode
void Leaderboard::setLocalPlayer(const std::string& name, int score, const sf::Texture* avatarTexture)
{
    m_localNameText.setString(name);
    m_localScoreText.setString(std::to_string(score) + " pts");

    if (avatarTexture)
    {
        m_localAvatarSprite.setTexture(*avatarTexture, true);

        // Rescale so the avatar (whatever its native resolution) fills the
        // fixed-size circle reserved for it, and center it there
        sf::Vector2u texSize = avatarTexture->getSize();
        if (texSize.x > 0 && texSize.y > 0)
        {
            float scale = m_avatarDiameter / static_cast<float>(std::max(texSize.x, texSize.y));
            m_localAvatarSprite.setScale({ scale, scale });
            m_localAvatarSprite.setOrigin({ texSize.x / 2.f, texSize.y / 2.f });
            m_localAvatarSprite.setPosition(m_avatarCenter);
        }
    }
}

// Renders player rankings inside left sidebar panel
void Leaderboard::draw(sf::RenderWindow& window)
{
    window.draw(m_background);

    sf::Vector2f pos = m_background.getPosition();

    // Reserved top strip: local player's avatar, name, and score
    window.draw(m_localAvatarFrame);
    window.draw(m_localAvatarSprite);
    window.draw(m_localNameText);
    window.draw(m_localScoreText);

    // Divider between the personal-info strip and the ranked player list below it
    sf::RectangleShape divider({ m_size.x - 24.f, 1.5f });
    divider.setFillColor(sf::Color(200, 195, 210, 180));
    divider.setPosition({ pos.x + 12.f, pos.y + m_localSectionHeight });
    window.draw(divider);

    float rowHeight = 45.f;
    float listStartY = pos.y + m_localSectionHeight + 12.f;

    for (size_t i = 0; i < m_players.size(); i++)
    {
        // Player Rank & Name
        sf::Text playerText(m_font, "#" + std::to_string(i + 1) + " " + m_players[i].name, 18);
        playerText.setFillColor(sf::Color(46, 39, 64));
        playerText.setPosition({ pos.x + 15.f, listStartY + i * rowHeight });

        // Player Score
        sf::Text scoreText(m_font, std::to_string(m_players[i].score) + " pts", 16);
        scoreText.setFillColor(sf::Color(100, 90, 130));
        scoreText.setPosition({ pos.x + 15.f, listStartY + 19.f + i * rowHeight });

        window.draw(playerText);
        window.draw(scoreText);
    }
}