#include "ToolDock.h"
#include <algorithm>
#include <utility>
#include <string>

// Constructor and initial tool state
ToolDock::ToolDock(float canvasX, float canvasY, float canvasWidth, sf::Font& bodyFont)
	: m_pencilSprite(m_pencilTexture)
	, m_eraserSprite(m_eraserTexture)
	, m_paintSprite(m_paintTexture)
	, m_clearBtn(bodyFont, "Clear", { 0.f,0.f }, { 90.f,52.f }, sf::Color(230, 100, 100), sf::Color::White, 16)
	, m_sizeBox({ 44.f,44.f }, 8.f)
	, m_showSizeOptions(false)
	, m_clearRequested(false)
	, m_activeColor(sf::Color::Black)
	, m_activeTool(ToolType::Pencil)
	, m_pencilThickness(8.f)
	, m_eraserThickness(18.f)
{
	float dockY = canvasY;

	initPalette(canvasX, dockY);
	initTools(canvasX, dockY, canvasWidth, bodyFont);

	updateSizeIndicator();
	updateToolHighlight();
}

// Color palette layout
void ToolDock::initPalette(float canvasX, float dockY)
{
	std::vector<sf::Color> colors =
	{
	sf::Color::Black, sf::Color(128,128,128), sf::Color(139,69,19), sf::Color::Red,
	sf::Color(255,140,0), sf::Color::Yellow, sf::Color::Green, sf::Color(0,191,255),
	sf::Color::White, sf::Color(200,200,200), sf::Color(255,192,203), sf::Color(160,32,240),
	sf::Color(255,215,0), sf::Color(144,238,144), sf::Color::Cyan, sf::Color(75,0,130)
	};

	float radius = 11.f;
	float gap = 26.f;

	for (size_t i = 0; i < colors.size(); i++)
	{
		ColorSwatch swatch;
		swatch.color = colors[i];
		swatch.shape.setRadius(radius);
		swatch.shape.setFillColor(colors[i]);

		int row = (i < 8) ? 0 : 1;
		int col = i % 8;

		swatch.shape.setPosition({ canvasX + col * gap, dockY + row * gap });
		m_colorPalette.push_back(swatch);
	}

	// Active color selection ring
	m_colorSelectorRing.setRadius(radius + 2.f);
	m_colorSelectorRing.setFillColor(sf::Color::Transparent);
	m_colorSelectorRing.setOutlineThickness(2.5f);
	m_colorSelectorRing.setOutlineColor(sf::Color(106, 90, 160));
	m_colorSelectorRing.setPosition({ canvasX - 2.f, dockY - 2.f });
}

// Tool, size selector and clear button layout
void ToolDock::initTools(float canvasX, float dockY, float canvasWidth, sf::Font& bodyFont)
{
	float iconSize = 52.f;
	float sizeBoxVisual = 44.f;

	float paletteWidth = 7 * 26.f + 22.f;
	float clearBtnWidth = 90.f;

	float fixedContentWidth = paletteWidth + (iconSize * 4.f) + clearBtnWidth;
	float remaining = canvasWidth - fixedContentWidth;
	float gap = remaining / 5.f;

	float pencilX = canvasX + paletteWidth + gap;
	float eraserX = pencilX + iconSize + gap;
	float sizeBoxX = eraserX + iconSize + gap;
	float paintX = sizeBoxX + iconSize + gap;
	float clearBtnX = paintX + iconSize + gap;

	// Pencil icon
	if (m_pencilTexture.loadFromFile("assets/pencil.png"))
	{
		m_pencilSprite.setTexture(m_pencilTexture, true);
		auto tex = m_pencilTexture.getSize();
		m_pencilSprite.setScale({ iconSize / tex.x, iconSize / tex.y });
		m_pencilSprite.setPosition({ pencilX, dockY });
	}

	// Eraser icon
	if (m_eraserTexture.loadFromFile("assets/eraser.png"))
	{
		m_eraserSprite.setTexture(m_eraserTexture, true);
		auto tex = m_eraserTexture.getSize();
		m_eraserSprite.setScale({ iconSize / tex.x, iconSize / tex.y });
		m_eraserSprite.setPosition({ eraserX, dockY });
	}

	// Shared pencil and eraser size button
	m_sizeBox.setFillColor(sf::Color(240, 235, 220));
	m_sizeBox.setOutlineColor(sf::Color(210, 210, 210));
	m_sizeBox.setOutlineThickness(1.5f);
	m_sizeBox.setPosition({ sizeBoxX + (iconSize - sizeBoxVisual) / 2.f, dockY + (iconSize - sizeBoxVisual) / 2.f });

	// Paint icon
	if (m_paintTexture.loadFromFile("assets/paint.png"))
	{
		m_paintSprite.setTexture(m_paintTexture, true);
		auto tex = m_paintTexture.getSize();
		m_paintSprite.setScale({ iconSize / tex.x, iconSize / tex.y });
		m_paintSprite.setPosition({ paintX, dockY });
	}

	// Clear button
	m_clearBtn.setPosition({ clearBtnX + clearBtnWidth / 2.f, dockY + iconSize / 2.f });

	// Active tool highlight bar style
	m_toolHighlight.setFillColor(sf::Color(106, 90, 160));

	// --- Size dropdown layout: opens DOWNWARD below the size box, as a single
	// panel containing one clearly labeled row per boldness level ---
	float rowW = 130.f;
	float rowH = 36.f;
	float rowGap = 6.f;
	float panelPad = 8.f;
	int levelCount = 3; // Thin / Medium / Bold

	float panelW = rowW + panelPad * 2.f;
	float panelH = levelCount * rowH + (levelCount - 1) * rowGap + panelPad * 2.f;
	float panelX = sizeBoxX + iconSize / 2.f - panelW / 2.f;
	float panelY = dockY - iconSize - 10.f; // sits below the icon row, dropping down

	m_sizePopupPanel.setSize({ panelW, panelH });
	m_sizePopupPanel.setRadius(10.f);
	m_sizePopupPanel.setFillColor(sf::Color(255, 255, 255, 250));
	m_sizePopupPanel.setOutlineColor(sf::Color(200, 195, 210));
	m_sizePopupPanel.setOutlineThickness(1.5f);
	m_sizePopupPanel.setPosition({ panelX, panelY });

	// Three clearly distinct boldness levels, each with a label and a stroke-sample
	// swatch that grows with the actual thickness so the level is visible at a glance.
	std::vector<std::pair<std::string, float>> pencilLevels =
	{
		{ "Thin",   3.f },
		{ "Medium", 8.f },
		{ "Bold",   14.f }
	};
	std::vector<std::pair<std::string, float>> eraserLevels =
	{
		{ "Thin",   8.f },
		{ "Medium", 18.f },
		{ "Bold",   30.f }
	};

	auto buildLevels = [&](std::vector<std::pair<std::string, float>>& levels,
		std::vector<SizeOption>& out, sf::Color swatchColor)
		{
			for (size_t i = 0; i < levels.size(); i++)
			{
				SizeOption option(bodyFont);
				option.thickness = levels[i].second;

				float rowX = panelX + panelPad;
				float rowY = panelY + panelPad + i * (rowH + rowGap);

				option.rowBg.setSize({ rowW, rowH });
				option.rowBg.setRadius(8.f);
				option.rowBg.setFillColor(sf::Color(244, 242, 248));
				option.rowBg.setOutlineThickness(0.f);
				option.rowBg.setPosition({ rowX, rowY });

				// Stroke sample: a short horizontal bar whose height IS the thickness,
				// so the row visually communicates exactly how bold the line will be.
				float swatchHeight = std::min(levels[i].second, rowH - 10.f);
				option.swatch.setSize({ 40.f, swatchHeight });
				option.swatch.setOrigin({ 0.f, swatchHeight / 2.f });
				option.swatch.setFillColor(swatchColor);
				option.swatch.setPosition({ rowX + 14.f, rowY + rowH / 2.f });

				option.label.setString(levels[i].first);
				option.label.setCharacterSize(14);
				option.label.setFillColor(sf::Color(60, 55, 70));
				option.label.setPosition({ rowX + 14.f + 40.f + 12.f, rowY + rowH / 2.f - 10.f });

				out.push_back(std::move(option));
			}
		};

	buildLevels(pencilLevels, m_pencilSizes, sf::Color(60, 60, 60));
	buildLevels(eraserLevels, m_eraserSizes, sf::Color(150, 150, 150));
}

// Update shared size indicator based on active tool
void ToolDock::updateSizeIndicator()
{
	float thickness = getActiveThickness();
	float maxThickness = (m_activeTool == ToolType::Eraser) ? 30.f : 14.f;
	float radius = 3.f + (thickness / maxThickness) * 7.f;

	m_sizeIndicatorDot.setRadius(radius);
	m_sizeIndicatorDot.setOrigin({ radius, radius });

	if (m_activeTool == ToolType::Eraser)
	{
		m_sizeBox.setFillColor(sf::Color(90, 90, 90));
		m_sizeIndicatorDot.setFillColor(sf::Color::White);
	}
	else
	{
		m_sizeBox.setFillColor(sf::Color(240, 235, 220));
		m_sizeIndicatorDot.setFillColor(sf::Color::Black);
	}

	sf::Vector2f pos = m_sizeBox.getPosition();
	float boxSize = 44.f;
	m_sizeIndicatorDot.setPosition({ pos.x + boxSize / 2.f, pos.y + boxSize / 2.f });
}

// Update the highlight bar to sit under whichever tool icon is active
void ToolDock::updateToolHighlight()
{
	const sf::Sprite* activeSprite = nullptr;

	switch (m_activeTool)
	{
	case ToolType::Pencil:     activeSprite = &m_pencilSprite; break;
	case ToolType::Eraser:     activeSprite = &m_eraserSprite; break;
	case ToolType::FillBucket: activeSprite = &m_paintSprite;  break;
	default: break;
	}

	if (!activeSprite)
		return;

	sf::FloatRect bounds = activeSprite->getGlobalBounds();
	float highlightHeight = 3.f;
	float verticalGap = 4.f;

	m_toolHighlight.setSize({ bounds.size.x, highlightHeight });
	m_toolHighlight.setPosition({ bounds.position.x, bounds.position.y + bounds.size.y + verticalGap });
}

// Tool and size state helpers
bool ToolDock::isSpriteClicked(const sf::Sprite& sprite, sf::Vector2i mousePos) const
{
	return sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

float ToolDock::getActiveThickness() const
{
	if (m_activeTool == ToolType::Eraser)
		return m_eraserThickness;
	return m_pencilThickness;
}

bool ToolDock::consumeClearRequest()
{
	if (m_clearRequested)
	{
		m_clearRequested = false;
		return true;
	}
	return false;
}

// Mouse interaction and tool selection
void ToolDock::handleMouseClick(sf::Vector2i mousePos)
{
	for (const auto& swatch : m_colorPalette)
	{
		if (swatch.isClicked(mousePos))
		{
			m_activeColor = swatch.color;
			m_colorSelectorRing.setPosition({ swatch.shape.getPosition().x - 2.f, swatch.shape.getPosition().y - 2.f });
			return;
		}
	}

	// Size option selection
	if (m_showSizeOptions)
	{
		auto& sizes = (m_activeTool == ToolType::Eraser) ? m_eraserSizes : m_pencilSizes;
		for (const auto& option : sizes)
		{
			if (option.isClicked(mousePos))
			{
				if (m_activeTool == ToolType::Eraser)
					m_eraserThickness = option.thickness;
				else
					m_pencilThickness = option.thickness;

				updateSizeIndicator();
				m_showSizeOptions = false;
				return;
			}
		}
	}

	// Shared size button toggle
	if (m_sizeBox.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos)))
	{
		if (m_activeTool != ToolType::FillBucket)
			m_showSizeOptions = !m_showSizeOptions;
	}
	else
	{
		m_showSizeOptions = false;
	}

	// Tool selection
	if (isSpriteClicked(m_pencilSprite, mousePos))
	{
		m_activeTool = ToolType::Pencil;
		updateSizeIndicator();
		updateToolHighlight();
	}
	else if (isSpriteClicked(m_eraserSprite, mousePos))
	{
		m_activeTool = ToolType::Eraser;
		updateSizeIndicator();
		updateToolHighlight();
	}
	else if (isSpriteClicked(m_paintSprite, mousePos))
	{
		m_activeTool = ToolType::FillBucket;
		m_showSizeOptions = false;
		updateSizeIndicator();
		updateToolHighlight();
	}

	// Clear canvas action
	if (m_clearBtn.isClicked(mousePos))
	{
		m_clearRequested = true;
	}
}

// Draw all dock components
void ToolDock::draw(sf::RenderWindow& window)
{
	window.draw(m_pencilSprite);
	window.draw(m_eraserSprite);
	window.draw(m_paintSprite);
	window.draw(m_toolHighlight);

	window.draw(m_sizeBox);
	window.draw(m_sizeIndicatorDot);

	m_clearBtn.draw(window);

	for (const auto& swatch : m_colorPalette)
		window.draw(swatch.shape);

	window.draw(m_colorSelectorRing);

	// Draw active size dropdown: panel background, then each row (highlighting
	// whichever level matches the tool's current thickness) with its swatch and label.
	if (m_showSizeOptions)
	{
		window.draw(m_sizePopupPanel);

		auto& sizes = (m_activeTool == ToolType::Eraser) ? m_eraserSizes : m_pencilSizes;
		float activeThickness = getActiveThickness();

		for (auto& option : sizes)
		{
			bool isSelected = (option.thickness == activeThickness);
			option.rowBg.setFillColor(isSelected ? sf::Color(224, 218, 240) : sf::Color(244, 242, 248));
			option.rowBg.setOutlineThickness(isSelected ? 1.5f : 0.f);
			option.rowBg.setOutlineColor(sf::Color(106, 90, 160));

			window.draw(option.rowBg);
			window.draw(option.swatch);
			window.draw(option.label);
		}
	}
}