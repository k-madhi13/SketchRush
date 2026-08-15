#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "RoundedRectangleShape.h"

class Button
{
public:
    Button(sf::Font& font, const std::string& label, sf::Vector2f position,
        sf::Vector2f size, sf::Color bgColor, sf::Color textColor, unsigned int fontSize = 28);

    void draw(sf::RenderWindow& window);
    bool isClicked(sf::Vector2i mousePos) const;
    void setPosition(sf::Vector2f position);
    void setText(const std::string& label);

private:
    RoundedRectangleShape m_shape;
    sf::Text m_text;
};