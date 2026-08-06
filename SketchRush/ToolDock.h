#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Button.h"
#include "RoundedRectangleShape.h"

// Active drawing tool
enum class ToolType
{
    Pencil,
    Eraser,
    FillBucket
};

// Individual color swatch
struct ColorSwatch
{
    sf::CircleShape shape;
    sf::Color color;

    bool isClicked(sf::Vector2i mousePos) const
    {
        return shape.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
    }
};

// Brush size option
struct SizeOption
{
    sf::CircleShape circle;
    float thickness = 0.f;

    bool isClicked(sf::Vector2i mousePos) const
    {
        return circle.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
    }
};

class ToolDock
{
public:

    // Constructor
    ToolDock(float canvasX, float canvasY, float canvasWidth, sf::Font& bodyFont);

    // Handle mouse clicks
    void handleMouseClick(sf::Vector2i mousePos);

    // Draw ToolDock
    void draw(sf::RenderWindow& window);

    // Return active color
    sf::Color getActiveColor() const
    {
        return m_activeColor;
    }

    // Return active tool
    ToolType getActiveTool() const
    {
        return m_activeTool;
    }

    // Return active brush size
    float getActiveThickness() const;

    // Return clear request
    bool consumeClearRequest();

private:

    // Initialize color palette
    void initPalette(float canvasX, float dockY);

    // Initialize tool buttons
    void initTools(float canvasX, float dockY, float canvasWidth, sf::Font& bodyFont);

    // Check sprite click
    bool isSpriteClicked(const sf::Sprite& sprite, sf::Vector2i mousePos) const;

    // Update size indicator
    void updateSizeIndicator();

private:

    // Tool textures
    sf::Texture m_pencilTexture;
    sf::Texture m_eraserTexture;
    sf::Texture m_paintTexture;

    // Tool sprites
    sf::Sprite m_pencilSprite;
    sf::Sprite m_eraserSprite;
    sf::Sprite m_paintSprite;

    // Clear button
    Button m_clearBtn;

    // Shared size selector
    RoundedRectangleShape m_sizeBox;
    sf::CircleShape m_sizeIndicatorDot;

    // Color palette
    std::vector<ColorSwatch> m_colorPalette;
    sf::CircleShape m_colorSelectorRing;

    // Size popup options
    std::vector<SizeOption> m_pencilSizes;
    std::vector<SizeOption> m_eraserSizes;

    // Popup visibility
    bool m_showSizeOptions;

    // Clear request flag
    bool m_clearRequested;

    // Active drawing color
    sf::Color m_activeColor;

    // Active tool
    ToolType m_activeTool;

    // Pencil thickness
    float m_pencilThickness;

    // Eraser thickness
    float m_eraserThickness;
};