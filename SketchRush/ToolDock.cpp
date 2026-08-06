#include "ToolDock.h"

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
    , m_pencilThickness(4.f)
    , m_eraserThickness(12.f)
{
    // canvasY is now the exact top-of-dock Y computed by GameScreen — used directly, no offset
    float dockY = canvasY;

    initPalette(canvasX, dockY);
    initTools(canvasX, dockY, canvasWidth, bodyFont);

    updateSizeIndicator();
}

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

    m_colorSelectorRing.setRadius(radius + 2.f);
    m_colorSelectorRing.setFillColor(sf::Color::Transparent);
    m_colorSelectorRing.setOutlineThickness(2.5f);
    m_colorSelectorRing.setOutlineColor(sf::Color(106, 90, 160));
    m_colorSelectorRing.setPosition({ canvasX - 2.f, dockY - 2.f });
}

void ToolDock::initTools(float canvasX, float dockY, float canvasWidth, sf::Font& bodyFont)
{
    float iconSize = 52.f; // increased from 42

    // Palette total width (8 columns, gap 26, swatch diameter 22)
    float paletteWidth = 7 * 26.f + 22.f;

    float clearBtnWidth = 90.f;

    // Fixed content: palette + 4 icons (pencil, eraser, sizeBox, paint) + clear button
    float fixedContentWidth = paletteWidth + (iconSize * 4.f) + clearBtnWidth;
    float remaining = canvasWidth - fixedContentWidth;
    float gap = remaining / 5.f; // 5 equal gaps between the 6 groups

    float pencilX = canvasX + paletteWidth + gap;
    float eraserX = pencilX + iconSize + gap;
    float sizeBoxX = eraserX + iconSize + gap;
    float paintX = sizeBoxX + iconSize + gap;
    float clearBtnX = paintX + iconSize + gap;

    if (m_pencilTexture.loadFromFile("assets/pencil.png"))
    {
        m_pencilSprite.setTexture(m_pencilTexture, true);
        auto tex = m_pencilTexture.getSize();
        m_pencilSprite.setScale({ iconSize / tex.x, iconSize / tex.y });
        m_pencilSprite.setPosition({ pencilX, dockY });
    }

    if (m_eraserTexture.loadFromFile("assets/eraser.png"))
    {
        m_eraserSprite.setTexture(m_eraserTexture, true);
        auto tex = m_eraserTexture.getSize();
        m_eraserSprite.setScale({ iconSize / tex.x, iconSize / tex.y });
        m_eraserSprite.setPosition({ eraserX, dockY });
    }

    float sizeBoxVisual = 44.f;
    m_sizeBox.setFillColor(sf::Color(240, 235, 220));
    m_sizeBox.setOutlineColor(sf::Color(210, 210, 210));
    m_sizeBox.setOutlineThickness(1.5f);
    m_sizeBox.setPosition({ sizeBoxX + (iconSize - sizeBoxVisual) / 2.f, dockY + (iconSize - sizeBoxVisual) / 2.f });

    if (m_paintTexture.loadFromFile("assets/paint.png"))
    {
        m_paintSprite.setTexture(m_paintTexture, true);
        auto tex = m_paintTexture.getSize();
        m_paintSprite.setScale({ iconSize / tex.x, iconSize / tex.y });
        m_paintSprite.setPosition({ paintX, dockY });
    }

    m_clearBtn.setPosition({ clearBtnX + clearBtnWidth /2.f, dockY + iconSize / 2.f });

    // Popup size circles — offset from sizeBox center (half of new 52px box = 26)
    std::vector<float> pencil = { 2.f, 5.f, 9.f, 15.f };
    for (size_t i = 0; i < pencil.size(); i++)
    {
        SizeOption option;
        option.thickness = pencil[i];
        float r = 3.f + i * 2.f;
        option.circle.setRadius(r);
        option.circle.setOrigin({ r,r });
        option.circle.setFillColor(sf::Color(60, 60, 60));
        option.circle.setPosition({ sizeBoxX + iconSize / 2.f, dockY - 22.f - i * 24.f });
        m_pencilSizes.push_back(option);
    }

    std::vector<float> eraser = { 6.f, 12.f, 20.f, 32.f };
    for (size_t i = 0; i < eraser.size(); i++)
    {
        SizeOption option;
        option.thickness = eraser[i];
        float r = 4.f + i * 2.5f;
        option.circle.setRadius(r);
        option.circle.setOrigin({ r,r });
        option.circle.setFillColor(sf::Color(100, 100, 100));
        option.circle.setPosition({ sizeBoxX + 26.f, dockY - 22.f - i * 24.f });
        m_eraserSizes.push_back(option);
    }
}

void ToolDock::updateSizeIndicator()
{
    float thickness = getActiveThickness();
    float maxThickness = (m_activeTool == ToolType::Eraser) ? 32.f : 15.f;
    float radius = 3.f + (thickness / maxThickness) * 7.f;

    m_sizeIndicatorDot.setRadius(radius);
    m_sizeIndicatorDot.setOrigin({ radius, radius });

    if (m_activeTool == ToolType::Eraser)
    {
        m_sizeBox.setFillColor(sf::Color(90, 90, 90));   // grayish
        m_sizeIndicatorDot.setFillColor(sf::Color::White);
    }
    else
    {
        m_sizeBox.setFillColor(sf::Color(240, 235, 220)); // cream
        m_sizeIndicatorDot.setFillColor(sf::Color::Black);
    }

    sf::Vector2f pos = m_sizeBox.getPosition();
    float boxSize = 44.f; // matches the smaller box below
    m_sizeIndicatorDot.setPosition({ pos.x + boxSize / 2.f, pos.y + boxSize / 2.f });
}

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

void ToolDock::handleMouseClick(sf::Vector2i mousePos)
{
    for (const auto& swatch : m_colorPalette)
    {
        if (swatch.isClicked(mousePos))
        {
            m_activeColor = swatch.color;
            m_colorSelectorRing.setPosition({ swatch.shape.getPosition().x - 2.f, swatch.shape.getPosition().y - 2.f });
            return; // fixed: return instead of break, stop processing here
        }
    }

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

    if (m_sizeBox.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos)))
    {
        if (m_activeTool != ToolType::FillBucket)
            m_showSizeOptions = !m_showSizeOptions;
    }
    else
    {
        m_showSizeOptions = false;
    }

    if (isSpriteClicked(m_pencilSprite, mousePos))
    {
        m_activeTool = ToolType::Pencil;
        updateSizeIndicator();
    }
    else if (isSpriteClicked(m_eraserSprite, mousePos))
    {
        m_activeTool = ToolType::Eraser;
        updateSizeIndicator();
    }
    else if (isSpriteClicked(m_paintSprite, mousePos))
    {
        m_activeTool = ToolType::FillBucket;
        m_showSizeOptions = false;
        updateSizeIndicator();
    }

    if (m_clearBtn.isClicked(mousePos))
    {
        m_clearRequested = true;
    }
}

void ToolDock::draw(sf::RenderWindow& window)
{
    window.draw(m_pencilSprite);
    window.draw(m_eraserSprite);
    window.draw(m_paintSprite);
    window.draw(m_sizeBox);
    window.draw(m_sizeIndicatorDot);
    m_clearBtn.draw(window);

    for (const auto& swatch : m_colorPalette)
        window.draw(swatch.shape);
    window.draw(m_colorSelectorRing);

    if (m_showSizeOptions)
    {
        const auto& sizes = (m_activeTool == ToolType::Eraser) ? m_eraserSizes : m_pencilSizes;
        for (const auto& option : sizes)
            window.draw(option.circle);
    }
}