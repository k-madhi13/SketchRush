#include "Application.h"


// =============================================================
// CONSTRUCTOR
// =============================================================

Application::Application()
    : window(
        sf::VideoMode::getDesktopMode(),
        "Sketch Rush",
        sf::State::Fullscreen
    )
{
    // ---------------------------------------------------------
    // Initial background
    // ---------------------------------------------------------

    window.clear(sf::Color(46, 39, 64));
    window.display();


    // ---------------------------------------------------------
    // Load body font first
    // ---------------------------------------------------------

    if (!bodyFont.openFromFile("assets/Baloo2.ttf"))
    {
        throw std::runtime_error(
            "FATAL ERROR: Body font not loaded"
        );
    }


    // ---------------------------------------------------------
    // Show loading screen
    // ---------------------------------------------------------

    showLoadingScreen();


    // ---------------------------------------------------------
    // Load remaining assets
    // ---------------------------------------------------------

    loadAssets();
    loadCursors();


    // ---------------------------------------------------------
    // Create screens AFTER fonts are loaded
    // ---------------------------------------------------------

    homeScreen.emplace(
        window.getSize(),
        titleFont,
        bodyFont,
        client
    );

    gameScreen.emplace(
        window.getSize(),
        titleFont,
        bodyFont,
        client
    );


    // ---------------------------------------------------------
    // Initial application state
    // ---------------------------------------------------------

    appState = AppState::Home;

    if (arrowCursor)
        window.setMouseCursor(*arrowCursor);


    // ---------------------------------------------------------
    // Initial cursor tool
    // ---------------------------------------------------------

    lastCursorTool =
        gameScreen->getToolDock().getActiveTool();
}


// =============================================================
// LOADING SCREEN
// =============================================================

void Application::showLoadingScreen()
{
    sf::Text loadingText(
        bodyFont,
        "Loading...",
        40
    );

    loadingText.setFillColor(sf::Color::White);

    sf::FloatRect bounds =
        loadingText.getLocalBounds();

    loadingText.setOrigin({
        bounds.position.x + bounds.size.x / 2.f,
        bounds.position.y + bounds.size.y / 2.f
    });

    loadingText.setPosition({
        window.getSize().x / 2.f,
        window.getSize().y / 2.f
    });

    window.clear(sf::Color(46, 39, 64));
    window.draw(loadingText);
    window.display();
}


// =============================================================
// LOAD ASSETS
// =============================================================

void Application::loadAssets()
{
    // ---------------------------------------------------------
    // Background
    // ---------------------------------------------------------

    if (!bgTexture.loadFromFile("assets/bg.png"))
    {
        throw std::runtime_error(
            "FATAL ERROR: Background not loaded"
        );
    }


    // SFML 3: Sprite needs a texture when constructed.
    bgSprite.emplace(bgTexture);


    sf::Vector2u texSize =
        bgTexture.getSize();

    bgSprite->setScale({
        static_cast<float>(window.getSize().x) /
            static_cast<float>(texSize.x),

        static_cast<float>(window.getSize().y) /
            static_cast<float>(texSize.y)
    });


    // ---------------------------------------------------------
    // Title font
    // ---------------------------------------------------------

    if (!titleFont.openFromFile(
        "assets/CinzelDecorative-Regular.ttf"))
    {
        throw std::runtime_error(
            "FATAL ERROR: Title font not loaded"
        );
    }


    // ---------------------------------------------------------
    // Background music
    // ---------------------------------------------------------

    if (!bgMusic.openFromFile("assets/music.ogg"))
    {
        std::cerr
            << "[music] failed to load "
            << "assets/music.ogg\n";
    }
    else
    {
        bgMusic.setLooping(true);
        bgMusic.setVolume(35.f);
        bgMusic.play();
    }
}


// =============================================================
// LOAD CURSORS
// =============================================================

void Application::loadCursors()
{
    auto loadCursor =
        [](sf::Image& image,
           const char* path,
           sf::Vector2u hotspot)
        -> std::optional<sf::Cursor>
    {
        if (!image.loadFromFile(path))
        {
            std::cerr
                << "[cursor] failed to load "
                << path << '\n';

            return std::nullopt;
        }

        auto cursor =
            sf::Cursor::createFromPixels(
                image.getPixelsPtr(),
                image.getSize(),
                hotspot
            );

        if (!cursor)
        {
            std::cerr
                << "[cursor] failed to create "
                << path << '\n';
        }

        return cursor;
    };


    pencilCursor =
        loadCursor(
            pencilCursorImg,
            "assets/pencil_cursor.png",
            { 0, 47 }
        );

    eraserCursor =
        loadCursor(
            eraserCursorImg,
            "assets/eraser_cursor.png",
            { 8, 38 }
        );

    paintCursor =
        loadCursor(
            paintCursorImg,
            "assets/paint_cursor.png",
            { 45, 39 }
        );


    arrowCursor =
        sf::Cursor::createFromSystem(
            sf::Cursor::Type::Arrow
        );
}


// =============================================================
// START GAME
// =============================================================

void Application::startGame()
{
    // Recreate GameScreen so every game starts fresh.

    gameScreen.emplace(
        window.getSize(),
        titleFont,
        bodyFont,
        client
    );


    gameScreen->setGuideMode(false);


    // Set local player's information.

    gameScreen->startPlayerSession(
        homeScreen->getPlayerName()
    );


    // Switch application state.

    appState = AppState::Game;


    // Set cursor according to current tool.

    lastCursorTool =
        gameScreen->getToolDock().getActiveTool();

    applyCursorForTool(lastCursorTool);
}


// =============================================================
// CURSOR
// =============================================================

void Application::applyCursorForTool(
    ToolType tool)
{
    if (tool == ToolType::Pencil &&
        pencilCursor)
    {
        window.setMouseCursor(*pencilCursor);
    }
    else if (tool == ToolType::Eraser &&
             eraserCursor)
    {
        window.setMouseCursor(*eraserCursor);
    }
    else if (tool == ToolType::FillBucket &&
             paintCursor)
    {
        window.setMouseCursor(*paintCursor);
    }
    else if (arrowCursor)
    {
        window.setMouseCursor(*arrowCursor);
    }
}


// =============================================================
// EVENTS
// =============================================================

void Application::processEvents()
{
    while (const std::optional event =
        window.pollEvent())
    {
        // -----------------------------------------------------
        // Window closed
        // -----------------------------------------------------

        if (event->is<sf::Event::Closed>())
        {
            window.close();
            continue;
        }


        // -----------------------------------------------------
        // Keyboard
        // -----------------------------------------------------

        if (const auto* keyPressed =
            event->getIf<sf::Event::KeyPressed>())
        {
            processKeyPressed(*keyPressed);
        }


        // -----------------------------------------------------
        // Mouse click
        // -----------------------------------------------------

        if (const auto* mousePressed =
            event->getIf<sf::Event::MouseButtonPressed>())
        {
            processMousePressed(*mousePressed);
        }


        // -----------------------------------------------------
        // Text input
        // -----------------------------------------------------

        if (const auto* textEntered =
            event->getIf<sf::Event::TextEntered>())
        {
            processTextEntered(*textEntered);
        }


        // -----------------------------------------------------
        // Mouse movement
        // -----------------------------------------------------

        if (const auto* mouseMoved =
            event->getIf<sf::Event::MouseMoved>())
        {
            processMouseMoved(*mouseMoved);
        }


        // -----------------------------------------------------
        // Mouse release
        // -----------------------------------------------------

        if (const auto* mouseReleased =
            event->getIf<sf::Event::MouseButtonReleased>())
        {
            processMouseReleased(*mouseReleased);
        }
    }
}


// =============================================================
// KEY PRESSED
// =============================================================

void Application::processKeyPressed(
    const sf::Event::KeyPressed& event)
{
    // ---------------------------------------------------------
    // Escape
    // ---------------------------------------------------------

    if (event.code == sf::Keyboard::Key::Escape)
    {
        window.close();
        return;
    }


    // ---------------------------------------------------------
    // Enter on Home
    // ---------------------------------------------------------

    if (event.code == sf::Keyboard::Key::Enter &&
        appState == AppState::Home)
    {
        if (homeScreen->tryStartGame())
        {
            homeScreen->setNameFocused(false);
            homeScreen->setIpFocused(false);

            startGame();
        }

        return;
    }


    // ---------------------------------------------------------
    // Enter in Game = send chat
    // ---------------------------------------------------------

    if (event.code == sf::Keyboard::Key::Enter &&
        appState == AppState::Game)
    {
        std::string message = gameScreen->GetChatMessage();
        client.SendChatMessage(message);
    }
}


// =============================================================
// MOUSE PRESSED
// =============================================================

void Application::processMousePressed(
    const sf::Event::MouseButtonPressed& event)
{
    if (event.button != sf::Mouse::Button::Left)
        return;


    sf::Vector2i mousePos(event.position);


    // =========================================================
    // HOME
    // =========================================================

    if (appState == AppState::Home)
    {
        // -----------------------------------------------------
        // Name input
        // -----------------------------------------------------

        if (homeScreen->nameInputButton
                .isClicked(mousePos))
        {
            homeScreen->setNameFocused(true);
            homeScreen->setIpFocused(false);
        }


        // -----------------------------------------------------
        // IP input
        // -----------------------------------------------------

        else if (homeScreen->serverIpInputButton
                     .isClicked(mousePos))
        {
            homeScreen->setNameFocused(false);
            homeScreen->setIpFocused(true);
        }


        // -----------------------------------------------------
        // Play
        // -----------------------------------------------------

        else if (homeScreen->playButton
                     .isClicked(mousePos))
        {
            if (homeScreen->tryStartGame())
            {
                homeScreen->setNameFocused(false);
                homeScreen->setIpFocused(false);

                startGame();
            }
        }


        // -----------------------------------------------------
        // Guide
        // -----------------------------------------------------

        else if (homeScreen->guideButton
                     .isClicked(mousePos))
        {
            homeScreen->setNameFocused(false);
            homeScreen->setIpFocused(false);

            gameScreen->setGuideMode(true);

            appState = AppState::Guide;

            gameScreen->SetCanDraw(true);

            if (arrowCursor)
                window.setMouseCursor(*arrowCursor);
        }


        // -----------------------------------------------------
        // Click outside inputs
        // -----------------------------------------------------

        else if (
            homeScreen->isNameFocused() ||
            homeScreen->isIpFocused())
        {
            homeScreen->setNameFocused(false);
            homeScreen->setIpFocused(false);
        }

        return;
    }


    // =========================================================
    // GUIDE
    // =========================================================

    if (appState == AppState::Guide)
    {
        gameScreen->handleMouseClick(mousePos);

        if (gameScreen->consumeGuideBackToHomeRequest())
        {
            gameScreen->setGuideMode(false);

            appState = AppState::Home;

            gameScreen->SetCanDraw(false);

            if (arrowCursor)
                window.setMouseCursor(*arrowCursor);
        }

        return;
    }


    // =========================================================
    // GAME
    // =========================================================

    if (appState == AppState::Game)
    {
        gameScreen->handleMouseClick(mousePos);


        // -----------------------------------------------------
        // Exit game
        // -----------------------------------------------------

        if (gameScreen->consumeExitRequest())
        {
            client.Disconnect();
            appState = AppState::Home;

            if (arrowCursor)
                window.setMouseCursor(*arrowCursor);
        }
    }
}


// =============================================================
// TEXT ENTERED
// =============================================================

void Application::processTextEntered(
    const sf::Event::TextEntered& event)
{
    // ---------------------------------------------------------
    // Home
    // ---------------------------------------------------------

    if (appState == AppState::Home)
    {
        if (homeScreen->isNameFocused())
        {
            homeScreen->handleTextEntered(
                event.unicode
            );
        }
        else if (homeScreen->isIpFocused())
        {
            homeScreen->handleIpTextEntered(
                event.unicode
            );
        }

        return;
    }


    // ---------------------------------------------------------
    // Game
    // ---------------------------------------------------------

    if (appState == AppState::Game)
    {
        gameScreen->handleTextEntered(
            event.unicode
        );
    }
}


// =============================================================
// MOUSE MOVED
// =============================================================

void Application::processMouseMoved(
    const sf::Event::MouseMoved& event)
{
    if (appState != AppState::Game)
        return;

    sf::Vector2i mousePos(event.position);

    gameScreen->handleMouseMoved(mousePos);
}


// =============================================================
// MOUSE RELEASED
// =============================================================

void Application::processMouseReleased(
    const sf::Event::MouseButtonReleased& event)
{
    if (appState != AppState::Game)
        return;

    if (event.button == sf::Mouse::Button::Left)
    {
        gameScreen->handleMouseReleased();
    }
}


// =============================================================
// UPDATE
// =============================================================

void Application::update(float dt)
{
    
    // Handle the messages obtained by the client
    ProcessNetworkMessages();

    // Close if server closes
    if (appState == AppState::Game && !client.IsConnected())
    {
        appState = AppState::Home;

        if (arrowCursor)
            window.setMouseCursor(*arrowCursor);

        return;
    }

    // ---------------------------------------------------------
    // Cursor
    // ---------------------------------------------------------


    if (appState == AppState::Game)
    {
        ToolType activeTool =
            gameScreen->getToolDock().getActiveTool();

        if (activeTool != lastCursorTool)
        {
            lastCursorTool = activeTool;

            applyCursorForTool(activeTool);
        }
    }


    // ---------------------------------------------------------
    // Home
    // ---------------------------------------------------------

    if (appState == AppState::Home)
    {
        homeScreen->update(dt);
    }


    // ---------------------------------------------------------
    // Game / Guide
    // ---------------------------------------------------------

    else if (
        appState == AppState::Game ||
        appState == AppState::Guide)
    {
        gameScreen->update(dt);
    }
}


// =============================================================
// RENDER
// =============================================================

void Application::render()
{
    window.clear();


    // ---------------------------------------------------------
    // Background
    // ---------------------------------------------------------

    if (bgSprite)
        window.draw(*bgSprite);


    // ---------------------------------------------------------
    // Screen
    // ---------------------------------------------------------

    if (appState == AppState::Home)
    {
        homeScreen->draw(window);
    }
    else
    {
        // GameScreen already handles Guide mode internally.
        gameScreen->draw(window);
    }


    window.display();
}


// =============================================================
// RUN
// =============================================================

void Application::run()
{
    while (window.isOpen())
    {
        float dt =
            frameClock.restart().asSeconds();


        processEvents();

        update(dt);

        render();
    }
}

void Application::ProcessNetworkMessages()
{
    IncomingMessage message;

    while (client.GetNextMessage(message))
    {
        switch (message.packetType)
        {
            case PacketType::LeaderboardUpdate:
            {
                int playerCount;
                message.packet >> playerCount;

                std::vector<PlayerEntry> players;

                for (int i = 0; i < playerCount; i++)
                {
                    std::string name;
                    int score;

                    message.packet >> name >> score;

                    players.push_back({name, score});
                }

                gameScreen->GetLeaderboard().setPlayers(players);
                break;
            }

            case PacketType::ChatUpdate:
            {

                std::string name,chatMessage;

                message.packet >> name >> chatMessage;
                gameScreen->DisplayChatMessage(name,chatMessage);
                break;
            }

            case PacketType::RoundNumberUpdate:
            {
                std::string roundText;
                message.packet>>roundText;
                gameScreen->SetRoundText(roundText);
                break;
            }

            case PacketType::DrawerUpdate:
            {
                std::string drawerName;
                message.packet >> drawerName;

                gameScreen->SetDrawerName(drawerName + " is drawing.");

                if(drawerName.empty()){
                    gameScreen->SetDrawerName(drawerName);
                    break;
                }

                // If this client is the drawer:
                if (drawerName == homeScreen->getPlayerName())
                    gameScreen->SetCanDraw(true);
                else
                    gameScreen->SetCanDraw(false);

                break;
            }

            case PacketType::WordUpdate:
            {
                std::string word;
                message.packet >> word;

                gameScreen->SetWord(word);

                break;
            }

            case PacketType::HintUpdate:
            {
                std::string hint;
                message.packet >> hint;

                gameScreen->SetHint(hint);

                break;
            }

            case PacketType::TimeLeft:
            {
                int timeLeft;
                message.packet >> timeLeft;

                gameScreen->SetTime(std::to_string(timeLeft));
                break;
            }            

            case PacketType::GameEnd:
            {
                gameScreen->SetCanDraw(false);
                gameScreen->SetDrawerName("Game ended.");
                break;
            }

            case PacketType::ClearScreen:
            {   
                gameScreen->ApplyRemoteClear();
                break;
            }

            case PacketType::Draw:
            {
                DrawingData data;
                uint8_t toolType;
                message.packet >> toolType
                >> data.x1
                >> data.y1
                >> data.x2
                >> data.y2
                >> data.r
                >> data.g
                >> data.b
                >> data.a
                >> data.thickness;

                data.type = static_cast<DrawingOperationType>(toolType);
                
                gameScreen->ApplyDrawingData(data);


            }


            // Send through chat
            /*case PacketType::RoundEnd:
            {
                gameScreen->HandleRoundEnd();

                break;
            }*/

            /*
            case PacketType::CorrectGuess:
            {
                std::string playerName;
                message.packet >> playerName;

                gameScreen->HandleCorrectGuess(playerName);

                break;
            }*/

            /*case PacketType::GameResults:
            {
                gameScreen->ShowGameResults();

                break;
            }*/

            default:
                break;
        }
    }
}