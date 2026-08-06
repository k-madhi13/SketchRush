#include <SFML/Graphics.hpp>
//#include "Homescreen.h"
#include "Gamescreen.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Sketch Rush", sf::State::Fullscreen);
    sf::Texture bgTexture;
    if (!bgTexture.loadFromFile("assets/bg.png"))
        return -1;

    sf::Sprite bgSprite(bgTexture);
    sf::Vector2u texSize = bgTexture.getSize();
    bgSprite.setScale({
        (float)window.getSize().x / texSize.x,
        (float)window.getSize().y / texSize.y
        });

    sf::Font titleFont, bodyFont;
    if (!titleFont.openFromFile("assets/CinzelDecorative-Regular.ttf"))
        return -1;
    if (!bodyFont.openFromFile("assets/Baloo2.ttf"))
        return -1;

    // HomeScreen homeScreen(window.getSize(), titleFont, bodyFont);
    GameScreen gameScreen(window.getSize(), titleFont, bodyFont);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Escape)
                    window.close();
            }

            // Handle mouse clicks for GameScreen UI (Color picking, tool switching, size pop-ups)
            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mousePressed->button == sf::Mouse::Button::Left)
                {
                    sf::Vector2i mousePos(mousePressed->position);
                    gameScreen.handleMouseClick(mousePos);
                }
            }
        }

        window.clear();
        window.draw(bgSprite);
        gameScreen.draw(window);
        window.display();
    }

    return 0;
}