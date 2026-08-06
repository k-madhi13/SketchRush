#include "Gamescreen.h"
#include "ToolDock.h"

// Calculates ToolDock Y-position by centering it in the remaining bottom space
float GameScreen::calcToolDockY(sf::Vector2u windowSize)
{
    float headerHeight = 48.f;
    float toolDockHeight = 80.f;
    float panelsHeight = windowSize.y * 0.68f;

    // Uniform margin calculation for upper sections
    float M_vertical = (windowSize.y - headerHeight - toolDockHeight - panelsHeight) / 4.f;

    // Calculate canvas bottom Y-coordinate
    float panelsY = M_vertical + headerHeight + M_vertical;
    float canvasBottom = panelsY + panelsHeight;

    // Center the ToolDock in space remaining below the canvas
    float remainingBottomSpace = windowSize.y - canvasBottom;
    return canvasBottom + (remainingBottomSpace - toolDockHeight) / 2.f;
}

GameScreen::GameScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont)
    : m_windowSize(windowSize)
    , m_isDrawer(true)
    , m_roundBox({ windowSize.x * 0.18f, 48.f }, 12.f)
    , m_wordBox({ windowSize.x * 0.58f * 0.5f, 48.f }, 12.f)
    , m_timerBox({ windowSize.x * 0.18f * 0.45f, 48.f }, 12.f)
    , m_roundText(bodyFont, "Round 1 of 4", 20)
    , m_wordText(bodyFont, "_ _ _ _ _", 24)
    , m_timerText(bodyFont, "60", 24)
    , m_exitBtn(bodyFont, "Exit", { 0,0 }, { windowSize.x * 0.18f * 0.45f, 48.f }, sf::Color(220, 80, 80), sf::Color::White, 18)
    , m_leaderboard({ windowSize.x * 0.18f, windowSize.y * 0.68f },
        { (windowSize.x - (windowSize.x * 0.18f + windowSize.x * 0.58f + windowSize.x * 0.18f)) / 4.f,
          (windowSize.y - 48.f - 80.f - windowSize.y * 0.68f) / 4.f + 48.f + (windowSize.y - 48.f - 80.f - windowSize.y * 0.68f) / 4.f },
        bodyFont)
    , m_canvas({ windowSize.x * 0.58f, windowSize.y * 0.68f }, 15.f)
    , m_chatPanel({ windowSize.x * 0.18f, windowSize.y * 0.68f }, 15.f)
    , m_chatInputBox({ windowSize.x * 0.16f, 40.f }, 10.f)
    , m_guesserStatusText(bodyFont, "Ram is drawing...", 24)
    , m_toolDock(
        (windowSize.x - (windowSize.x * 0.18f + windowSize.x * 0.58f + windowSize.x * 0.18f)) / 4.f + windowSize.x * 0.18f + (windowSize.x - (windowSize.x * 0.18f + windowSize.x * 0.58f + windowSize.x * 0.18f)) / 4.f,
        calcToolDockY(windowSize),
        windowSize.x * 0.58f,
        bodyFont)
{
    // Width definitions
    float leaderboardWidth = windowSize.x * 0.18f;
    float canvasWidth = windowSize.x * 0.58f;
    float chatWidth = windowSize.x * 0.18f;

    // Height definitions
    float headerHeight = 48.f;
    float toolDockHeight = 80.f;
    float panelsHeight = windowSize.y * 0.68f;

    // Symmetric horizontal spacing calculation
    float totalPanelsWidth = leaderboardWidth + canvasWidth + chatWidth;
    float M_horizontal = (windowSize.x - totalPanelsWidth) / 4.f;

    // Symmetric upper vertical spacing calculation
    float M_vertical = (windowSize.y - headerHeight - toolDockHeight - panelsHeight) / 4.f;

    // Top Header and Panels Y positions
    float headerY = M_vertical;
    float panelsY = headerY + headerHeight + M_vertical;

    // Centered Tool Dock Y calculation
    float canvasBottom = panelsY + panelsHeight;
    float remainingBottomSpace = windowSize.y - canvasBottom;
    float toolDockY = canvasBottom + (remainingBottomSpace - toolDockHeight) / 2.f;

    // Horizontal X positions
    float leaderboardX = M_horizontal;
    float canvasX = leaderboardX + leaderboardWidth + M_horizontal;
    float chatX = canvasX + canvasWidth + M_horizontal;

    // 1. Header controls setup
    m_roundBox.setFillColor(sf::Color(255, 255, 255, 180));
    m_roundBox.setPosition({ leaderboardX, headerY });

    float wordBoxWidth = canvasWidth * 0.5f;
    m_wordBox.setFillColor(sf::Color(255, 255, 255, 220));
    m_wordBox.setPosition({ canvasX + (canvasWidth - wordBoxWidth) / 2.f, headerY });

    float timerWidth = chatWidth * 0.45f;
    m_timerBox.setFillColor(sf::Color(106, 90, 160));
    m_timerBox.setPosition({ chatX, headerY });

    m_exitBtn.setPosition({ chatX + chatWidth - timerWidth / 2.f, headerY + headerHeight / 2.f });

    // Header text alignment
    m_roundText.setFillColor(sf::Color(46, 39, 64));
    m_roundText.setPosition({ leaderboardX + 15.f, headerY + 12.f });

    m_wordText.setFillColor(sf::Color(46, 39, 64));
    sf::FloatRect wb = m_wordText.getLocalBounds();
    m_wordText.setPosition({ canvasX + canvasWidth / 2.f - wb.size.x / 2.f - wb.position.x, headerY + 10.f });

    m_timerText.setFillColor(sf::Color::White);
    sf::FloatRect tb = m_timerText.getLocalBounds();
    m_timerText.setPosition({ chatX + timerWidth / 2.f - tb.size.x / 2.f - tb.position.x, headerY + 10.f });

    // 2. Main panels setup
    m_canvas.setFillColor(sf::Color(255, 255, 255, 240));
    m_canvas.setPosition({ canvasX, panelsY });

    m_chatPanel.setFillColor(sf::Color(255, 255, 255, 180));
    m_chatPanel.setPosition({ chatX, panelsY });

    m_chatInputBox.setFillColor(sf::Color(255, 255, 255, 240));
    m_chatInputBox.setPosition({ chatX + (chatWidth - windowSize.x * 0.16f) / 2.f, panelsY + panelsHeight - 55.f });

    // 3. Guesser text centered in tool dock space
    m_guesserStatusText.setFillColor(sf::Color(46, 39, 64));
    sf::FloatRect gb = m_guesserStatusText.getLocalBounds();
    m_guesserStatusText.setPosition({ canvasX + canvasWidth / 2.f - gb.size.x / 2.f - gb.position.x, toolDockY + (toolDockHeight / 2.f) - (gb.size.y / 2.f) });
}

void GameScreen::setDrawerRole(bool isDrawer, const std::string& drawerName)
{
    // Update role state and player text
    m_isDrawer = isDrawer;
    m_guesserStatusText.setString(drawerName + " is drawing...");
}

void GameScreen::setRoundInfo(int currentRound, int totalRounds)
{
    // Update round text string
    m_roundText.setString("Round " + std::to_string(currentRound) + " of " + std::to_string(totalRounds));
}

void GameScreen::setWordHint(const std::string& hint)
{
    // Update word display string
    m_wordText.setString(hint);
}

void GameScreen::setTimerValue(int seconds)
{
    // Update timer value string
    m_timerText.setString(std::to_string(seconds));
}

void GameScreen::handleMouseClick(sf::Vector2i mousePos)
{
    // Route clicks to dock if active drawer
    if (m_isDrawer)
        m_toolDock.handleMouseClick(mousePos);
}

void GameScreen::draw(sf::RenderWindow& window)
{
    // Render top header
    window.draw(m_roundBox);
    window.draw(m_wordBox);
    window.draw(m_timerBox);
    window.draw(m_roundText);
    window.draw(m_wordText);
    window.draw(m_timerText);
    m_exitBtn.draw(window);

    // Render middle workspace
    m_leaderboard.draw(window);
    window.draw(m_canvas);
    window.draw(m_chatPanel);
    window.draw(m_chatInputBox);

    // Render bottom drawer controls or guesser message
    if (m_isDrawer)
        m_toolDock.draw(window);
    else
        window.draw(m_guesserStatusText);
}