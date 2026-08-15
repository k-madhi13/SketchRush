#include "WaitingScreen.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

WaitingScreen::PlayerSlot::PlayerSlot(sf::Font& font, sf::Texture& placeholderTexture)
    : avatarSprite(placeholderTexture)
    , nameText(font, "", 18)
{
    avatarFrame.setFillColor(sf::Color::Transparent);
    avatarFrame.setOutlineThickness(3.f);
    avatarFrame.setOutlineColor(sf::Color(106, 90, 160));
}

WaitingScreen::WaitingScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont, sf::Texture& placeholderAvatarTexture)
    : m_windowSize(windowSize)
    , m_bodyFont(bodyFont)
    , m_placeholderAvatarTexture(placeholderAvatarTexture)
    , m_card({ windowSize.x * 0.45f, windowSize.y * 0.4f }, 25.f)
    , m_title(titleFont, "WAITING FOR PLAYERS", 36)
    , m_statusText(bodyFont, "Waiting for more players to join...", 20)
{
    sf::Color textDark(46, 39, 64);

    // Card, centered on screen
    m_card.setFillColor(sf::Color(255, 255, 255, 170));
    m_card.setOrigin({ m_card.getGlobalBounds().size.x / 2.f, m_card.getGlobalBounds().size.y / 2.f });
    sf::Vector2f cardCenter = { windowSize.x * 0.5f, windowSize.y * 0.5f };
    m_card.setPosition(cardCenter);

    float cardTop = cardCenter.y - (windowSize.y * 0.4f) / 2.f;

    // Title, near the top of the card
    m_title.setFillColor(textDark);
    sf::FloatRect titleBounds = m_title.getLocalBounds();
    m_title.setOrigin({ titleBounds.position.x + titleBounds.size.x / 2.f, titleBounds.position.y + titleBounds.size.y / 2.f });
    m_title.setPosition({ windowSize.x * 0.5f, cardTop + 50.f });

    // Status line, just under the title
    m_statusText.setFillColor(sf::Color(90, 80, 115));
    refreshStatusPosition();

    // Spinner sits in the lower part of the card, below where the player row goes
    m_spinnerCenter = { windowSize.x * 0.5f, cardTop + (windowSize.y * 0.4f) - 55.f };
}

void WaitingScreen::refreshStatusPosition()
{
    sf::FloatRect sb = m_statusText.getLocalBounds();
    m_statusText.setOrigin({ sb.position.x + sb.size.x / 2.f, sb.position.y + sb.size.y / 2.f });
    m_statusText.setPosition({ m_windowSize.x * 0.5f, m_title.getPosition().y + 45.f });
}

void WaitingScreen::setStatusMessage(const std::string& message)
{
    m_statusText.setString(message);
    refreshStatusPosition();
}

void WaitingScreen::setPlayers(const std::vector<WaitingPlayer>& players)
{
    m_players = players;
    if (m_players.size() > m_maxDisplayedPlayers)
        m_players.resize(m_maxDisplayedPlayers);

    m_slots.clear();
    m_slots.reserve(m_players.size());
    for (const WaitingPlayer& p : m_players)
        m_slots.emplace_back(m_bodyFont, m_placeholderAvatarTexture);

    layoutPlayerSlots();
}

// Lays out joined players as a centered row of avatar-circle + name, evenly
// spaced across the card width.
void WaitingScreen::layoutPlayerSlots()
{
    if (m_slots.empty())
        return;

    sf::Color textDark(46, 39, 64);
    float slotSpacing = m_avatarDiameter + 40.f;
    float totalWidth = (m_slots.size() - 1) * slotSpacing;
    float startX = m_windowSize.x * 0.5f - totalWidth / 2.f;
    float rowY = m_statusText.getPosition().y + 55.f;

    for (size_t i = 0; i < m_slots.size(); i++)
    {
        PlayerSlot& slot = m_slots[i];
        const WaitingPlayer& player = m_players[i];
        sf::Vector2f center = { startX + i * slotSpacing, rowY };

        // Avatar image, scaled to fill the fixed-size circle and centered on it
        const sf::Texture* tex = player.avatarTexture ? player.avatarTexture : &m_placeholderAvatarTexture;
        slot.avatarSprite.setTexture(*tex, true);
        sf::Vector2u texSize = tex->getSize();
        if (texSize.x > 0 && texSize.y > 0)
        {
            float scale = m_avatarDiameter / static_cast<float>(std::max(texSize.x, texSize.y));
            slot.avatarSprite.setScale({ scale, scale });
            slot.avatarSprite.setOrigin({ texSize.x / 2.f, texSize.y / 2.f });
        }
        slot.avatarSprite.setPosition(center);

        float ringRadius = m_avatarDiameter / 2.f + 3.f;
        slot.avatarFrame.setRadius(ringRadius);
        slot.avatarFrame.setOrigin({ ringRadius, ringRadius });
        slot.avatarFrame.setPosition(center);

        // Name, centered under the avatar
        slot.nameText.setString(player.name);
        slot.nameText.setFillColor(textDark);
        sf::FloatRect nb = slot.nameText.getLocalBounds();
        slot.nameText.setOrigin({ nb.position.x + nb.size.x / 2.f, nb.position.y + nb.size.y / 2.f });
        slot.nameText.setPosition({ center.x, center.y + m_avatarDiameter / 2.f + 22.f });
    }
}

void WaitingScreen::update(float dt)
{
    m_spinnerAngle += dt * 220.f; // degrees/sec
    if (m_spinnerAngle >= 360.f)
        m_spinnerAngle -= 360.f;
}

// The "waiting sign": a ring of dots that fade out going backwards from the
// current lead angle, giving the classic rotating-loader look without needing
// a drawn arc (SFML has no built-in partial-circle primitive).
void WaitingScreen::drawSpinner(sf::RenderWindow& window)
{
    for (int i = 0; i < m_spinnerDotCount; i++)
    {
        float dotAngleDeg = m_spinnerAngle - i * (360.f / m_spinnerDotCount);
        float rad = dotAngleDeg * static_cast<float>(M_PI) / 180.f;

        sf::Vector2f pos = {
            m_spinnerCenter.x + std::cos(rad) * m_spinnerRadius,
            m_spinnerCenter.y + std::sin(rad) * m_spinnerRadius
        };

        // Fade out the trailing dots so the ring reads as a moving head, not a static ring
        float fade = 1.f - (static_cast<float>(i) / m_spinnerDotCount);
        std::uint8_t alpha = static_cast<std::uint8_t>(60.f + fade * 195.f);

        float dotRadius = 4.5f;
        sf::CircleShape dot(dotRadius);
        dot.setOrigin({ dotRadius, dotRadius });
        dot.setPosition(pos);
        dot.setFillColor(sf::Color(106, 90, 160, alpha));
        window.draw(dot);
    }
}

void WaitingScreen::draw(sf::RenderWindow& window)
{
    window.draw(m_card);
    window.draw(m_title);
    window.draw(m_statusText);

    for (PlayerSlot& slot : m_slots)
    {
        window.draw(slot.avatarFrame);
        window.draw(slot.avatarSprite);
        window.draw(slot.nameText);
    }

    drawSpinner(window);
}
