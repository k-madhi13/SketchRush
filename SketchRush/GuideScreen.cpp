#include "Gamescreen.h"
#include <cmath>

void GameScreen::setGuideMode(bool enabled)
{
	m_guideMode = enabled;
	if (enabled)
	{
		m_guidePage = 0;
		// Guide mode is a static walkthrough, not a real session - show the
		// silhouette placeholder and literal "[NAME]" rather than real player data
		m_leaderboard.setLocalPlayer("[NAME]", 0, &m_placeholderAvatarTexture);
	}
}

bool GameScreen::consumeGuideBackToHomeRequest()
{
	if (m_guideBackRequested)
	{
		m_guideBackRequested = false;
		return true;
	}
	return false;
}

// One-time layout setup for the guide's nav buttons, pinned to the bottom
// corners of the window so they never overlap the canvas/toolbar content.
void GameScreen::initGuideElements(sf::Font& bodyFont)
{
	float navY = static_cast<float>(m_windowSize.y) - 40.f;
	float margin = 100.f;

	m_guideBackBtn.setPosition({ margin, navY });
	m_guideNextBtn.setPosition({ static_cast<float>(m_windowSize.x) - margin, navY });

	m_guideTitleText.setFillColor(sf::Color(46, 39, 64));
	m_guideBodyText.setFillColor(sf::Color(70, 65, 85));
}

// Draws a simple arrow (line + triangular head) from `from` to `to`, entirely
// with primitive shapes so no arrow image asset is needed.
void GameScreen::drawArrow(sf::RenderWindow& window, sf::Vector2f from, sf::Vector2f to, sf::Color color) const
{
	sf::Vector2f delta = to - from;
	float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
	float angleDeg = std::atan2(delta.y, delta.x) * 180.f / 3.14159265f;
	float headSize = 14.f;
	float shaftLength = std::max(0.f, length - headSize);

	sf::RectangleShape shaft({ shaftLength, 3.f });
	shaft.setFillColor(color);
	shaft.setOrigin({ 0.f, 1.5f });
	shaft.setPosition(from);
	shaft.setRotation(sf::degrees(angleDeg));
	window.draw(shaft);

	sf::ConvexShape head(3);
	head.setPoint(0, { 0.f, -headSize * 0.5f });
	head.setPoint(1, { 0.f,  headSize * 0.5f });
	head.setPoint(2, { headSize, 0.f });
	head.setFillColor(color);
	head.setOrigin({ 0.f, 0.f });
	head.setPosition(from + delta * (shaftLength / std::max(1.f, length)));
	head.setRotation(sf::degrees(angleDeg));
	window.draw(head);
}

// Handles clicks while in guide mode. Left button: "Back to Home" on the
// first page, "Previous" on every later page. Right button: "Next" until the
// second-to-last page, "Proceed" on the final page.
void GameScreen::handleGuideClick(sf::Vector2i mousePos)
{
	if (m_guideBackBtn.isClicked(mousePos))
	{
		if (m_guidePage == 0)
			m_guideBackRequested = true;
		else
			m_guidePage--;
		return;
	}

	if (m_guideNextBtn.isClicked(mousePos))
	{
		if (m_guidePage >= m_guidePageCount - 1)
			m_guideBackRequested = true; // last page's "Proceed" button
		else
			m_guidePage++;
	}
}

// Renders the full screen layout as a static backdrop (header, leaderboard,
// canvas, chat panel, ToolDock - none of it clickable in guide mode, since
// handleMouseClick routes to handleGuideClick exclusively), then arrows
// pointing out at the relevant UI element(s) (drawn first, so they sit behind
// the text), the current page's title/body text centered on the canvas, and
// the two nav buttons pinned to the bottom corners with page-aware labels.
void GameScreen::drawGuideOverlay(sf::RenderWindow& window)
{
	drawBaseLayout(window); // canvas renders exactly as normal, no dimming

	sf::FloatRect canvasBounds = m_canvas.getGlobalBounds();
	sf::Vector2f canvasCenter(
		canvasBounds.position.x + canvasBounds.size.x / 2.f,
		canvasBounds.position.y + canvasBounds.size.y / 2.f
	);

	// Approximate ToolDock icon layout (mirrors ToolDock::initTools math) so
	// arrows can point at real icon positions without ToolDock exposing them.
	float paletteWidth = 7 * 26.f + 22.f;
	float clearBtnWidth = 90.f;
	float iconSize = 52.f;
	float fixedContentWidth = paletteWidth + (iconSize * 4.f) + clearBtnWidth;
	float remaining = m_toolDockWidthVal - fixedContentWidth;
	float gap = remaining / 5.f;

	sf::Vector2f paletteCenter(m_toolDockX + paletteWidth / 2.f, m_toolDockYVal + 13.f);
	sf::Vector2f pencilCenter(m_toolDockX + paletteWidth + gap + iconSize / 2.f, m_toolDockYVal + iconSize / 2.f);
	sf::Vector2f eraserCenter(pencilCenter.x + iconSize + gap, pencilCenter.y);
	sf::Vector2f sizeBoxCenter(eraserCenter.x + iconSize + gap, pencilCenter.y);
	sf::Vector2f paintCenter(sizeBoxCenter.x + iconSize + gap, pencilCenter.y);
	sf::Vector2f clearCenter(paintCenter.x + iconSize + gap, pencilCenter.y);

	// Other UI elements this guide points at, taken from their own drawn bounds
	sf::FloatRect leaderboardBounds = m_leaderboard.getBounds();
	sf::Vector2f leaderboardCenter(
		leaderboardBounds.position.x + leaderboardBounds.size.x / 2.f,
		leaderboardBounds.position.y + leaderboardBounds.size.y / 2.f
	);

	sf::FloatRect roundBounds = m_roundBox.getGlobalBounds();
	sf::Vector2f roundCenter(roundBounds.position.x + roundBounds.size.x / 2.f, roundBounds.position.y + roundBounds.size.y / 2.f);

	sf::FloatRect wordBounds = m_wordBox.getGlobalBounds();
	sf::Vector2f wordCenter(wordBounds.position.x + wordBounds.size.x / 2.f, wordBounds.position.y + wordBounds.size.y / 2.f);

	sf::FloatRect timerBounds = m_timerBox.getGlobalBounds();
	sf::Vector2f timerCenter(timerBounds.position.x + timerBounds.size.x / 2.f, timerBounds.position.y + timerBounds.size.y / 2.f);

	sf::FloatRect chatBounds = m_chatPanel.getGlobalBounds();
	sf::Vector2f chatCenter(chatBounds.position.x + chatBounds.size.x / 2.f, chatBounds.position.y + chatBounds.size.y / 2.f);

	sf::Color arrowColor(147, 89, 211);

	struct Page { std::string title; std::string body; };
	Page pages[m_guidePageCount] =
	{
		{ "Leaderboard", "See how everyone's scoring, live, as rounds play out." },
		{ "Round, Word & Timer", "The top bar shows the current round, the word to guess (or draw),\nand how much time is left." },
		{ "Colors", "Tap a swatch to pick your drawing color.\nThe outlined circle shows what's active." },
		{ "Pencil, Eraser & Size", "Pencil draws, Eraser removes.\nTap the size box between them to change stroke thickness." },
		{ "Fill Bucket, Clear & Chat", "Paint bucket floods an area, Clear wipes the canvas.\nGuessers type their answers in the chat panel on the right." },
		{ "You're all set!", "That's everything - let's begin." }
	};

	// Arrows drawn first, so the text block renders on top of them
	if (m_guidePage == 0)
	{
		drawArrow(window, canvasCenter, leaderboardCenter, arrowColor);
	}
	else if (m_guidePage == 1)
	{
		drawArrow(window, canvasCenter, roundCenter, arrowColor);
		drawArrow(window, canvasCenter, wordCenter, arrowColor);
		drawArrow(window, canvasCenter, timerCenter, arrowColor);
	}
	else if (m_guidePage == 2)
	{
		drawArrow(window, canvasCenter, paletteCenter, arrowColor);
	}
	else if (m_guidePage == 3)
	{
		drawArrow(window, canvasCenter, pencilCenter, arrowColor);
		drawArrow(window, canvasCenter, eraserCenter, arrowColor);
		drawArrow(window, canvasCenter, sizeBoxCenter, arrowColor);
	}
	else if (m_guidePage == 4)
	{
		drawArrow(window, canvasCenter, paintCenter, arrowColor);
		drawArrow(window, canvasCenter, clearCenter, arrowColor);
		drawArrow(window, canvasCenter, chatCenter, arrowColor);
	}
	// page 5 ("You're all set!") has no arrows

	// Center title/body text as a block on the canvas
	m_guideTitleText.setString(pages[m_guidePage].title);
	sf::FloatRect titleBounds = m_guideTitleText.getLocalBounds();
	m_guideTitleText.setOrigin({ titleBounds.position.x + titleBounds.size.x / 2.f, titleBounds.position.y + titleBounds.size.y / 2.f });
	m_guideTitleText.setPosition({ canvasCenter.x, canvasCenter.y - 30.f });

	m_guideBodyText.setString(pages[m_guidePage].body);
	sf::FloatRect bodyBounds = m_guideBodyText.getLocalBounds();
	m_guideBodyText.setOrigin({ bodyBounds.position.x + bodyBounds.size.x / 2.f, bodyBounds.position.y + bodyBounds.size.y / 2.f });
	m_guideBodyText.setPosition({ canvasCenter.x, canvasCenter.y + 20.f });

	window.draw(m_guideTitleText);
	window.draw(m_guideBodyText);

	// Update nav button labels for the current page before drawing them
	m_guideBackBtn.setText(m_guidePage == 0 ? "Back to Home" : "Previous");
	m_guideNextBtn.setText(m_guidePage == m_guidePageCount - 1 ? "Proceed" : "Next");

	m_guideBackBtn.draw(window);
	m_guideNextBtn.draw(window);
}