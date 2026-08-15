#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Button.h"
#include "RoundedRectangleShape.h"

class HomeScreen
{
public:
    HomeScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont);
    void draw(sf::RenderWindow& window);
    void update(float dt);

    void setNameFocused(bool focused);
    void handleTextEntered(char32_t unicode);
    bool tryStartGame(); // false + shows a warning if the name field is empty

    const std::string& getPlayerName() const { return m_playerName; }
    bool isNameFocused() const { return m_nameFocused; }

    Button playButton;
    Button guideButton;
    Button nameInputButton;

private:
    void refreshNameDisplay();

    RoundedRectangleShape m_card;
    sf::Text m_title;

    std::string m_playerName;
    bool m_nameFocused = false;
    bool m_cursorVisible = true;
    float m_cursorBlinkTimer = 0.f;
    float m_warningTimer = 0.f;
    static constexpr unsigned int m_maxNameLength = 8;
};