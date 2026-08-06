#include "Gamescreen.h"
#include "ToolDock.h"

GameScreen::GameScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont)
    : m_windowSize(windowSize)
    , m_isDrawer(true) // Default to drawer mode for active UI testing
    , m_roundBox({ windowSize.x * 0.14f, 48.f }, 12.f)
    , m_wordBox({ windowSize.x * 0.32f, 48.f }, 12.f)
    , m_timerBox({ windowSize.x * 0.08f, 48.f }, 12.f)
    , m_leaderboard({ windowSize.x * 0.18f, windowSize.y * 0.72f }, { windowSize.x * 0.015f, windowSize.y * 0.02f + 65.f }, bodyFont)
    , m_canvas({ windowSize.x * 0.58f, windowSize.y * 0.72f }, 15.f)
    , m_chatPanel({ windowSize.x * 0.18f, windowSize.y * 0.72f }, 15.f)
    , m_chatInputBox({ windowSize.x * 0.16f, 40.f }, 10.f)
    , m_roundText(bodyFont, "Round 1 of 4", 20)
    , m_wordText(bodyFont, "_ _ _ _ _", 24)
    , m_timerText(bodyFont, "60", 24)
    , m_exitBtn(bodyFont, "Exit", { 0,0 }, { 80, 48 }, sf::Color(220, 80, 80), sf::Color::White, 18)
    , m_guesserStatusText(bodyFont, "Ram is drawing...", 24)
    , m_toolDock(
        windowSize.x * 0.205f,
        windowSize.y * 0.74f + 65.f,
        windowSize.x * 0.58f,
        bodyFont)
{
    float margin = windowSize.x * 0.015f;
    float topMargin = windowSize.y * 0.02f;

    // 1. Top Header Setup
    m_roundBox.setFillColor(sf::Color(255, 255, 255, 180));
    m_roundBox.setPosition({ margin, topMargin });

    m_wordBox.setFillColor(sf::Color(255, 255, 255, 220));
    m_wordBox.setPosition({ windowSize.x * 0.28f, topMargin });

    m_timerBox.setFillColor(sf::Color(106, 90, 160));
    m_timerBox.setPosition({ windowSize.x * 0.62f, topMargin });

    m_exitBtn.setPosition({ windowSize.x - margin - 80.f, topMargin });

    // Header Text Position Alignments
    m_roundText.setFillColor(sf::Color(46, 39, 64));
    m_roundText.setPosition({ margin + 15.f, topMargin + 10.f });

    m_wordText.setFillColor(sf::Color(46, 39, 64));
    m_wordText.setPosition({ windowSize.x * 0.40f, topMargin + 8.f });

    m_timerText.setFillColor(sf::Color::White);
    m_timerText.setPosition({ windowSize.x * 0.645f, topMargin + 8.f });

    // 2. Panel Setup (Canvas & Chat)
    float contentY = topMargin + 65.f;

    m_canvas.setFillColor(sf::Color(255, 255, 255, 240));
    m_canvas.setPosition({ margin + windowSize.x * 0.19f, contentY });

    float chatX = margin + windowSize.x * 0.78f;
    m_chatPanel.setFillColor(sf::Color(255, 255, 255, 180));
    m_chatPanel.setPosition({ chatX, contentY });

    m_chatInputBox.setFillColor(sf::Color(255, 255, 255, 240));
    m_chatInputBox.setPosition({ chatX + windowSize.x * 0.01f, contentY + windowSize.y * 0.65f });

    // Guesser Banner Positioning
    m_guesserStatusText.setFillColor(sf::Color(46, 39, 64));
    m_guesserStatusText.setPosition({ windowSize.x * 0.42f, windowSize.y * 0.88f });
}

// Call this to dynamically toggle between Drawer UI and Guesser Banner
void GameScreen::setDrawerRole(bool isDrawer, const std::string& drawerName)
{
    m_isDrawer = isDrawer;
    m_guesserStatusText.setString(drawerName + " is drawing...");
}

// Event delegation
void GameScreen::handleMouseClick(sf::Vector2i mousePos)
{
    if (m_isDrawer)
        m_toolDock.handleMouseClick(mousePos);
}

// Main Render Routine
void GameScreen::draw(sf::RenderWindow& window)
{
    // Draw Top Header
    window.draw(m_roundBox);
    window.draw(m_wordBox);
    window.draw(m_timerBox);
    window.draw(m_roundText);
    window.draw(m_wordText);
    window.draw(m_timerText);
    m_exitBtn.draw(window);

    // Draw Main Gameplay Panels
    m_leaderboard.draw(window);
    window.draw(m_canvas);
    window.draw(m_chatPanel);
    window.draw(m_chatInputBox);

    // Draw Bottom Dock depending on user role
    if (m_isDrawer)
        m_toolDock.draw(window);
    else
        window.draw(m_guesserStatusText);
}