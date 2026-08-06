#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "RoundedRectangleShape.h"
#include "Button.h"
#include "ToolDock.h"
#include "Leaderboard.h"

class GameScreen
{
public:
    // Constructor initializing graphics layout based on window dimensions
    GameScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont);

    // Main draw call for rendering UI components
    void draw(sf::RenderWindow& window);

    // Processes mouse interactions for active tools
    void handleMouseClick(sf::Vector2i mousePos);

    // UI state updater methods
    void setDrawerRole(bool isDrawer, const std::string& drawerName = "Player");
    void setRoundInfo(int currentRound, int totalRounds);
    void setWordHint(const std::string& hint);
    void setTimerValue(int seconds);

    // Accessors for canvas and tool controls
    ToolDock& getToolDock() { return m_toolDock; }
    sf::FloatRect getCanvasBounds() const { return m_canvas.getGlobalBounds(); }

private:
    // Helper function calculating dynamic vertical placement for bottom toolbar
    static float calcToolDockY(sf::Vector2u windowSize);

    // Window dimensions state
    sf::Vector2u m_windowSize;
    bool m_isDrawer;

    // Top header UI elements
    RoundedRectangleShape m_roundBox;
    RoundedRectangleShape m_wordBox;
    RoundedRectangleShape m_timerBox;
    sf::Text m_roundText;
    sf::Text m_wordText;
    sf::Text m_timerText;
    Button m_exitBtn;

    // Main layout panels
    Leaderboard m_leaderboard;
    RoundedRectangleShape m_canvas;
    RoundedRectangleShape m_chatPanel;
    RoundedRectangleShape m_chatInputBox;

    // Non-drawer status text
    sf::Text m_guesserStatusText;

    // Bottom tool dock palette
    ToolDock m_toolDock;
};