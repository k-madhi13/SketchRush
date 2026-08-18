#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <optional>
#include <iostream>
#include <stdexcept>
#include <memory>

#include "HomeScreen.h"
#include "Gamescreen.h"
#include "Client.h"


enum class AppState
{
    Home,
    Game,
    Guide
};


class Application
{
private:

    // =========================================================
    // WINDOW
    // =========================================================

    sf::RenderWindow window;


    // =========================================================
    // ASSETS
    // =========================================================

    sf::Font titleFont;
    sf::Font bodyFont;

    sf::Texture bgTexture;
    std::optional<sf::Sprite> bgSprite;

    sf::Music bgMusic;


    // =========================================================
    // NETWORK
    // =========================================================

    Client client;


    // =========================================================
    // SCREENS
    // =========================================================

    std::optional<HomeScreen> homeScreen;
    std::optional<GameScreen> gameScreen;


    // =========================================================
    // CURSORS
    // =========================================================

    sf::Image pencilCursorImg;
    sf::Image eraserCursorImg;
    sf::Image paintCursorImg;

    std::optional<sf::Cursor> pencilCursor;
    std::optional<sf::Cursor> eraserCursor;
    std::optional<sf::Cursor> paintCursor;
    std::optional<sf::Cursor> arrowCursor;


    // =========================================================
    // STATE
    // =========================================================

    AppState appState = AppState::Home;

    ToolType lastCursorTool = ToolType::Pencil;

    sf::Clock frameClock;


    // =========================================================
    // INITIALIZATION
    // =========================================================

    void showLoadingScreen();

    void loadAssets();

    void loadCursors();


    // GAME

    void startGame();


    // EVENTS

    void processEvents();

    void processKeyPressed(
        const sf::Event::KeyPressed& event);

    void processMousePressed(
        const sf::Event::MouseButtonPressed& event);

    void processTextEntered(
        const sf::Event::TextEntered& event);

    void processMouseMoved(
        const sf::Event::MouseMoved& event);

    void processMouseReleased(
        const sf::Event::MouseButtonReleased& event);




    void applyCursorForTool(ToolType tool);



    void update(float dt);

    void render();


public:

    Application();

    void run();

    void ProcessNetworkMessages();

};