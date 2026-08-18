#pragma once

#include <vector>
#include <random>
#include <string>

#include "Player.h"
#include "CommunicationUtilities.h"

enum GameState
{
    Round,
    Results,
    Break,
    Start
};

class Game
{
private:

    // ---------------------------------------------------------
    // Game state
    // ---------------------------------------------------------

    bool resultsTriggered = false;

    std::vector<std::unique_ptr<Player>>& players;

    // ---------------------------------------------------------
    // Words
    // ---------------------------------------------------------

    // Temporary for test
    std::vector<std::string> words =
    {
        "apple",
        "ball",
        "cat",
        "table",
        "chair",
        "glass",
        "monkey",
        "flower",
        "earth",
        "volcano",
        "magic",
        "phone"
    };

    // ---------------------------------------------------------
    // Game clock
    // ---------------------------------------------------------

    sf::Clock gameClock;

    // ---------------------------------------------------------
    // Game settings
    // ---------------------------------------------------------

    sf::Time totalGuessTime = sf::seconds(60);
    sf::Time firstHintTime = sf::seconds(20);
    sf::Time secondHintTime = sf::seconds(40);

    // Time between rounds
    sf::Time timeAfterRound = sf::seconds(2);

    // ---------------------------------------------------------
    // Time tracking
    // ---------------------------------------------------------

    sf::Time timeSinceRoundStart = sf::seconds(0);
    sf::Time timeSinceBreak = sf::seconds(0);

    sf::Time resultsTime = sf::seconds(10);

    // Last value sent to clients through TimeLeft.
    // Prevents sending the same second repeatedly.
    int lastTimeLeftSent = -1;

    // ---------------------------------------------------------
    // Hints
    // ---------------------------------------------------------

    bool firstHintGiven = false;
    bool secondHintGiven = false;

    int firstHintIndex = -1;
    int secondHintIndex = -1;

    // ---------------------------------------------------------
    // Miscellaneous timing
    // ---------------------------------------------------------

    sf::Time elapsedTimeAtLastTick;

    // ---------------------------------------------------------
    // Drawer
    // ---------------------------------------------------------

    Player* drawer = nullptr;

    // Meaning no one is drawing
    int currentDrawerIndex = -1;

    // ---------------------------------------------------------
    // Current word
    // ---------------------------------------------------------

    std::string currentWord;

    // ---------------------------------------------------------
    // Randomizer
    // ---------------------------------------------------------

    std::random_device randomDevice;
    std::mt19937 randomGenerator;
    std::uniform_int_distribution<int> dist;

    // ---------------------------------------------------------
    // Round information
    // ---------------------------------------------------------

    int numberOfRounds = 2;
    int currentRound = 1;

    enum GameState gameState = Start;

    bool everyoneGuessed = false;

public:

    Game(std::vector<std::unique_ptr<Player>>& connectedPlayers);

    // ---------------------------------------------------------
    // Game control
    // ---------------------------------------------------------

    void StartGame();
    void Update(sf::Time deltaTime);

    void StartRound();
    bool GetNextDrawer();

    // ---------------------------------------------------------
    // Results
    // ---------------------------------------------------------

    void TriggerResults();

    // ---------------------------------------------------------
    // Guessing
    // ---------------------------------------------------------

    bool CheckGuess(const std::string& guess)
    {
        return guess == currentWord;
    }

    bool ProcessGuess(
        Player* player,
        const std::string& guess
    );

    void CheckEveryoneGuessed();

    // ---------------------------------------------------------
    // Hints
    // ---------------------------------------------------------

    void GiveHint();
    void SendHint();

    // ---------------------------------------------------------
    // Communication
    // ---------------------------------------------------------

    void SendRoundEnd();

    void SendChatMessage(
        const std::string& text
    );

    void BroadcastPacket(
        sf::Packet packet
    );

    void SendLeaderboardUpdate();

    bool GameFinished(){
        return gameState == GameState::Results;
    }

    bool CanFinishGame(){
        return resultsTriggered;
    }
};
