#include "Leaderboard.h"

Leaderboard::Leaderboard(sf::Vector2f size, sf::Vector2f position, sf::Font& font)
    : m_background(size, 15.f)
    , m_font(font)
{
    m_background.setFillColor(sf::Color(255, 255, 255, 160));
    m_background.setPosition(position);
}

void Leaderboard::setPlayers(const std::vector<PlayerEntry>& players)
{
    m_players = players;
}

// Renders player rankings inside left sidebar panel
void Leaderboard::draw(sf::RenderWindow& window)
{
    window.draw(m_background);

    sf::Vector2f pos = m_background.getPosition();
    float rowHeight = 45.f;

    for (size_t i = 0; i < m_players.size(); i++)
    {
        // Player Rank & Name
        sf::Text playerText(m_font, "#" + std::to_string(i + 1) + " " + m_players[i].name, 18);
        playerText.setFillColor(sf::Color(46, 39, 64));
        playerText.setPosition({ pos.x + 15.f, pos.y + 15.f + i * rowHeight });

        // Player Score
        sf::Text scoreText(m_font, std::to_string(m_players[i].score) + " pts", 16);
        scoreText.setFillColor(sf::Color(100, 90, 130));
        scoreText.setPosition({ pos.x + 15.f, pos.y + 34.f + i * rowHeight });

        window.draw(playerText);
        window.draw(scoreText);
    }
}