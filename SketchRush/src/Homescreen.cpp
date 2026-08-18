#include "HomeScreen.h"

HomeScreen::HomeScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont, Client& client)
    : m_title(titleFont, "SKETCH RUSH", 58)
    , m_card({ windowSize.x * 0.32f, windowSize.y * 0.33f }, 25.f)
    , m_client(client)

    , nameInputButton(
        bodyFont,
        "Enter Name",
        { windowSize.x * 0.5f, 0.f },
        { windowSize.x * 0.24f, 55.f },
        sf::Color(220, 215, 230, 160),
        sf::Color::Black,
        24
    )

    , serverIpInputButton(
        bodyFont,
        "Server IP",
        { windowSize.x * 0.5f, 0.f },
        { windowSize.x * 0.24f, 55.f },
        sf::Color(220, 215, 230, 160),
        sf::Color::Black,
        24
    )

    , playButton(
        bodyFont,
        "Play!",
        { windowSize.x * 0.5f, 0.f },
        { windowSize.x * 0.24f, 60.f },
        sf::Color(106, 90, 160),
        sf::Color::White,
        26
    )

    , guideButton(
        bodyFont,
        "How to Play",
        { windowSize.x * 0.5f, 0.f },
        { windowSize.x * 0.24f, 60.f },
        sf::Color(200, 190, 225),
        sf::Color(60, 50, 90),
        24
    )
{
    sf::Color textDark(46, 39, 64);

    // Card
    m_card.setFillColor(sf::Color(255, 255, 255, 140));

    m_card.setOrigin({
        m_card.getGlobalBounds().size.x / 2.f,
        m_card.getGlobalBounds().size.y / 2.f
    });

    sf::Vector2f cardCenter = {
        windowSize.x * 0.5f,
        windowSize.y * 0.5f
    };

    m_card.setPosition(cardCenter);

    // ---------------------------------------------------------
    // Equal spacing calculation
    // ---------------------------------------------------------

    float cardHeight = windowSize.y * 0.33f;
    float cardTop = cardCenter.y - cardHeight / 2.f;

    float nameBoxHeight = 55.f;
    float ipBoxHeight = 55.f;
    float playHeight = 60.f;
    float guideHeight = 60.f;

    float totalButtonsHeight =
        nameBoxHeight +
        ipBoxHeight +
        playHeight +
        guideHeight;

    // Five gaps:
    //
    // top
    // name -> IP
    // IP -> Play
    // Play -> Guide
    // bottom
    //
    float remainingSpace = cardHeight - totalButtonsHeight;
    float gap = remainingSpace / 5.f;

    float nameBoxY =
        cardTop +
        gap +
        nameBoxHeight / 2.f;

    float ipBoxY =
        nameBoxY +
        nameBoxHeight / 2.f +
        gap +
        ipBoxHeight / 2.f;

    float playY =
        ipBoxY +
        ipBoxHeight / 2.f +
        gap +
        playHeight / 2.f;

    float guideY =
        playY +
        playHeight / 2.f +
        gap +
        guideHeight / 2.f;

    nameInputButton.setPosition({
        windowSize.x * 0.5f,
        nameBoxY
    });

    serverIpInputButton.setPosition({
        windowSize.x * 0.5f,
        ipBoxY
    });

    playButton.setPosition({
        windowSize.x * 0.5f,
        playY
    });

    guideButton.setPosition({
        windowSize.x * 0.5f,
        guideY
    });

    // ---------------------------------------------------------
    // Title
    // ---------------------------------------------------------

    m_title.setFillColor(textDark);

    sf::FloatRect titleBounds = m_title.getLocalBounds();

    m_title.setOrigin({
        titleBounds.position.x + titleBounds.size.x / 2.f,
        titleBounds.position.y + titleBounds.size.y / 2.f
    });

    m_title.setPosition({
        windowSize.x * 0.5f,
        windowSize.y * 0.18f
    });
}

void HomeScreen::draw(sf::RenderWindow& window)
{
    window.draw(m_card);
    window.draw(m_title);

    nameInputButton.draw(window);
    serverIpInputButton.draw(window);
    playButton.draw(window);
    guideButton.draw(window);
}

void HomeScreen::update(float dt)
{
    // Blink cursor while either input is focused
    if (m_nameFocused || m_ipFocused)
    {
        m_cursorBlinkTimer += dt;

        if (m_cursorBlinkTimer >= 0.5f)
        {
            m_cursorBlinkTimer = 0.f;
            m_cursorVisible = !m_cursorVisible;

            refreshNameDisplay();
            refreshIpDisplay();
        }
    }

    // Count down warning
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

// -------------------------------------------------------------
// NAME INPUT
// -------------------------------------------------------------

void HomeScreen::setNameFocused(bool focused)
{
    m_nameFocused = focused;

    if (focused)
    {
        // Only one input can be focused at a time
        m_ipFocused = false;
    }

    m_cursorVisible = true;
    m_cursorBlinkTimer = 0.f;

    refreshNameDisplay();
    refreshIpDisplay();
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
    else if (unicode < 32)
    {
        // Ignore control characters
        return;
    }
    else if (unicode < 128 && m_playerName.size() < m_maxNameLength)
    {
        // Only accept printable ASCII
        m_playerName += static_cast<char>(unicode);
    }

    // Typing clears warning
    m_warningTimer = 0.f;

    refreshNameDisplay();
}

// -------------------------------------------------------------
// SERVER IP INPUT
// -------------------------------------------------------------

void HomeScreen::setIpFocused(bool focused)
{
    m_ipFocused = focused;

    if (focused)
    {
        // Only one input can be focused at a time
        m_nameFocused = false;
    }

    m_cursorVisible = true;
    m_cursorBlinkTimer = 0.f;

    refreshNameDisplay();
    refreshIpDisplay();
}

void HomeScreen::handleIpTextEntered(char32_t unicode)
{
    if (!m_ipFocused)
        return;

    const char32_t backspace = 8;

    if (unicode == backspace)
    {
        if (!m_serverIp.empty())
            m_serverIp.pop_back();
    }
    else if (unicode < 32)
    {
        // Ignore control characters
        return;
    }
    else if (unicode < 128 && m_serverIp.size() < m_maxIpLength)
    {
        // Server IP only accepts numbers and '.'
        if ((unicode >= '0' && unicode <= '9') || unicode == '.')
        {
            m_serverIp += static_cast<char>(unicode);
        }
    }

    refreshIpDisplay();
}

// -------------------------------------------------------------
// START GAME
// -------------------------------------------------------------

bool HomeScreen::tryStartGame()
{
    // Guard against a name that's just whitespace
    bool hasRealName =
        m_playerName.find_first_not_of(' ') != std::string::npos;

    auto resolvedIpAddress = sf::IpAddress::resolve(m_serverIp);
    
    if(!resolvedIpAddress){
        return false;
    }

    bool serverConnectionError = m_client.Connect(*resolvedIpAddress,m_playerName);

    if (!hasRealName or serverConnectionError)
    {
        m_warningTimer = 1.5f;
        refreshNameDisplay();
        return false;
    }

    return true;
}

// -------------------------------------------------------------
// DISPLAY
// -------------------------------------------------------------

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

void HomeScreen::refreshIpDisplay()
{
    std::string display = m_serverIp;

    if (m_ipFocused && m_cursorVisible)
        display += "|";

    if (display.empty())
        display = m_ipFocused ? "|" : "Server IP";

    serverIpInputButton.setText(display);
}