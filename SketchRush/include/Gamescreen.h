#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "RoundedRectangleShape.h"
#include "Button.h"
#include "ToolDock.h"
#include "Leaderboard.h"
#include "CommunicationUtilities.h"

class Client;
// Main gameplay screen containing all game UI components
class GameScreen
{
public:
	// Constructs and initializes the complete game interface
	GameScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont,Client& client);

	// Rendering and input handling
	void draw(sf::RenderWindow& window);
	void update(float dt);                              // per-frame updates (chat cursor blink, etc.)
	void handleMouseClick(sf::Vector2i mousePos);      // mouse button pressed
	void handleMouseMoved(sf::Vector2i mousePos);       // mouse dragged while button held
	void handleMouseReleased();                         // mouse button released

	// --- Chat ---
	// Routes typed characters into the chat input while it's focused (focus is
	// set by clicking the chat input box - see handleMouseClick). Call this from
	// main.cpp's TextEntered handler while AppState::Game is active.
	void handleTextEntered(char32_t unicode);
	// Sends whatever is currently typed in the chat box (if anything, and if the
	// box is focused), then clears it. Call this from main.cpp when Enter is
	// pressed during AppState::Game - it's a no-op otherwise, so it's safe to
	// call unconditionally.
	std::string GetChatMessage();
	void DisplayChatMessage(std::string, std::string);

	// Dynamic UI update methods
	void setDrawerRole(bool isDrawer, const std::string& drawerName = "Player");
	void setRoundInfo(int currentRound, int totalRounds);
	void setWordHint(const std::string& hint);
	void setTimerValue(int seconds);

	// Called once a Play/Enter session begins: assigns a random avatar and shows
	// the player's real name + avatar in the leaderboard's personal-info strip.
	// (Guide mode overrides this with a fixed placeholder - see setGuideMode.)
	void startPlayerSession(const std::string& playerName);

	// Accessors for gameplay integration
	ToolDock& getToolDock() { return m_toolDock; }
	sf::FloatRect getCanvasBounds() const { return m_canvas.getGlobalBounds(); }

	// Check for exit request from the Exit button; returns true once per click
	bool consumeExitRequest();

	// --- Guide mode: reuses this same screen's full layout as a static backdrop
	// (toolDock/leaderboard/chat all render but are non-interactive - guide mode
	// never routes clicks to them) with a text+arrow overlay on top.
	// Defined in GuideScreen.cpp. ---
	void setGuideMode(bool enabled);
	bool isGuideMode() const { return m_guideMode; }
	bool consumeGuideBackToHomeRequest(); // either nav button returning to Home

	Leaderboard& GetLeaderboard()
	{
    	return m_leaderboard;
	}

	void SetRoundText(std::string text){
		m_roundText.setString(text);
	}

	void SetDrawerName(std::string drawer){
		m_guesserStatusText.setString(drawer);
	}

	void SetCanDraw(bool canDraw){
		m_isDrawer = canDraw;
	}

	void SetWord(std::string word){
		m_wordText.setString(word);
	}

	void SetHint(std::string hint){
		m_wordText.setString(hint);
	}

	void SetTime(std::string timeLeft){
		m_timerText.setString(timeLeft);
	}

	void ApplyDrawingData(
        const DrawingData& data);

	void ApplyRemoteStroke(
        const DrawingData& data);

    void ApplyRemoteFill(
        const DrawingData& data);

    void ApplyRemoteClear();


private:

	Client& m_client;

	// Calculates the vertical position of the tool dock
	static float calcToolDockY(sf::Vector2u windowSize);

	// Begins a new stroke at the given point and paints the first stamp
	void beginStroke(sf::Vector2f pos);

	// Continues the current stroke from the last point to newPos
	void strokeTo(sf::Vector2f newPos);

	// Flood fill the canvas with the active color (used for the Fill Bucket tool)
	void floodFillCanvas(sf::Vector2f canvasPos, sf::Color fillColor);

	// Renders the shared screen layout (header, leaderboard/canvas/chat, and
	// either the ToolDock or the guesser banner). Used by both normal gameplay
	// rendering and the guide-mode backdrop.
	void drawBaseLayout(sf::RenderWindow& window);

	// --- Chat internals ---
	void setChatFocused(bool focused);
	void refreshChatInputDisplay();       // rebuilds the input box's text (typed text / cursor / placeholder)
	void drawChatMessages(sf::RenderWindow& window);

	// --- Guide mode internals (defined in GuideScreen.cpp) ---
	void initGuideElements(sf::Font& bodyFont);
	void drawGuideOverlay(sf::RenderWindow& window);
	void handleGuideClick(sf::Vector2i mousePos);
	void drawArrow(sf::RenderWindow& window, sf::Vector2f from, sf::Vector2f to, sf::Color color) const;

	// Window and player state
	sf::Vector2u m_windowSize;
	bool m_isDrawer;

	// Top header UI
	RoundedRectangleShape m_roundBox;
	RoundedRectangleShape m_wordBox;
	RoundedRectangleShape m_timerBox;
	sf::Text m_roundText;
	sf::Text m_wordText;
	sf::Text m_timerText;
	Button m_exitBtn;

	// Avatar assets for the leaderboard's personal-info strip: randomized real
	// avatars used during gameplay, and a fixed silhouette shown in guide mode.
	// Declared before m_leaderboard since its constructor needs a reference to
	// an already-constructed placeholder texture.
	std::vector<sf::Texture> m_avatarTextures;
	sf::Texture m_placeholderAvatarTexture;

	// Main gameplay layout
	Leaderboard m_leaderboard;
	RoundedRectangleShape m_canvas;
	RoundedRectangleShape m_chatPanel;
	RoundedRectangleShape m_chatInputBox;

	// --- Chat state ---
	// One sent message: who sent it and what they wrote. Rendered newest-first
	// (top of the chat panel) by drawChatMessages().
	struct ChatMessage
	{
		std::string sender;
		std::string text;
	};
	sf::Font& m_chatFont;              // reference to bodyFont, kept so chat lines can be built on the fly when drawing
	sf::Text m_chatInputText;          // what's currently shown inside m_chatInputBox
	std::string m_playerName = "Player"; // local player's name, used to prefix their own sent messages
	std::vector<ChatMessage> m_chatMessages;
	std::string m_chatInputStr;
	bool m_chatFocused = false;
	bool m_chatCursorVisible = true;
	float m_chatCursorBlinkTimer = 0.f;
	static constexpr unsigned int m_maxChatMessageLength = 80;
	static constexpr size_t m_maxChatHistory = 100;

	// Drawing surface: strokes are stamped here, then drawn on top of m_canvas
	sf::RenderTexture m_drawingLayer;
	sf::Sprite m_drawingSprite;
	bool m_isDrawingStroke;
	sf::Vector2f m_lastDrawPos;

	// Guesser mode status banner
	sf::Text m_guesserStatusText;

	// Drawing tools panel
	ToolDock m_toolDock;

	// Exit button click state
	bool m_exitRequested = false;

	// --- Guide mode state ---
	bool m_guideMode = false;
	int m_guidePage = 0;
	static const int m_guidePageCount = 6;

	Button m_guideBackBtn;  // "Back to Home" on page 0, "Previous" otherwise
	Button m_guideNextBtn;  // "Next" until the last page, "Home" on the last page
	sf::Text m_guideTitleText;
	sf::Text m_guideBodyText;

	bool m_guideBackRequested = false;

	// Stored so guide-mode arrows can point at ToolDock icons without ToolDock
	// needing to expose its internal icon positions
	float m_toolDockX = 0.f;
	float m_toolDockYVal = 0.f;
	float m_toolDockWidthVal = 0.f;
};