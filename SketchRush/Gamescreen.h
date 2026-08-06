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
    GameScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont);

    void draw(sf::RenderWindow& window);
    void handleMouseClick(sf::Vector2i mousePos);

    // Dynamic State Switchers
    void setDrawerRole(bool isDrawer, const std::string& drawerName = "Player");
    void setRoundInfo(int currentRound, int totalRounds);
    void setWordHint(const std::string& hint);
    void setTimerValue(int seconds);

    // Accessors for teammates
    ToolDock& getToolDock() { return m_toolDock; }
    sf::FloatRect getCanvasBounds() const { return m_canvas.getGlobalBounds(); }

private:
    sf::Vector2u m_windowSize;
    bool m_isDrawer; // Flag controlling Drawer UI vs Guesser Banner

    // Top Bar UI
    RoundedRectangleShape m_roundBox;
    RoundedRectangleShape m_wordBox;
    RoundedRectangleShape m_timerBox;
    sf::Text m_roundText;
    sf::Text m_wordText;
    sf::Text m_timerText;
    Button m_exitBtn;

    // Layout Containers
    Leaderboard m_leaderboard;
    RoundedRectangleShape m_canvas;
    RoundedRectangleShape m_chatPanel;
    RoundedRectangleShape m_chatInputBox;

    // Guesser Status Text ("... is drawing")
    sf::Text m_guesserStatusText;

    // Bottom Tool Sub-Component
    ToolDock m_toolDock;
};