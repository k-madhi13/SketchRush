#pragma once
#include <string>
#include <SFML/Network.hpp>

class Player;

enum PacketType{
    Chat,
    Draw,
    PlayerJoin,
    PlayerLeave,
    GameState,
    ChatUpdate,
    LeaderboardUpdate,
    RoundNumberUpdate,
    DrawerUpdate,
    GameResults,
    WordUpdate,
    HintUpdate,
    CorrectGuess,
    RoundEnd,
    TimeLeft,
    GameEnd,
    ClearScreen
};

enum DrawingOperationType{
    Stroke,Fill,Clear
};

struct DrawingData{
    DrawingOperationType type;
    // Normalized canvas coordinates: 0.0 to 1.0
    float x1 = 0.f;
    float y1 = 0.f;

    float x2 = 0.f;
    float y2 = 0.f;

    // Drawing properties
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    float thickness = 1.f;
};

struct IncomingMessage{
    Player* player;
    PacketType packetType;
    sf::Packet packet;
};

