#pragma once
#include <SFML/Graphics.hpp>
#include "Button.h"
#include "RoundedRectangleShape.h"

class HomeScreen
{
public:
    HomeScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont);
    void draw(sf::RenderWindow& window);

    Button playButton;
    Button createRoomButton;
    Button nameInputButton; // acts as a clickable field, no visible border box

private:
    RoundedRectangleShape m_card;
    sf::Text m_title;
};