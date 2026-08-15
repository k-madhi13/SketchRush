#include "Gamescreen.h"
#include "ToolDock.h"
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
	float M_vertical = (windowSize.y - headerHeight - toolDockHeight - panelsHeight) / 4.f;

	// Calculate canvas bottom Y-coordinate
	float panelsY = M_vertical + headerHeight + M_vertical;
	float canvasBottom = panelsY + panelsHeight;

	// Center the ToolDock in space remaining below the canvas
	float remainingBottomSpace = windowSize.y - canvasBottom;
	return canvasBottom + (remainingBottomSpace - toolDockHeight) / 2.f;
}

GameScreen::GameScreen(sf::Vector2u windowSize, sf::Font& titleFont, sf::Font& bodyFont)
	: m_windowSize(windowSize)
	, m_isDrawer(true)
	, m_roundBox({ windowSize.x * 0.18f, 48.f }, 12.f)
	, m_wordBox({ windowSize.x * 0.58f * 0.5f, 48.f }, 12.f)
	, m_timerBox({ windowSize.x * 0.18f * 0.45f, 48.f }, 12.f)
	, m_roundText(bodyFont, "Round 1 of 4", 20)
	, m_wordText(bodyFont, "_ _ _ _ _", 24)
	, m_timerText(bodyFont, "60", 24)
	, m_exitBtn(bodyFont, "Exit", { 0,0 }, { windowSize.x * 0.18f * 0.45f, 48.f }, sf::Color(220, 80, 80), sf::Color::White, 18)
	, m_leaderboard({ windowSize.x * 0.18f, windowSize.y * 0.68f },
		{ (windowSize.x - (windowSize.x * 0.18f + windowSize.x * 0.58f + windowSize.x * 0.18f)) / 4.f,
		(windowSize.y - 48.f - 80.f - windowSize.y * 0.68f) / 4.f + 48.f + (windowSize.y - 48.f - 80.f - windowSize.y * 0.68f) / 4.f },
		bodyFont, m_placeholderAvatarTexture)
	, m_canvas({ windowSize.x * 0.58f, windowSize.y * 0.68f }, 15.f)
	, m_chatPanel({ windowSize.x * 0.18f, windowSize.y * 0.68f }, 15.f)
	, m_chatInputBox({ windowSize.x * 0.16f, 40.f }, 10.f)
	, m_chatFont(bodyFont)
	, m_chatInputText(bodyFont, "Type a message...", 15)
	, m_drawingSprite(m_drawingLayer.getTexture())
	, m_isDrawingStroke(false)
	, m_guesserStatusText(bodyFont, "Ram is drawing...", 24)
	, m_toolDock(
		(windowSize.x - (windowSize.x * 0.18f + windowSize.x * 0.58f + windowSize.x * 0.18f)) / 4.f + windowSize.x * 0.18f + (windowSize.x - (windowSize.x * 0.18f + windowSize.x * 0.58f + windowSize.x * 0.18f)) / 4.f,
		calcToolDockY(windowSize),
		windowSize.x * 0.58f,
		bodyFont)
	, m_guideBackBtn(bodyFont, "Back to Home", { 0,0 }, { 200.f, 52.f }, sf::Color(232, 168, 56), sf::Color(46, 39, 64), 18)
	, m_guideNextBtn(bodyFont, "Next", { 0,0 }, { 160.f, 52.f }, sf::Color(232, 168, 56), sf::Color(46, 39, 64), 18)
	, m_guideTitleText(bodyFont, "", 26)
	, m_guideBodyText(bodyFont, "", 18)
{
	// Width definitions
	float leaderboardWidth = windowSize.x * 0.18f;
	float canvasWidth = windowSize.x * 0.58f;
	float chatWidth = windowSize.x * 0.18f;

	// Height definitions
	float headerHeight = 48.f;
	float toolDockHeight = 80.f;
	float panelsHeight = windowSize.y * 0.68f;

	// Symmetric horizontal spacing calculation
	float totalPanelsWidth = leaderboardWidth + canvasWidth + chatWidth;
	float M_horizontal = (windowSize.x - totalPanelsWidth) / 4.f;

	// Symmetric upper vertical spacing calculation
	float M_vertical = (windowSize.y - headerHeight - toolDockHeight - panelsHeight) / 4.f;

	// Top Header and Panels Y positions
	float headerY = M_vertical;
	float panelsY = headerY + headerHeight + M_vertical;

	// Centered Tool Dock Y calculation
	float canvasBottom = panelsY + panelsHeight;
	float remainingBottomSpace = windowSize.y - canvasBottom;
	float toolDockY = canvasBottom + (remainingBottomSpace - toolDockHeight) / 2.f;

	// Horizontal X positions
	float leaderboardX = M_horizontal;
	float canvasX = leaderboardX + leaderboardWidth + M_horizontal;
	float chatX = canvasX + canvasWidth + M_horizontal;

	// 1. Header controls setup
	m_roundBox.setFillColor(sf::Color(255, 255, 255, 180));
	m_roundBox.setPosition({ leaderboardX, headerY });

	float wordBoxWidth = canvasWidth * 0.5f;
	m_wordBox.setFillColor(sf::Color(255, 255, 255, 220));
	m_wordBox.setPosition({ canvasX + (canvasWidth - wordBoxWidth) / 2.f, headerY });

	float timerWidth = chatWidth * 0.45f;
	m_timerBox.setFillColor(sf::Color(106, 90, 160));
	m_timerBox.setPosition({ chatX, headerY });

	m_exitBtn.setPosition({ chatX + chatWidth - timerWidth / 2.f, headerY + headerHeight / 2.f });

	// Header text alignment
	m_roundText.setFillColor(sf::Color(46, 39, 64));
	m_roundText.setPosition({ leaderboardX + 15.f, headerY + 12.f });

	m_wordText.setFillColor(sf::Color(46, 39, 64));
	sf::FloatRect wb = m_wordText.getLocalBounds();
	m_wordText.setPosition({ canvasX + canvasWidth / 2.f - wb.size.x / 2.f - wb.position.x, headerY + 10.f });

	m_timerText.setFillColor(sf::Color::White);
	sf::FloatRect tb = m_timerText.getLocalBounds();
	m_timerText.setPosition({ chatX + timerWidth / 2.f - tb.size.x / 2.f - tb.position.x, headerY + 10.f });

	// 2. Main panels setup
	m_canvas.setFillColor(sf::Color(255, 255, 255, 240));
	m_canvas.setPosition({ canvasX, panelsY });

	// Drawing layer: sized to match the canvas exactly, positioned on top of it
	m_drawingLayer.resize({ static_cast<unsigned int>(canvasWidth), static_cast<unsigned int>(panelsHeight) });
	m_drawingLayer.clear(sf::Color::Transparent);
	m_drawingLayer.display();
	m_drawingSprite.setTexture(m_drawingLayer.getTexture(), true);
	m_drawingSprite.setPosition({ canvasX, panelsY });

	m_chatPanel.setFillColor(sf::Color(255, 255, 255, 180));
	m_chatPanel.setPosition({ chatX, panelsY });

	m_chatInputBox.setFillColor(sf::Color(255, 255, 255, 240));
	m_chatInputBox.setPosition({ chatX + (chatWidth - windowSize.x * 0.16f) / 2.f, panelsY + panelsHeight - 55.f });

	// Chat input text: left-aligned inside the input box, roughly vertically centered
	m_chatInputText.setFillColor(sf::Color(150, 145, 165));
	sf::Vector2f chatInputBoxPos = m_chatInputBox.getPosition();
	m_chatInputText.setPosition({ chatInputBoxPos.x + 10.f, chatInputBoxPos.y + 9.f });

	// 3. Guesser text centered in tool dock space
	m_guesserStatusText.setFillColor(sf::Color(46, 39, 64));
	sf::FloatRect gb = m_guesserStatusText.getLocalBounds();
	m_guesserStatusText.setPosition({ canvasX + canvasWidth / 2.f - gb.size.x / 2.f - gb.position.x, toolDockY + (toolDockHeight / 2.f) - (gb.size.y / 2.f) });

	// Store toolDock geometry so guide-mode arrows can target icon positions
	m_toolDockX = canvasX;
	m_toolDockYVal = toolDockY;
	m_toolDockWidthVal = canvasWidth;

	initGuideElements(bodyFont);

	// Load avatar assets: tries assets/avatar1.png upward, stopping at the first
	// missing file - so either 5 or 6 avatarN.png files both work fine.
	m_avatarTextures.reserve(6);
	for (int i = 1; i <= 6; i++)
	{
		sf::Texture tex;
		std::string path = "assets/avatar" + std::to_string(i) + ".png";
		if (!tex.loadFromFile(path))
			break;
		m_avatarTextures.push_back(std::move(tex));
	}
	if (m_avatarTextures.empty())
		std::cerr << "[avatar] no avatar images found (expected assets/avatar1.png, etc.)\n";

	if (!m_placeholderAvatarTexture.loadFromFile("assets/avatar_placeholder.png"))
		std::cerr << "[avatar] failed to load assets/avatar_placeholder.png\n";

	// Default the leaderboard's personal-info strip to the placeholder until a
	// real session starts (startPlayerSession) or guide mode is entered (setGuideMode)
	m_leaderboard.setLocalPlayer("[NAME]", 0, &m_placeholderAvatarTexture);
}

// Picks a random avatar from the loaded set and shows the player's real name +
// avatar at the top of the leaderboard. Called once Play/Enter succeeds on Home.
void GameScreen::startPlayerSession(const std::string& playerName)
{
	m_playerName = playerName; // used to prefix this player's own chat messages

	if (m_avatarTextures.empty())
	{
		m_leaderboard.setLocalPlayer(playerName, 0, nullptr);
		return;
	}

	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<size_t> dist(0, m_avatarTextures.size() - 1);
	size_t index = dist(rng);

	m_leaderboard.setLocalPlayer(playerName, 0, &m_avatarTextures[index]);
}

void GameScreen::setDrawerRole(bool isDrawer, const std::string& drawerName)
{
	// Update role state and player text
	m_isDrawer = isDrawer;
	m_guesserStatusText.setString(drawerName + " is drawing...");
}

void GameScreen::setRoundInfo(int currentRound, int totalRounds)
{
	// Update round text string
	m_roundText.setString("Round " + std::to_string(currentRound) + " of " + std::to_string(totalRounds));
}

void GameScreen::setWordHint(const std::string& hint)
{
	// Update word display string
	m_wordText.setString(hint);
}

void GameScreen::setTimerValue(int seconds)
{
	// Update timer value string
	m_timerText.setString(std::to_string(seconds));
}

// Per-frame updates that don't depend on input events - currently just the
// chat input's blinking cursor.
void GameScreen::update(float dt)
{
	if (!m_chatFocused)
		return;

	m_chatCursorBlinkTimer += dt;
	if (m_chatCursorBlinkTimer >= 0.5f)
	{
		m_chatCursorBlinkTimer = 0.f;
		m_chatCursorVisible = !m_chatCursorVisible;
		refreshChatInputDisplay();
	}
}

void GameScreen::setChatFocused(bool focused)
{
	m_chatFocused = focused;
	m_chatCursorVisible = true;
	m_chatCursorBlinkTimer = 0.f;
	m_chatInputBox.setFillColor(focused ? sf::Color(255, 255, 255, 255) : sf::Color(255, 255, 255, 240));
	refreshChatInputDisplay();
}

// Rebuilds the input box's text from current state: the typed message with a
// blinking cursor while focused, or a greyed-out placeholder otherwise.
void GameScreen::refreshChatInputDisplay()
{
	if (m_chatInputStr.empty() && !m_chatFocused)
	{
		m_chatInputText.setFillColor(sf::Color(150, 145, 165));
		m_chatInputText.setString("Type a message...");
		return;
	}

	std::string display = m_chatInputStr;
	if (m_chatFocused && m_chatCursorVisible)
		display += "|";

	m_chatInputText.setFillColor(sf::Color(46, 39, 64));
	m_chatInputText.setString(display);
}

void GameScreen::handleTextEntered(char32_t unicode)
{
	if (m_guideMode || !m_chatFocused)
		return;

	const char32_t backspace = 8;

	if (unicode == backspace)
	{
		if (!m_chatInputStr.empty())
			m_chatInputStr.pop_back();
	}
	// Ignore other control characters (Enter, Tab, Esc, ...) - Enter is handled
	// separately by sendChatMessage(), called from main.cpp
	else if (unicode < 32)
	{
		return;
	}
	else if (unicode < 128 && m_chatInputStr.size() < m_maxChatMessageLength)
	{
		// Only accept printable ASCII, matching the name field's behavior
		m_chatInputStr += static_cast<char>(unicode);
	}

	refreshChatInputDisplay();
}

// Sends whatever is currently typed (if the box is focused and it isn't just
// whitespace), then clears the input box. New messages are pushed to the back
// of m_chatMessages; drawChatMessages() reads that list back-to-front so the
// newest message renders at the top of the chat panel.
void GameScreen::sendChatMessage()
{
	if (!m_chatFocused)
		return;

	bool hasContent = m_chatInputStr.find_first_not_of(' ') != std::string::npos;
	if (hasContent)
	{
		m_chatMessages.push_back({ m_playerName, m_chatInputStr });

		if (m_chatMessages.size() > m_maxChatHistory)
			m_chatMessages.erase(m_chatMessages.begin());
	}

	m_chatInputStr.clear();
	refreshChatInputDisplay();
}

void GameScreen::handleMouseClick(sf::Vector2i mousePos)
{
	if (m_guideMode)
	{
		handleGuideClick(mousePos);
		return;
	}

	sf::Vector2f p = static_cast<sf::Vector2f>(mousePos);

	// Chat input focus toggling: available to both drawer and guesser, so this
	// is checked before the drawer-only gate below.
	if (m_chatInputBox.getGlobalBounds().contains(p))
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

	ToolType tool = m_toolDock.getActiveTool();

	if (getCanvasBounds().contains(p) && (tool == ToolType::Pencil || tool == ToolType::Eraser))
	{
		beginStroke(p);
		return;
	}

	if (getCanvasBounds().contains(p) && tool == ToolType::FillBucket)
	{
		sf::Vector2f localPos = p - m_drawingSprite.getPosition(); // world -> drawing-layer local space
		floodFillCanvas(localPos, m_toolDock.getActiveColor());
		return;
	}

	m_toolDock.handleMouseClick(mousePos);

	// Check for exit button click
	if (m_exitBtn.isClicked(mousePos))
	{
		m_exitRequested = true;
		return;
	}
}

void GameScreen::handleMouseMoved(sf::Vector2i mousePos)
{
	if (!m_isDrawer || !m_isDrawingStroke)
		return;

	sf::Vector2f p = static_cast<sf::Vector2f>(mousePos);

	// Clamp drawing to the canvas so strokes don't leak past its edges
	sf::FloatRect bounds = getCanvasBounds();
	p.x = std::max(bounds.position.x, std::min(p.x, bounds.position.x + bounds.size.x));
	p.y = std::max(bounds.position.y, std::min(p.y, bounds.position.y + bounds.size.y));

	strokeTo(p);
}

void GameScreen::handleMouseReleased()
{
	m_isDrawingStroke = false;
}

void GameScreen::beginStroke(sf::Vector2f pos)
{
	m_isDrawingStroke = true;
	m_lastDrawPos = pos;
	strokeTo(pos); // stamp a single dot immediately, so a plain click still paints a point
}

void GameScreen::strokeTo(sf::Vector2f newPos)
{
	// Pencil paints with the ToolDock's active color; Eraser "erases" by painting
	// opaque white (matching the canvas background) over existing strokes.
	ToolType tool = m_toolDock.getActiveTool();
	sf::Color color = (tool == ToolType::Eraser) ? sf::Color::White : m_toolDock.getActiveColor();
	float radius = m_toolDock.getActiveThickness() / 2.f;

	// Convert screen-space points into the drawing layer's local coordinate space
	sf::Vector2f origin = m_drawingSprite.getPosition();
	sf::Vector2f from = m_lastDrawPos - origin;
	sf::Vector2f to = newPos - origin;

	sf::CircleShape dot(radius);
	dot.setOrigin({ radius, radius });
	dot.setFillColor(color);

	// SFML has no built-in thick-line primitive, so a stroke is drawn as a row of
	// overlapping circular stamps interpolated along the segment from -> to.
	sf::Vector2f delta = to - from;
	float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
	int steps = std::max(1, static_cast<int>(distance / std::max(1.f, radius * 0.5f)));

	for (int i = 0; i <= steps; i++)
	{
		float t = static_cast<float>(i) / steps;
		dot.setPosition(from + delta * t);
		m_drawingLayer.draw(dot);
	}

	m_drawingLayer.display();
	m_lastDrawPos = newPos;
}

void GameScreen::draw(sf::RenderWindow& window)
{
	if (m_guideMode)
	{
		drawGuideOverlay(window);
		return;
	}

	// Apply a pending Clear request from the ToolDock before rendering this frame
	if (m_toolDock.consumeClearRequest())
	{
		m_drawingLayer.clear(sf::Color::Transparent);
		m_drawingLayer.display();
	}

	drawBaseLayout(window);
}

// Renders the header, main panels (leaderboard/canvas/chat), and either the
// ToolDock or the guesser banner. Shared by normal gameplay rendering and by
// the guide-mode backdrop (drawGuideOverlay calls this too, in GuideScreen.cpp).
void GameScreen::drawBaseLayout(sf::RenderWindow& window)
{
	// Render top header
	window.draw(m_roundBox);
	window.draw(m_wordBox);
	window.draw(m_timerBox);
	window.draw(m_roundText);
	window.draw(m_wordText);
	window.draw(m_timerText);
	m_exitBtn.draw(window);

	// Render middle workspace
	m_leaderboard.draw(window);
	window.draw(m_canvas);
	window.draw(m_drawingSprite); // strokes render on top of the blank canvas
	window.draw(m_chatPanel);
	drawChatMessages(window);
	window.draw(m_chatInputBox);
	window.draw(m_chatInputText);

	// Render bottom drawer controls or guesser message
	if (m_isDrawer)
		m_toolDock.draw(window);
	else
		window.draw(m_guesserStatusText);
}

namespace
{
	bool colorsMatch(sf::Color a, sf::Color b, std::uint8_t tolerance)
	{
		return std::abs(int(a.r) - int(b.r)) <= tolerance &&
			std::abs(int(a.g) - int(b.g)) <= tolerance &&
			std::abs(int(a.b) - int(b.b)) <= tolerance &&
			std::abs(int(a.a) - int(b.a)) <= tolerance;
	}

	// Greedily breaks text into lines that each fit within maxWidth, breaking on
	// word boundaries. Used to wrap chat messages inside the (narrow) chat panel.
	std::vector<std::string> wrapText(const std::string& text, sf::Font& font, unsigned int charSize, float maxWidth)
	{
		std::vector<std::string> lines;
		std::istringstream words(text);
		std::string word, currentLine;

		while (words >> word)
		{
			std::string candidate = currentLine.empty() ? word : currentLine + " " + word;
			sf::Text probe(font, candidate, charSize);

			if (probe.getLocalBounds().size.x > maxWidth && !currentLine.empty())
			{
				lines.push_back(currentLine);
				currentLine = word;
			}
			else
			{
				currentLine = candidate;
			}
		}

		if (!currentLine.empty())
			lines.push_back(currentLine);
		if (lines.empty())
			lines.push_back("");

		return lines;
	}
}

// Renders sent chat messages inside m_chatPanel, newest message at the top.
// Each message is wrapped to fit the panel's width, and only as many wrapped
// lines as fit above the input box are drawn (older lines simply scroll out
// of view rather than being clipped mid-line).
void GameScreen::drawChatMessages(sf::RenderWindow& window)
{
	const unsigned int fontSize = 15;
	const float lineHeight = 20.f;
	const float sidePadding = 12.f;
	const float topPadding = 12.f;

	sf::Vector2f panelPos = m_chatPanel.getPosition();
	sf::Vector2f panelSize = m_chatPanel.getSize();
	float maxTextWidth = panelSize.x - sidePadding * 2.f;

	float availableHeight = (m_chatInputBox.getPosition().y - 10.f) - (panelPos.y + topPadding);
	int maxLines = static_cast<int>(availableHeight / lineHeight);
	if (maxLines <= 0)
		return;

	// Walk messages newest-first, collecting wrapped display lines until the
	// visible area is full.
	std::vector<std::string> visibleLines;
	for (auto it = m_chatMessages.rbegin(); it != m_chatMessages.rend(); ++it)
	{
		std::string full = it->sender + ": " + it->text;
		std::vector<std::string> wrapped = wrapText(full, m_chatFont, fontSize, maxTextWidth);

		for (const std::string& line : wrapped)
		{
			if (static_cast<int>(visibleLines.size()) >= maxLines)
				break;
			visibleLines.push_back(line);
		}

		if (static_cast<int>(visibleLines.size()) >= maxLines)
			break;
	}

	float y = panelPos.y + topPadding;
	for (const std::string& line : visibleLines)
	{
		sf::Text lineText(m_chatFont, line, fontSize);
		lineText.setFillColor(sf::Color(46, 39, 64));
		lineText.setPosition({ panelPos.x + sidePadding, y });
		window.draw(lineText);
		y += lineHeight;
	}
}

// Flood fill algorithm: fills a contiguous region of pixels with the specified color
void GameScreen::floodFillCanvas(sf::Vector2f canvasPos, sf::Color fillColor)
{
	sf::Vector2u size = m_drawingLayer.getSize();

	int startX = static_cast<int>(canvasPos.x);
	int startY = static_cast<int>(canvasPos.y);

	if (startX < 0 || startY < 0 ||
		static_cast<unsigned>(startX) >= size.x ||
		static_cast<unsigned>(startY) >= size.y)
		return;

	fillColor.a = 255; // match the opaque alpha used by pencil/eraser stamps

	// Pull the layer's current pixels into CPU memory - RenderTexture has no
	// per-pixel access, only draw() calls, so this is the only way to inspect it.
	sf::Image img = m_drawingLayer.getTexture().copyToImage();

	sf::Color targetColor = img.getPixel({ static_cast<unsigned>(startX), static_cast<unsigned>(startY) });

	if (colorsMatch(targetColor, fillColor, 0))
		return; // clicked region already is this color

	const std::uint8_t tolerance = 30; // absorbs anti-aliased edge pixels along strokes

	std::vector<bool> visited(size.x * size.y, false);
	std::stack<sf::Vector2i> pending;
	pending.push({ startX, startY });

	while (!pending.empty())
	{
		sf::Vector2i pos = pending.top();
		pending.pop();

		if (pos.x < 0 || pos.y < 0 ||
			static_cast<unsigned>(pos.x) >= size.x ||
			static_cast<unsigned>(pos.y) >= size.y)
			continue;

		size_t index = static_cast<size_t>(pos.y) * size.x + pos.x;
		if (visited[index])
			continue;

		sf::Color pixel = img.getPixel({ static_cast<unsigned>(pos.x), static_cast<unsigned>(pos.y) });
		if (!colorsMatch(pixel, targetColor, tolerance))
			continue;

		visited[index] = true;
		img.setPixel({ static_cast<unsigned>(pos.x), static_cast<unsigned>(pos.y) }, fillColor);

		pending.push({ pos.x + 1, pos.y });
		pending.push({ pos.x - 1, pos.y });
		pending.push({ pos.x, pos.y + 1 });
		pending.push({ pos.x, pos.y - 1 });
	}

	// Push the mutated image back into the RenderTexture by rebuilding a
	// texture from it and drawing that as a single full-canvas sprite.
	sf::Texture patchedTexture;
	if (!patchedTexture.loadFromImage(img))
		return;

	sf::Sprite patchedSprite(patchedTexture);
	m_drawingLayer.clear(sf::Color::Transparent);
	m_drawingLayer.draw(patchedSprite);
	m_drawingLayer.display();
}

// Returns true once per click if the Exit button was pressed
bool GameScreen::consumeExitRequest()
{
	if (m_exitRequested)
	{
		m_exitRequested = false;
		return true;
	}
	return false;
}