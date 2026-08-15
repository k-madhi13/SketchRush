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
	// placeholderAvatarTexture backs the avatar sprite until setLocalPlayer() is
	// called with a real texture (sf::Sprite always needs a valid texture to reference)
	Leaderboard(sf::Vector2f size, sf::Vector2f position, sf::Font& font, sf::Texture& placeholderAvatarTexture);

	// Update player rank list from network/game state
	void setPlayers(const std::vector<PlayerEntry>& players);

	// Sets the reserved top strip (10% of the panel height): avatar image, display
	// name, and score. Pass avatarTexture = nullptr to keep the currently shown
	// avatar and only refresh the name/score text.
	void setLocalPlayer(const std::string& name, int score, const sf::Texture* avatarTexture);

	void draw(sf::RenderWindow& window);

	// Exposes the panel's screen bounds (used by GameScreen's guide overlay to
	// point an arrow at the leaderboard)
	sf::FloatRect getBounds() const { return m_background.getGlobalBounds(); }

private:
	// One-time layout of the reserved top strip's geometry (avatar circle position/
	// size, name/score text position) - independent of which texture is shown there
	void layoutLocalSection();

	RoundedRectangleShape m_background;
	sf::Font& m_font;
	std::vector<PlayerEntry> m_players;
	sf::Vector2f m_size;

	// Reserved top strip: local player's avatar, name, and score
	static constexpr float m_localSectionRatio = 0.10f;
	float m_localSectionHeight = 0.f;
	float m_avatarDiameter = 0.f;
	sf::Vector2f m_avatarCenter;

	sf::Sprite m_localAvatarSprite;
	sf::CircleShape m_localAvatarFrame; // decorative ring around the avatar
	sf::Text m_localNameText;
	sf::Text m_localScoreText;
};