#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "RoundedRectangleShape.h"

// Struct for passing player data into the Leaderboard
struct PlayerEntry {
    std::string name;
    int score;
    bool isDrawing;
};

class Leaderboard
{
public:
    Leaderboard(sf::Vector2f size, sf::Vector2f position, sf::Font& font);

    // Update player rank list from network/game state
    void setPlayers(const std::vector<PlayerEntry>& players);
    void draw(sf::RenderWindow& window);

private:
    RoundedRectangleShape m_background;
    sf::Font& m_font;
    std::vector<PlayerEntry> m_players;
};
