#include "HomeScreen.h"

HomeScreen::HomeScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont)
    : m_title(titleFont, "SKETCH RUSH", 58)
    , m_card({ windowSize.x * 0.32f, windowSize.y * 0.33f }, 25.f) // slightly taller to fit equal spacing
    , nameInputButton(bodyFont, "Enter Name", { windowSize.x * 0.5f, 0.f },
        { windowSize.x * 0.24f, 55.f }, sf::Color(220, 215, 230, 160), sf::Color::Black, 24)
    , playButton(bodyFont, "Play!", { windowSize.x * 0.5f, 0.f },
        { windowSize.x * 0.24f, 60.f }, sf::Color(106, 90, 160), sf::Color::White, 26)
    , guideButton(bodyFont, "How to Play", { windowSize.x * 0.5f, 0.f },
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
    guideButton.setPosition({ windowSize.x * 0.5f, createY });

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
    guideButton.draw(window);
}

void HomeScreen::update(float dt)
{
    // Blink the text-entry cursor while the name field is focused
    if (m_nameFocused)
    {
        m_cursorBlinkTimer += dt;
        if (m_cursorBlinkTimer >= 0.5f)
        {
            m_cursorBlinkTimer = 0.f;
            m_cursorVisible = !m_cursorVisible;
            refreshNameDisplay();
        }
    }

    // Count down the "please enter a name" warning, then restore the normal display
    if (m_warningTimer > 0.f)
    {
        m_warningTimer -= dt;
        if (m_warningTimer <= 0.f)
        {
            m_warningTimer = 0.f;
            refreshNameDisplay();
        }
    }
}

void HomeScreen::setNameFocused(bool focused)
{
    m_nameFocused = focused;
    m_cursorVisible = true;
    m_cursorBlinkTimer = 0.f;
    refreshNameDisplay();
}

void HomeScreen::handleTextEntered(char32_t unicode)
{
    if (!m_nameFocused)
        return;

    const char32_t backspace = 8;

    if (unicode == backspace)
    {
        if (!m_playerName.empty())
            m_playerName.pop_back();
    }
    // Ignore other control characters (Enter, Tab, Esc, ...) - Enter/Escape are
    // handled separately by main.cpp
    else if (unicode < 32)
    {
        return;
    }
    else if (unicode < 128 && m_playerName.size() < m_maxNameLength)
    {
        // Only accept printable ASCII to keep the name field simple and safe
        m_playerName += static_cast<char>(unicode);
    }

    // Typing clears any active warning immediately
    m_warningTimer = 0.f;
    refreshNameDisplay();
}

bool HomeScreen::tryStartGame()
{
    // Guard against a name that's just whitespace
    bool hasRealName = m_playerName.find_first_not_of(' ') != std::string::npos;

    if (!hasRealName)
    {
        m_warningTimer = 1.5f;
        refreshNameDisplay();
        return false;
    }

    return true;
}

// Rebuilds the name button's label from current state: the warning message,
// the typed name with a blinking cursor while focused, or the placeholder.
void HomeScreen::refreshNameDisplay()
{
    if (m_warningTimer > 0.f)
    {
        nameInputButton.setText("Please enter a name!");
        return;
    }

    std::string display = m_playerName;

    if (m_nameFocused && m_cursorVisible)
        display += "|";

    if (display.empty())
        display = m_nameFocused ? "|" : "Enter Name";

    nameInputButton.setText(display);
}