#include "HomeScreen.h"

HomeScreen::HomeScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont)
    : m_title(titleFont, "SKETCH RUSH", 58)
    , m_card({ windowSize.x * 0.32f, windowSize.y * 0.33f }, 25.f) // slightly taller to fit equal spacing
    , nameInputButton(bodyFont, "Enter Name", { windowSize.x * 0.5f, 0.f }, // Y set below
        { windowSize.x * 0.24f, 55.f }, sf::Color(255,255,255,0), sf::Color::Black, 24)
    , playButton(bodyFont, "Play!", { windowSize.x * 0.5f, 0.f },
        { windowSize.x * 0.24f, 60.f }, sf::Color(106, 90, 160), sf::Color::White, 26)
    , createRoomButton(bodyFont, "Create Room", { windowSize.x * 0.5f, 0.f },
        { windowSize.x * 0.24f, 60.f }, sf::Color(200, 190, 225), sf::Color(60, 50, 90), 24)
{
    sf::Color textDark(46, 39, 64);


    // Card
    m_card.setFillColor(sf::Color(255, 255, 255, 140));
    m_card.setOrigin({ m_card.getGlobalBounds().size.x / 2.f, m_card.getGlobalBounds().size.y / 2.f });
    sf::Vector2f cardCenter = { windowSize.x * 0.5f, windowSize.y * 0.5f };
    m_card.setPosition(cardCenter);

    // ---- Equal spacing calculation ----
    float cardHeight = windowSize.y * 0.33f;
    float cardTop = cardCenter.y - cardHeight / 2.f;

    float nameBoxHeight = 55.f;
    float playHeight = 60.f;
    float createHeight = 60.f;

    // Total content height + we want 4 equal gaps: top-margin, gap1, gap2, bottom-margin
    float totalButtonsHeight = nameBoxHeight + playHeight + createHeight;
    float remainingSpace = cardHeight - totalButtonsHeight;
    float gap = remainingSpace / 4.f; // 4 gaps: top, between1, between2, bottom

    float nameBoxY = cardTop + gap + nameBoxHeight / 2.f;
    float playY = nameBoxY + nameBoxHeight / 2.f + gap + playHeight / 2.f;
    float createY = playY + playHeight / 2.f + gap + createHeight / 2.f;

    nameInputButton.setPosition({ windowSize.x * 0.5f, nameBoxY });
    playButton.setPosition({ windowSize.x * 0.5f, playY });
    createRoomButton.setPosition({ windowSize.x * 0.5f, createY });

    // Title
    m_title.setFillColor(textDark);
    sf::FloatRect titleBounds = m_title.getLocalBounds();
    m_title.setOrigin({ titleBounds.position.x + titleBounds.size.x / 2.f, titleBounds.position.y + titleBounds.size.y / 2.f });
    m_title.setPosition({ windowSize.x * 0.5f, windowSize.y * 0.18f });
}

void HomeScreen::draw(sf::RenderWindow& window)
{
    window.draw(m_card);
    window.draw(m_title);
    nameInputButton.draw(window);
    playButton.draw(window);
    createRoomButton.draw(window);
}