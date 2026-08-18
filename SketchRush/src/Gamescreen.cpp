#include "Gamescreen.h"
#include "ToolDock.h"
#include "Client.h"

#include <cmath>
#include <algorithm>
#include <stack>
#include <cstdint>
#include <random>
#include <iostream>
#include <sstream>

// Calculates ToolDock Y-position by centering it in the remaining bottom space
float GameScreen::calcToolDockY(sf::Vector2u windowSize)
{
	float headerHeight = 48.f;
	float toolDockHeight = 80.f;
	float panelsHeight = windowSize.y * 0.7f;

	// Uniform margin calculation for upper sections
	float M_vertical =
		(windowSize.y -
			headerHeight -
			toolDockHeight -
			panelsHeight) / 4.f;

	// Calculate canvas bottom Y-coordinate
	float panelsY =
		M_vertical +
		headerHeight +
		M_vertical;

	float canvasBottom =
		panelsY + panelsHeight;

	// Center the ToolDock in space remaining below the canvas
	float remainingBottomSpace =
		windowSize.y - canvasBottom;

	return canvasBottom +
		(remainingBottomSpace - toolDockHeight) / 2.f;
}


GameScreen::GameScreen(
	sf::Vector2u windowSize,
	sf::Font& titleFont,
	sf::Font& bodyFont,
	Client& client
)
	: m_windowSize(windowSize)
	, m_client(client)
	, m_isDrawer(false)
	, m_roundBox(
		{ windowSize.x * 0.18f, 48.f },
		12.f)
	, m_wordBox(
		{ windowSize.x * 0.58f * 0.5f, 48.f },
		12.f)
	, m_timerBox(
		{ windowSize.x * 0.18f * 0.45f, 48.f },
		12.f)
	, m_roundText(
		bodyFont,
		"Waiting...",
		20)
	, m_wordText(
		bodyFont,
		"_ _ _ _ _",
		24)
	, m_timerText(
		bodyFont,
		"60",
		24)
	, m_exitBtn(
		bodyFont,
		"Exit",
		{ 0,0 },
		{
			windowSize.x * 0.18f * 0.45f,
			48.f
		},
		sf::Color(220, 80, 80),
		sf::Color::White,
		18)
	, m_leaderboard(
		{
			windowSize.x * 0.18f,
			windowSize.y * 0.68f
		},
		{
			(windowSize.x -
				(
					windowSize.x * 0.18f +
					windowSize.x * 0.58f +
					windowSize.x * 0.18f
				)) / 4.f,

			(
				windowSize.y -
				48.f -
				80.f -
				windowSize.y * 0.68f
			) / 4.f
			+ 48.f
			+
			(
				windowSize.y -
				48.f -
				80.f -
				windowSize.y * 0.68f
			) / 4.f
		},
		bodyFont,
		m_placeholderAvatarTexture)
	, m_canvas(
		{
			windowSize.x * 0.58f,
			windowSize.y * 0.68f
		},
		15.f)
	, m_chatPanel(
		{
			windowSize.x * 0.18f,
			windowSize.y * 0.68f
		},
		15.f)
	, m_chatInputBox(
		{
			windowSize.x * 0.16f,
			40.f
		},
		10.f)
	, m_chatFont(bodyFont)
	, m_chatInputText(
		bodyFont,
		"Type a message...",
		15)
	, m_drawingSprite(
		m_drawingLayer.getTexture())
	, m_isDrawingStroke(false)
	, m_guesserStatusText(
		bodyFont,
		"",
		24)
	, m_toolDock(
		(
			windowSize.x -
			(
				windowSize.x * 0.18f +
				windowSize.x * 0.58f +
				windowSize.x * 0.18f
			)
		) / 4.f
		+
		windowSize.x * 0.18f
		+
		(
			windowSize.x -
			(
				windowSize.x * 0.18f +
				windowSize.x * 0.58f +
				windowSize.x * 0.18f
			)
		) / 4.f,

		calcToolDockY(windowSize),

		windowSize.x * 0.58f,
		bodyFont)
	, m_guideBackBtn(
		bodyFont,
		"Back to Home",
		{ 0,0 },
		{ 200.f, 52.f },
		sf::Color(232, 168, 56),
		sf::Color(46, 39, 64),
		18)
	, m_guideNextBtn(
		bodyFont,
		"Next",
		{ 0,0 },
		{ 160.f, 52.f },
		sf::Color(232, 168, 56),
		sf::Color(46, 39, 64),
		18)
	, m_guideTitleText(
		bodyFont,
		"",
		26)
	, m_guideBodyText(
		bodyFont,
		"",
		18)
{
	// Width definitions
	float leaderboardWidth =
		windowSize.x * 0.18f;

	float canvasWidth =
		windowSize.x * 0.58f;

	float chatWidth =
		windowSize.x * 0.18f;

	// Height definitions
	float headerHeight = 48.f;
	float toolDockHeight = 80.f;
	float panelsHeight =
		windowSize.y * 0.68f;

	// Symmetric horizontal spacing
	float totalPanelsWidth =
		leaderboardWidth +
		canvasWidth +
		chatWidth;

	float M_horizontal =
		(windowSize.x - totalPanelsWidth) / 4.f;

	// Symmetric upper vertical spacing
	float M_vertical =
		(
			windowSize.y -
			headerHeight -
			toolDockHeight -
			panelsHeight
		) / 4.f;

	// Top Header and Panels Y positions
	float headerY = M_vertical;

	float panelsY =
		headerY +
		headerHeight +
		M_vertical;

	// Centered Tool Dock
	float canvasBottom =
		panelsY + panelsHeight;

	float remainingBottomSpace =
		windowSize.y - canvasBottom;

	float toolDockY =
		canvasBottom +
		(
			remainingBottomSpace -
			toolDockHeight
		) / 2.f;

	// Horizontal X positions
	float leaderboardX =
		M_horizontal;

	float canvasX =
		leaderboardX +
		leaderboardWidth +
		M_horizontal;

	float chatX =
		canvasX +
		canvasWidth +
		M_horizontal;

	// =========================================================
	// Header controls
	// =========================================================

	m_roundBox.setFillColor(
		sf::Color(255, 255, 255, 180));

	m_roundBox.setPosition({
		leaderboardX,
		headerY
	});

	float wordBoxWidth =
		canvasWidth * 0.5f;

	m_wordBox.setFillColor(
		sf::Color(255, 255, 255, 220));

	m_wordBox.setPosition({
		canvasX +
			(canvasWidth - wordBoxWidth) / 2.f,
		headerY
	});

	float timerWidth =
		chatWidth * 0.45f;

	m_timerBox.setFillColor(
		sf::Color(106, 90, 160));

	m_timerBox.setPosition({
		chatX,
		headerY
	});

	m_exitBtn.setPosition({
		chatX +
			chatWidth -
			timerWidth / 2.f,
		headerY +
			headerHeight / 2.f
	});

	// Header text alignment
	m_roundText.setFillColor(
		sf::Color(46, 39, 64));

	m_roundText.setPosition({
		leaderboardX + 15.f,
		headerY + 12.f
	});

	m_wordText.setFillColor(
		sf::Color(46, 39, 64));

	sf::FloatRect wb =
		m_wordText.getLocalBounds();

	m_wordText.setPosition({
		canvasX +
			canvasWidth / 2.f -
			wb.size.x / 2.f -
			wb.position.x,
		headerY + 10.f
	});

	m_timerText.setFillColor(
		sf::Color::White);

	sf::FloatRect tb =
		m_timerText.getLocalBounds();

	m_timerText.setPosition({
		chatX +
			timerWidth / 2.f -
			tb.size.x / 2.f -
			tb.position.x,
		headerY + 10.f
	});

	// =========================================================
	// Main panels
	// =========================================================

	m_canvas.setFillColor(
		sf::Color(255, 255, 255, 240));

	m_canvas.setPosition({
		canvasX,
		panelsY
	});

	// Drawing layer
	m_drawingLayer.resize({
		static_cast<unsigned int>(canvasWidth),
		static_cast<unsigned int>(panelsHeight)
	});

	m_drawingLayer.clear(
		sf::Color::Transparent);

	m_drawingLayer.display();

	m_drawingSprite.setTexture(
		m_drawingLayer.getTexture(),
		true);

	m_drawingSprite.setPosition({
		canvasX,
		panelsY
	});

	m_chatPanel.setFillColor(
		sf::Color(255, 255, 255, 180));

	m_chatPanel.setPosition({
		chatX,
		panelsY
	});

	m_chatInputBox.setFillColor(
		sf::Color(255, 255, 255, 240));

	m_chatInputBox.setPosition({
		chatX +
			(chatWidth -
				windowSize.x * 0.16f) / 2.f,

		panelsY +
			panelsHeight -
			55.f
	});

	// Chat input text
	m_chatInputText.setFillColor(
		sf::Color(150, 145, 165));

	sf::Vector2f chatInputBoxPos =
		m_chatInputBox.getPosition();

	m_chatInputText.setPosition({
		chatInputBoxPos.x + 10.f,
		chatInputBoxPos.y + 9.f
	});

	// =========================================================
	// Guesser text
	// =========================================================

	m_guesserStatusText.setFillColor(
		sf::Color(46, 39, 64));

	sf::FloatRect gb =
		m_guesserStatusText.getLocalBounds();

	m_guesserStatusText.setPosition({
		canvasX +
			canvasWidth / 2.f -
			gb.size.x / 2.f -
			gb.position.x,

		toolDockY +
			(toolDockHeight / 2.f) -
			(gb.size.y / 2.f)
	});

	// Store tool dock geometry
	m_toolDockX = canvasX;
	m_toolDockYVal = toolDockY;
	m_toolDockWidthVal = canvasWidth;

	initGuideElements(bodyFont);

	// Load avatar assets

	m_avatarTextures.reserve(6);

	for (int i = 1; i <= 6; i++)
	{
		sf::Texture tex;

		std::string path =
			"assets/avatar" +
			std::to_string(i) +
			".png";

		if (!tex.loadFromFile(path))
			break;

		m_avatarTextures.push_back(
			std::move(tex));
	}

	if (m_avatarTextures.empty())
	{
		std::cerr
			<< "[avatar] no avatar images found "
			<< "(expected assets/avatar1.png, etc.)\n";
	}

	if (!m_placeholderAvatarTexture.loadFromFile(
		"assets/avatar_placeholder.png"))
	{
		std::cerr
			<< "[avatar] failed to load "
			<< "assets/avatar_placeholder.png\n";
	}

	m_leaderboard.setLocalPlayer(
		"[NAME]",
		0,
		&m_placeholderAvatarTexture);
}


// =============================================================
// PLAYER SESSION
// =============================================================

void GameScreen::startPlayerSession(
	const std::string& playerName)
{
	m_playerName = playerName;

	if (m_avatarTextures.empty())
	{
		m_leaderboard.setLocalPlayer(
			playerName,
			0,
			nullptr);

		return;
	}

	static std::mt19937 rng(
		std::random_device{}());

	std::uniform_int_distribution<size_t> dist(
		0,
		m_avatarTextures.size() - 1);

	size_t index = dist(rng);

	m_leaderboard.setLocalPlayer(
		playerName,
		0,
		&m_avatarTextures[index]);
}


// =============================================================
// DRAWER ROLE
// =============================================================

void GameScreen::setDrawerRole(
	bool isDrawer,
	const std::string& drawerName)
{
	m_isDrawer = isDrawer;

	m_guesserStatusText.setString(
		drawerName +
		" is drawing...");
}


// =============================================================
// ROUND
// =============================================================

void GameScreen::setRoundInfo(
	int currentRound,
	int totalRounds)
{
	m_roundText.setString(
		"Round " +
		std::to_string(currentRound) +
		" of " +
		std::to_string(totalRounds));
}


// =============================================================
// WORD
// =============================================================

void GameScreen::setWordHint(
	const std::string& hint)
{
	m_wordText.setString(hint);
}


// =============================================================
// TIMER
// =============================================================

void GameScreen::setTimerValue(int seconds)
{
	m_timerText.setString(
		std::to_string(seconds));
}


// =============================================================
// UPDATE
// =============================================================

void GameScreen::update(float dt)
{
	if (!m_chatFocused)
		return;

	m_chatCursorBlinkTimer += dt;

	if (m_chatCursorBlinkTimer >= 0.5f)
	{
		m_chatCursorBlinkTimer = 0.f;

		m_chatCursorVisible =
			!m_chatCursorVisible;

		refreshChatInputDisplay();
	}
}


// =============================================================
// CHAT FOCUS
// =============================================================

void GameScreen::setChatFocused(bool focused)
{
	m_chatFocused = focused;

	m_chatCursorVisible = true;
	m_chatCursorBlinkTimer = 0.f;

	m_chatInputBox.setFillColor(
		focused
			? sf::Color(255, 255, 255, 255)
			: sf::Color(255, 255, 255, 240));

	refreshChatInputDisplay();
}


// =============================================================
// CHAT INPUT DISPLAY
// =============================================================

void GameScreen::refreshChatInputDisplay()
{
	if (m_chatInputStr.empty() &&
		!m_chatFocused)
	{
		m_chatInputText.setFillColor(
			sf::Color(150, 145, 165));

		m_chatInputText.setString(
			"Type a message...");

		return;
	}

	std::string display =
		m_chatInputStr;

	if (m_chatFocused &&
		m_chatCursorVisible)
	{
		display += "|";
	}

	m_chatInputText.setFillColor(
		sf::Color(46, 39, 64));

	m_chatInputText.setString(display);
}


// =============================================================
// TEXT ENTERED
// =============================================================

void GameScreen::handleTextEntered(
	char32_t unicode)
{
	if (m_guideMode ||
		!m_chatFocused)
		return;

	const char32_t backspace = 8;

	if (unicode == backspace)
	{
		if (!m_chatInputStr.empty())
			m_chatInputStr.pop_back();
	}
	else if (unicode < 32)
	{
		return;
	}
	else if (
		unicode < 128 &&
		m_chatInputStr.size() <
			m_maxChatMessageLength)
	{
		m_chatInputStr +=
			static_cast<char>(unicode);
	}

	refreshChatInputDisplay();
}


// =============================================================
// DISPLAY CHAT MESSAGE
// =============================================================

void GameScreen::DisplayChatMessage(
	std::string name,
	std::string message)
{
	if (name == "Server"){
		name = "";
	}
	// Add newest message to the back
	m_chatMessages.push_back({
		name,
		message
	});

	// Keep only the newest 15 messages
	if (m_chatMessages.size() >
		m_maxChatHistory)
	{
		m_chatMessages.erase(
			m_chatMessages.begin());
	}
}


// =============================================================
// GET CHAT MESSAGE
// =============================================================

std::string GameScreen::GetChatMessage()
{
	if (!m_chatFocused)
		return "";

	bool hasContent =
		m_chatInputStr.find_first_not_of(' ')
		!= std::string::npos;

	std::string temp;

	if (hasContent)
	{
		temp = m_chatInputStr;
	}

	m_chatInputStr.clear();

	refreshChatInputDisplay();

	return temp;
}


// =============================================================
// MOUSE CLICK
// =============================================================

void GameScreen::handleMouseClick(
	sf::Vector2i mousePos)
{
	// Exit button
	if (m_exitBtn.isClicked(mousePos))
	{
		m_exitRequested = true;
		return;
	}

	if (m_guideMode)
	{
		handleGuideClick(mousePos);
		return;
	}

	sf::Vector2f p =
		static_cast<sf::Vector2f>(mousePos);

	// Chat input
	if (m_chatInputBox.getGlobalBounds()
		.contains(p))
	{
		setChatFocused(true);
		return;
	}
	else if (m_chatFocused)
	{
		setChatFocused(false);
	}

	if (!m_isDrawer)
		return;

	ToolType tool =
		m_toolDock.getActiveTool();

	if (
		getCanvasBounds().contains(p) &&
		(
			tool == ToolType::Pencil ||
			tool == ToolType::Eraser
		))
	{
		beginStroke(p);
		return;
	}

	if (
		getCanvasBounds().contains(p) &&
		tool == ToolType::FillBucket)
	{
		sf::Vector2f localPos =
			p - m_drawingSprite.getPosition();

		floodFillCanvas(
			localPos,
			m_toolDock.getActiveColor());

		sf::Color color = m_toolDock.getActiveColor();

		// Send to server
		sf::Vector2u size =
			m_drawingLayer.getSize();

		DrawingData data;

		data.type =
			DrawingOperationType::Fill;

		data.x1 =
			localPos.x /
			static_cast<float>(size.x);

		data.y1 =
			localPos.y /
			static_cast<float>(size.y);

		data.x2 = 0.f;
		data.y2 = 0.f;

		data.r = color.r;
		data.g = color.g;
		data.b = color.b;
		data.a = color.a;

		data.thickness = 0.f;

		
		m_client.SendDrawingData(data);


		return;
	}

	m_toolDock.handleMouseClick(mousePos);
}


// =============================================================
// MOUSE MOVED
// =============================================================

void GameScreen::handleMouseMoved(
	sf::Vector2i mousePos)
{
	if (!m_isDrawer ||
		!m_isDrawingStroke)
		return;

	sf::Vector2f p =
		static_cast<sf::Vector2f>(mousePos);

	sf::FloatRect bounds =
		getCanvasBounds();

	p.x = std::max(
		bounds.position.x,
		std::min(
			p.x,
			bounds.position.x +
				bounds.size.x));

	p.y = std::max(
		bounds.position.y,
		std::min(
			p.y,
			bounds.position.y +
				bounds.size.y));

	strokeTo(p);
}


// =============================================================
// MOUSE RELEASED
// =============================================================

void GameScreen::handleMouseReleased()
{
	m_isDrawingStroke = false;
}


// =============================================================
// BEGIN STROKE
// =============================================================

void GameScreen::beginStroke(
	sf::Vector2f pos)
{
	m_isDrawingStroke = true;

	m_lastDrawPos = pos;

	strokeTo(pos);
}


// =============================================================
// STROKE
// =============================================================

void GameScreen::strokeTo(
	sf::Vector2f newPos)
{
	ToolType tool =
		m_toolDock.getActiveTool();

	sf::Color color =
		(tool == ToolType::Eraser)
			? sf::Color::White
			: m_toolDock.getActiveColor();

	float radius =
		m_toolDock.getActiveThickness() / 2.f;

	sf::Vector2f origin =
		m_drawingSprite.getPosition();

	sf::Vector2f from =
		m_lastDrawPos - origin;

	sf::Vector2f to =
		newPos - origin;

	sf::CircleShape dot(radius);

	dot.setOrigin({
		radius,
		radius
	});

	dot.setFillColor(color);

	sf::Vector2f delta =
		to - from;

	float distance =
		std::sqrt(
			delta.x * delta.x +
			delta.y * delta.y);

	int steps =
		std::max(
			1,
			static_cast<int>(
				distance /
				std::max(
					1.f,
					radius * 0.5f)));

	for (int i = 0;
		i <= steps;
		i++)
	{
		float t =
			static_cast<float>(i) /
			steps;

		dot.setPosition(
			from + delta * t);

		m_drawingLayer.draw(dot);
	}

	m_drawingLayer.display();

	// Sending Data
	sf::Vector2u size =
        m_drawingLayer.getSize();

    DrawingData data;

    data.type =
        DrawingOperationType::Stroke;

    // Normalized coordinates
    data.x1 =
        from.x / static_cast<float>(size.x);

    data.y1 =
        from.y / static_cast<float>(size.y);

    data.x2 =
        to.x / static_cast<float>(size.x);

    data.y2 =
        to.y / static_cast<float>(size.y);

    // Color
    data.r = color.r;
    data.g = color.g;
    data.b = color.b;
    data.a = color.a;

    // Normalized thickness
    data.thickness =
        m_toolDock.getActiveThickness() / static_cast<float>(size.x);

    m_client.SendDrawingData(data);


	m_lastDrawPos = newPos;
}


// =============================================================
// DRAW
// =============================================================

void GameScreen::draw(
	sf::RenderWindow& window)
{
	if (m_guideMode)
	{
		drawGuideOverlay(window);
		return;
	}

	// Apply pending clear request
	if (m_toolDock.consumeClearRequest())
	{
		m_drawingLayer.clear(
			sf::Color::Transparent);

		m_drawingLayer.display();

		DrawingData data;

		data.type =
			DrawingOperationType::Clear;

		data.x1 = 0.f;
		data.y1 = 0.f;
		data.x2 = 0.f;
		data.y2 = 0.f;

		data.r = 0;
		data.g = 0;
		data.b = 0;
		data.a = 0;

		data.thickness = 0.f;

		m_client.SendDrawingData(data);
	}

	drawBaseLayout(window);
}


// =============================================================
// BASE LAYOUT
// =============================================================

void GameScreen::drawBaseLayout(
	sf::RenderWindow& window)
{
	// Header
	window.draw(m_roundBox);
	window.draw(m_wordBox);
	window.draw(m_timerBox);

	window.draw(m_roundText);
	window.draw(m_wordText);
	window.draw(m_timerText);

	m_exitBtn.draw(window);

	// Main workspace
	m_leaderboard.draw(window);

	window.draw(m_canvas);

	window.draw(m_drawingSprite);

	window.draw(m_chatPanel);

	drawChatMessages(window);

	window.draw(m_chatInputBox);
	window.draw(m_chatInputText);

	// Bottom controls
	if (m_isDrawer)
		m_toolDock.draw(window);
	else
		window.draw(m_guesserStatusText);
}


// =============================================================
// HELPER FUNCTIONS
// =============================================================

namespace
{
	bool colorsMatch(
		sf::Color a,
		sf::Color b,
		std::uint8_t tolerance)
	{
		return
			std::abs(int(a.r) - int(b.r)) <= tolerance &&
			std::abs(int(a.g) - int(b.g)) <= tolerance &&
			std::abs(int(a.b) - int(b.b)) <= tolerance &&
			std::abs(int(a.a) - int(b.a)) <= tolerance;
	}


	std::vector<std::string> wrapText(
		const std::string& text,
		sf::Font& font,
		unsigned int charSize,
		float maxWidth)
	{
		std::vector<std::string> lines;

		std::istringstream words(text);

		std::string word;
		std::string currentLine;

		while (words >> word)
		{
			std::string candidate =
				currentLine.empty()
					? word
					: currentLine + " " + word;

			sf::Text probe(
				font,
				candidate,
				charSize);

			if (
				probe.getLocalBounds().size.x >
					maxWidth &&
				!currentLine.empty())
			{
				lines.push_back(
					currentLine);

				currentLine = word;
			}
			else
			{
				currentLine = candidate;
			}
		}

		if (!currentLine.empty())
		{
			lines.push_back(
				currentLine);
		}

		if (lines.empty())
		{
			lines.push_back("");
		}

		return lines;
	}
}


// =============================================================
// DRAW CHAT MESSAGES
// =============================================================

void GameScreen::drawChatMessages(
	sf::RenderWindow& window)
{
	const unsigned int fontSize = 15;

	const float lineHeight = 20.f;

	const float sidePadding = 12.f;

	const float bottomPadding = 12.f;

	sf::Vector2f panelPos =
		m_chatPanel.getPosition();

	sf::Vector2f panelSize =
		m_chatPanel.getSize();

	float maxTextWidth =
		panelSize.x -
		sidePadding * 2.f;

	// =========================================================
	// Calculate available chat area
	// =========================================================

	float chatBottom =
		m_chatInputBox.getPosition().y -
		10.f;

	float chatTop =
		panelPos.y +
		bottomPadding;

	float availableHeight =
		chatBottom -
		chatTop;

	int maxLines =
		static_cast<int>(
			availableHeight /
			lineHeight);

	if (maxLines <= 0)
		return;

	// =========================================================
	// Convert messages into wrapped display lines
	//
	// Oldest -> newest
	// =========================================================

	std::vector<std::string> displayLines;

	for (const auto& chat :
		m_chatMessages)
	{
		std::string full =
			chat.sender +
			": " +
			chat.text;

		std::vector<std::string> wrapped =
			wrapText(
				full,
				m_chatFont,
				fontSize,
				maxTextWidth);

		for (const std::string& line :
			wrapped)
		{
			displayLines.push_back(line);
		}
	}

	// =========================================================
	// Keep newest lines if there are too many
	// =========================================================

	if (
		static_cast<int>(
			displayLines.size()) >
		maxLines)
	{
		displayLines.erase(
			displayLines.begin(),
			displayLines.end() - maxLines);
	}

	// =========================================================
	// Draw top -> bottom
	//
	// Oldest visible message = top
	// Newest visible message = bottom
	// =========================================================

	float y = chatTop;

	for (const std::string& line :
		displayLines)
	{
		sf::Text lineText(
			m_chatFont,
			line,
			fontSize);

		lineText.setFillColor(
			sf::Color(46, 39, 64));

		lineText.setPosition({
			panelPos.x +
				sidePadding,
			y
		});

		window.draw(lineText);

		y += lineHeight;
	}
}


// =============================================================
// FLOOD FILL
// =============================================================

void GameScreen::floodFillCanvas(
	sf::Vector2f canvasPos,
	sf::Color fillColor)
{
	sf::Vector2u size =
		m_drawingLayer.getSize();

	int startX =
		static_cast<int>(
			canvasPos.x);

	int startY =
		static_cast<int>(
			canvasPos.y);

	if (
		startX < 0 ||
		startY < 0 ||
		static_cast<unsigned>(startX) >= size.x ||
		static_cast<unsigned>(startY) >= size.y)
	{
		return;
	}

	fillColor.a = 255;

	sf::Image img =
		m_drawingLayer
			.getTexture()
			.copyToImage();

	sf::Color targetColor =
		img.getPixel({
			static_cast<unsigned>(startX),
			static_cast<unsigned>(startY)
		});

	if (
		colorsMatch(
			targetColor,
			fillColor,
			0))
	{
		return;
	}

	const std::uint8_t tolerance = 30;

	std::vector<bool> visited(
		size.x * size.y,
		false);

	std::stack<sf::Vector2i> pending;

	pending.push({
		startX,
		startY
	});

	while (!pending.empty())
	{
		sf::Vector2i pos =
			pending.top();

		pending.pop();

		if (
			pos.x < 0 ||
			pos.y < 0 ||
			static_cast<unsigned>(pos.x) >= size.x ||
			static_cast<unsigned>(pos.y) >= size.y)
		{
			continue;
		}

		size_t index =
			static_cast<size_t>(pos.y) *
			size.x +
			pos.x;

		if (visited[index])
			continue;

		sf::Color pixel =
			img.getPixel({
				static_cast<unsigned>(pos.x),
				static_cast<unsigned>(pos.y)
			});

		if (
			!colorsMatch(
				pixel,
				targetColor,
				tolerance))
		{
			continue;
		}

		visited[index] = true;

		img.setPixel(
			{
				static_cast<unsigned>(pos.x),
				static_cast<unsigned>(pos.y)
			},
			fillColor);

		pending.push({
			pos.x + 1,
			pos.y
		});

		pending.push({
			pos.x - 1,
			pos.y
		});

		pending.push({
			pos.x,
			pos.y + 1
		});

		pending.push({
			pos.x,
			pos.y - 1
		});
	}

	// Rebuild drawing layer

	sf::Texture patchedTexture;

	if (!patchedTexture.loadFromImage(img))
		return;

	sf::Sprite patchedSprite(
		patchedTexture);

	m_drawingLayer.clear(
		sf::Color::Transparent);

	m_drawingLayer.draw(
		patchedSprite);

	m_drawingLayer.display();
}



bool GameScreen::consumeExitRequest()
{
	if (m_exitRequested)
	{
		m_exitRequested = false;
		return true;
	}

	return false;
}

void GameScreen::ApplyDrawingData(
    const DrawingData& data)
{
    switch (data.type)
    {
        case DrawingOperationType::Stroke:
        {
            ApplyRemoteStroke(data);
            break;
        }

        case DrawingOperationType::Fill:
        {
            ApplyRemoteFill(data);
            break;
        }

        case DrawingOperationType::Clear:
        {
            ApplyRemoteClear();
            break;
        }

        default:
        {
            break;
        }
    }
}

void GameScreen::ApplyRemoteClear()
{
    m_drawingLayer.clear(
        sf::Color::Transparent);

    m_drawingLayer.display();
}

void GameScreen::ApplyRemoteFill(
    const DrawingData& data)
{
    sf::Vector2u size =
        m_drawingLayer.getSize();

    sf::Vector2f localPosition(
        data.x1 * static_cast<float>(size.x),
        data.y1 * static_cast<float>(size.y));


    sf::Color fillColor(
        data.r,
        data.g,
        data.b,
        data.a);


    floodFillCanvas(
        localPosition,
        fillColor);
}

void GameScreen::ApplyRemoteStroke(
    const DrawingData& data)
{
    sf::Vector2u size =
        m_drawingLayer.getSize();

    // Convert normalized coordinates
    // back to canvas pixels.
    sf::Vector2f from(
        data.x1 * static_cast<float>(size.x),
        data.y1 * static_cast<float>(size.y));

    sf::Vector2f to(
        data.x2 * static_cast<float>(size.x),
        data.y2 * static_cast<float>(size.y));


    // Convert normalized thickness
    // back to pixels.
    float thickness =
        data.thickness *
        static_cast<float>(size.x);

    thickness =
        std::max(1.f, thickness);

    float radius =
        thickness / 2.f;


    // Determine the drawing color.
    sf::Color color(
        data.r,
        data.g,
        data.b,
        data.a);



    sf::CircleShape dot(radius);

    dot.setOrigin({
        radius,
        radius
    });

    dot.setFillColor(color);


    sf::Vector2f delta =
        to - from;

    float distance =
        std::sqrt(
            delta.x * delta.x +
            delta.y * delta.y);


    int steps =
        std::max(
            1,
            static_cast<int>(
                distance /
                std::max(
                    1.f,
                    radius * 0.5f)));


    for (int i = 0; i <= steps; ++i)
    {
        float t =
            static_cast<float>(i) /
            static_cast<float>(steps);

        dot.setPosition(
            from + delta * t);

        m_drawingLayer.draw(dot);
    }

    m_drawingLayer.display();
}