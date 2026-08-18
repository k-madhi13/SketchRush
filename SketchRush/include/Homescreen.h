#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Button.h"
#include "RoundedRectangleShape.h"
#include "Client.h"

class HomeScreen
{
public:
    HomeScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont, Client& client);

    void draw(sf::RenderWindow& window);
    void update(float dt);

    // Name input
    void setNameFocused(bool focused);
    void handleTextEntered(char32_t unicode);

    // Server IP input
    void setIpFocused(bool focused);
    void handleIpTextEntered(char32_t unicode);

    bool tryStartGame();

    const std::string& getPlayerName() const { return m_playerName; }
    const std::string& getServerIp() const { return m_serverIp; }

    bool isNameFocused() const { return m_nameFocused; }
    bool isIpFocused() const { return m_ipFocused; }

    Button playButton;
    Button guideButton;
    Button nameInputButton;
    Button serverIpInputButton;

private:
    void refreshNameDisplay();
    void refreshIpDisplay();

    Client& m_client;

    RoundedRectangleShape m_card;
    sf::Text m_title;

    std::string m_playerName;
    std::string m_serverIp;

    bool m_nameFocused = false;
    bool m_ipFocused = false;

    bool m_cursorVisible = true;
    float m_cursorBlinkTimer = 0.f;
    float m_warningTimer = 0.f;

    static constexpr unsigned int m_maxNameLength = 8;
    static constexpr unsigned int m_maxIpLength = 15;
};