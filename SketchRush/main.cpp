#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <iostream>
#include "Homescreen.h"
#include "Gamescreen.h"

// Which screen is currently active
enum class AppState
{
    Home,
    Game,
    Guide
};

int main()
{
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Sketch Rush", sf::State::Fullscreen);

    // The white flash on launch happens because nothing is drawn to the window
    // until every asset below finishes loading. Painting a loading screen as the
    // very first thing (before any file I/O) closes that gap immediately.
    window.clear(sf::Color(46, 39, 64));
    window.display();

    // Body font is loaded first (it's small/fast) purely so the loading text
    // itself can be drawn; the same font object is reused later for the UI.
    sf::Font titleFont, bodyFont;
    if (!bodyFont.openFromFile("assets/Baloo2.ttf"))
        return -1;

    {
        sf::Text loadingText(bodyFont, "Loading...", 40);
        loadingText.setFillColor(sf::Color::White);
        sf::FloatRect tb = loadingText.getLocalBounds();
        loadingText.setOrigin({ tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f });
        loadingText.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f });

        window.clear(sf::Color(46, 39, 64));
        window.draw(loadingText);
        window.display();
    }

    // Everything below is the slower asset loading (background image, title font,
    // cursors, music, avatar textures inside GameScreen) - the loading screen
    // above is already visible on screen while this runs.
    sf::Texture bgTexture;
    if (!bgTexture.loadFromFile("assets/bg.png"))
        return -1;

    sf::Sprite bgSprite(bgTexture);
    sf::Vector2u texSize = bgTexture.getSize();
    bgSprite.setScale({
        (float)window.getSize().x / texSize.x,
        (float)window.getSize().y / texSize.y
        });

    if (!titleFont.openFromFile("assets/CinzelDecorative-Regular.ttf"))
        return -1;

    // Background music: loops continuously from launch until the program closes,
    // kept at a mild volume so it doesn't drown out anything else
    sf::Music bgMusic;
    if (!bgMusic.openFromFile("assets/music.ogg"))
        std::cerr << "[music] failed to load assets/music.ogg\n";
    else
    {
        bgMusic.setLooping(true);
        bgMusic.setVolume(35.f); // 0-100 scale; kept low/mild on purpose
        bgMusic.play();
    }

    // Custom tool cursors: each image is cropped tight to its tip, and the hotspot
    // (the pixel that represents the actual click/draw point) is pinned to that tip
    // so the cursor "points" exactly where the stroke/action will land:
    //   - pencil: the graphite point
    //   - eraser: the corner of the red/orange rubber face
    //   - paint bucket: the tip of the dripping paint
    sf::Image pencilCursorImg, eraserCursorImg, paintCursorImg;
    std::optional<sf::Cursor> pencilCursor, eraserCursor, paintCursor, arrowCursor;

    auto loadCursor = [](sf::Image& img, const char* path, sf::Vector2u hotspot) -> std::optional<sf::Cursor>
        {
            if (!img.loadFromFile(path))
            {
                std::cerr << "[cursor] failed to load " << path << "\n";
                return std::nullopt;
            }
            auto cursor = sf::Cursor::createFromPixels(img.getPixelsPtr(), img.getSize(), hotspot);
            if (!cursor)
                std::cerr << "[cursor] createFromPixels failed for " << path << "\n";
            return cursor;
        };

    pencilCursor = loadCursor(pencilCursorImg, "assets/pencil_cursor.png", { 0, 47 });
    eraserCursor = loadCursor(eraserCursorImg, "assets/eraser_cursor.png", { 8, 38 });
    paintCursor = loadCursor(paintCursorImg, "assets/paint_cursor.png", { 45, 39 });
    arrowCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);

    // HomeScreen homeScreen(window.getSize(), titleFont, bodyFont);
    // GameScreen is wrapped in std::optional so a brand-new instance can be
    // constructed each time a game session starts (see startGame() below) -
    // this guarantees every Play/Enter after an Exit begins from a clean
    // GameScreen (empty canvas, empty chat, round/timer reset, etc.) instead
    // of reusing whatever state was left over from the previous session.
    std::optional<GameScreen> gameScreen;
    gameScreen.emplace(window.getSize(), titleFont, bodyFont);
    HomeScreen homeScreen(window.getSize(), titleFont, bodyFont);

    // Start on the home screen; switch to Game once the player taps Play (or presses Enter)
    AppState appState = AppState::Home;

    // Applies whichever cursor matches the given tool, falling back to the
    // system arrow if that tool's custom cursor failed to load.
    auto applyCursorForTool = [&](ToolType tool)
        {
            if (tool == ToolType::Pencil && pencilCursor)
                window.setMouseCursor(*pencilCursor);
            else if (tool == ToolType::Eraser && eraserCursor)
                window.setMouseCursor(*eraserCursor);
            else if (tool == ToolType::FillBucket && paintCursor)
                window.setMouseCursor(*paintCursor);
            else if (arrowCursor)
                window.setMouseCursor(*arrowCursor);
        };

    // Tracks which cursor is currently applied so we only call setMouseCursor()
    // when the active tool actually changes, rather than every frame.
    // On the home screen we always want the plain system arrow, regardless of
    // whatever tool GameScreen's ToolDock happens to default to.
    ToolType lastCursorTool = gameScreen->getToolDock().getActiveTool();
    if (arrowCursor)
        window.setMouseCursor(*arrowCursor);

    // Rebuilds gameScreen from scratch and starts a fresh session for the
    // current homeScreen player name. Shared by both ways of starting a game
    // (Enter key and the Play button) so every session - including the first
    // one after returning from an Exit - begins from a clean GameScreen.
    auto startGame = [&]()
        {
            gameScreen.emplace(window.getSize(), titleFont, bodyFont);
            gameScreen->setGuideMode(false);
            gameScreen->setDrawerRole(true, homeScreen.getPlayerName());
            gameScreen->startPlayerSession(homeScreen.getPlayerName());
            appState = AppState::Game;
            lastCursorTool = gameScreen->getToolDock().getActiveTool();
            applyCursorForTool(lastCursorTool);
        };

    sf::Clock frameClock;

    while (window.isOpen())
    {
        float dt = frameClock.restart().asSeconds();
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Escape)
                    window.close();

                // Pressing Enter on the home screen starts the game, same as tapping Play -
                // but only once a name has been entered
                if (keyPressed->code == sf::Keyboard::Key::Enter && appState == AppState::Home)
                {
                    if (homeScreen.tryStartGame())
                    {
                        homeScreen.setNameFocused(false);
                        startGame();
                    }
                }
                // Pressing Enter while the chat box is focused sends the typed
                // message; sendChatMessage() is a no-op if it isn't focused.
                else if (keyPressed->code == sf::Keyboard::Key::Enter && appState == AppState::Game)
                {
                    gameScreen->sendChatMessage();
                }
            }

            // Handle mouse clicks for GameScreen UI (Color picking, tool switching, size pop-ups)
            // and for starting a pencil/eraser stroke on the canvas.
            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mousePressed->button == sf::Mouse::Button::Left)
                {
                    sf::Vector2i mousePos(mousePressed->position);

                    if (appState == AppState::Home)
                    {
                        // Clicking the name field focuses it so typed characters go there
                        if (homeScreen.nameInputButton.isClicked(mousePos))
                        {
                            homeScreen.setNameFocused(true);
                        }
                        // Play only proceeds once a non-empty name has been entered;
                        // tryStartGame() shows an inline warning otherwise
                        else if (homeScreen.playButton.isClicked(mousePos))
                        {
                            homeScreen.setNameFocused(false);
                            if (homeScreen.tryStartGame())
                            {
                                startGame();
                            }
                        }
                        // Guide button opens the tool walkthrough instead
                        else if (homeScreen.guideButton.isClicked(mousePos))
                        {
                            homeScreen.setNameFocused(false);
                            gameScreen->setGuideMode(true);
                            appState = AppState::Guide;
                            if (arrowCursor)
                                window.setMouseCursor(*arrowCursor);
                        }
                        // Clicking anywhere else on the home screen unfocuses the name field
                        else if (homeScreen.isNameFocused())
                        {
                            homeScreen.setNameFocused(false);
                        }
                    }
                    else if (appState == AppState::Guide)
                    {
                        gameScreen->handleMouseClick(mousePos);

                        // Either nav button reaching its "home" state returns to Home
                        if (gameScreen->consumeGuideBackToHomeRequest())
                        {
                            gameScreen->setGuideMode(false);
                            appState = AppState::Home;
                        }
                    }
                    else // AppState::Game
                    {
                        gameScreen->handleMouseClick(mousePos);

                        // Exit from game screen back to home screen
                        if (gameScreen->consumeExitRequest())
                        {
                            appState = AppState::Home;
                            if (arrowCursor)
                                window.setMouseCursor(*arrowCursor);
                        }
                    }
                }
            }

            // Route typed characters into the name field while it's focused
            if (const auto* textEntered = event->getIf<sf::Event::TextEntered>())
            {
                if (appState == AppState::Home)
                    homeScreen.handleTextEntered(textEntered->unicode);
                else if (appState == AppState::Game)
                    gameScreen->handleTextEntered(textEntered->unicode);
            }

            // Continue an in-progress stroke while the mouse is dragged
            if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>())
            {
                if (appState == AppState::Game)
                {
                    sf::Vector2i mousePos(mouseMoved->position);
                    gameScreen->handleMouseMoved(mousePos);
                }
            }

            // End the current stroke when the mouse button is released
            if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>())
            {
                if (appState == AppState::Game && mouseReleased->button == sf::Mouse::Button::Left)
                    gameScreen->handleMouseReleased();
            }
        }

        // Swap the OS cursor whenever the active tool has changed (game screen only -
        // the guide screen always uses the plain arrow, set when it's entered)
        if (appState == AppState::Game)
        {
            ToolType activeTool = gameScreen->getToolDock().getActiveTool();
            if (activeTool != lastCursorTool)
            {
                lastCursorTool = activeTool;
                applyCursorForTool(activeTool);
            }
        }

        if (appState == AppState::Home)
            homeScreen.update(dt);
        else if (appState == AppState::Game)
            gameScreen->update(dt); // chat cursor blink, etc.

        window.clear();
        window.draw(bgSprite);
        if (appState == AppState::Home)
            homeScreen.draw(window);
        else
            gameScreen->draw(window); // handles both Game and Guide internally
        window.display();
    }

    return 0;
}