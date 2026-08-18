#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "RoundedRectangleShape.h"

// One joined player shown on the waiting screen.
struct WaitingPlayer
{
    std::string name;
    const sf::Texture* avatarTexture = nullptr; // nullptr -> placeholder avatar is used
};

// Passive "waiting for players" screen shown before a match starts.
//
// This class only draws UI - it has no idea how many players are required,
// when enough players have joined, or how players actually connect over LAN.
// Whoever wires up the lobby/networking logic should:
//   - call setPlayers(...) each time the joined-player list changes
//   - call setStatusMessage(...) to update the line under the title (optional)
//   - call update(dt) once per frame so the waiting spinner animates
//   - call draw(window) while this screen should be visible
//
// NOTE: this screen is intentionally NOT wired into main.cpp's AppState/game
// loop yet - it's a standalone file so it can't affect the current Home/Game/
// Guide flow. Hook it up once the lobby logic is ready.
class WaitingScreen
{
public:
    WaitingScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont, sf::Texture& placeholderAvatarTexture);

    // Advances the spinner animation - call once per frame with the frame's delta time
    void update(float dt);

    void draw(sf::RenderWindow& window);

    // Full replace of the joined-players list (name + optional avatar each).
    // Capped internally so an unexpectedly large list won't break the layout.
    void setPlayers(const std::vector<WaitingPlayer>& players);

    // Optional line under the title, e.g. "2 of 4 players joined".
    // Left at a generic default if this is never called.
    void setStatusMessage(const std::string& message);

private:
    // One avatar + name "slot" per joined player, laid out in a row
    struct PlayerSlot
    {
        PlayerSlot(sf::Font& font, sf::Texture& placeholderTexture);

        sf::Sprite avatarSprite;
        sf::CircleShape avatarFrame; // decorative ring around the avatar
        sf::Text nameText;
    };

    void layoutPlayerSlots(); // recomputes avatar/name positions after setPlayers()
    void refreshStatusPosition();
    void drawSpinner(sf::RenderWindow& window);

    sf::Vector2u m_windowSize;
    sf::Font& m_bodyFont;
    sf::Texture& m_placeholderAvatarTexture;

    RoundedRectangleShape m_card;
    sf::Text m_title;
    sf::Text m_statusText;

    std::vector<WaitingPlayer> m_players;
    std::vector<PlayerSlot> m_slots;
    static constexpr size_t m_maxDisplayedPlayers = 6;
    static constexpr float m_avatarDiameter = 76.f;

    // Small rotating-dots spinner drawn under the player slots - the "waiting sign"
    float m_spinnerAngle = 0.f;
    static constexpr int m_spinnerDotCount = 8;
    static constexpr float m_spinnerRadius = 20.f;
    sf::Vector2f m_spinnerCenter;
};
