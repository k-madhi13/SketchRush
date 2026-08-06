#include "Button.h"

Button::Button(sf::Font& font, const std::string& label, sf::Vector2f position,
    sf::Vector2f size, sf::Color bgColor, sf::Color textColor, unsigned int fontSize)
    : m_shape(size, 15.f)
    , m_text(font, label, fontSize)
{
    m_shape.setFillColor(bgColor);
    m_shape.setOrigin({ size.x / 2.f, size.y / 2.f });
    m_shape.setPosition(position);

    m_text.setFillColor(textColor);
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin({
        textBounds.position.x + textBounds.size.x / 2.f,
        textBounds.position.y + textBounds.size.y / 2.f
        });
    m_text.setPosition(position);
}

void Button::draw(sf::RenderWindow& window)
{
    window.draw(m_shape);
    window.draw(m_text);
}

bool Button::isClicked(sf::Vector2i mousePos) const
{
    return m_shape.getGlobalBounds().contains(sf::Vector2f(mousePos));
}
void Button::setPosition(sf::Vector2f position)
{
    m_shape.setPosition(position);
    m_text.setPosition(position);
}