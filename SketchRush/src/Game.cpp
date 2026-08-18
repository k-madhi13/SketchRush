#include "Game.h"
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include "Player.h"
#include "CommunicationUtilities.h"

Game::Game(std::vector<std::unique_ptr<Player>>& connectedPlayers)
    : players(connectedPlayers)
{
    randomGenerator = std::mt19937(randomDevice());

    dist = std::uniform_int_distribution<int>(
        0,
        static_cast<int>(words.size()) - 1
    );

    // Time between rounds
    timeAfterRound = sf::seconds(3.f);
}

void Game::StartGame()
{
    gameState = GameState::Start;

    for (auto& player : players)
    {
        player->ClearScore();
        player->ClearTimesDrawn();
        player->SetDrawer(false);
        player->SetGuessed(false);
    }

    timeSinceBreak = sf::seconds(0);
    timeSinceRoundStart = sf::seconds(0);

    currentRound = 1;
    currentDrawerIndex = -1;
    drawer = nullptr;

    firstHintGiven = false;
    secondHintGiven = false;

    firstHintIndex = -1;
    secondHintIndex = -1;

    everyoneGuessed = false;
    resultsTriggered = false;

    lastTimeLeftSent = -1;
}

// GET NEXT DRAWER

bool Game::GetNextDrawer()
{

    if (currentDrawerIndex == -1)
    {
        if (players.empty())
        {
            return false;
        }

        drawer = players.front().get();
        currentDrawerIndex = drawer->GetId();

        drawer->AddTimesDrawn();

        return true;
    }

    // Find current drawer

    size_t currentIndex = 0;

    for (size_t i = 0; i < players.size(); ++i)
    {
        if (players[i].get() == drawer)
        {
            currentIndex = i;
            break;
        }
    }

    // Move to next player

    size_t nextIndex = currentIndex + 1;

    // We reached the end of the player list.
    //
    // This means EVERY player has drawn once for this round.
    // Move to the next round.

    if (nextIndex >= players.size())
    {
        currentRound++;

        // No more rounds

        if (currentRound > numberOfRounds)
        {
            return false;
        }

        // Start from first player again
        nextIndex = 0;
    }

    // ---------------------------------------------------------
    // Remove drawer status from previous drawer
    // ---------------------------------------------------------

    drawer->SetDrawer(false);

    // ---------------------------------------------------------
    // Set new drawer
    // ---------------------------------------------------------

    drawer = players[nextIndex].get();
    currentDrawerIndex = drawer->GetId();

    drawer->AddTimesDrawn();

    return true;
}

// =============================================================
// SEND SERVER CHAT MESSAGE
// =============================================================

void Game::SendChatMessage(const std::string& text)
{
    sf::Packet packet;

    packet << PacketType::ChatUpdate;
    packet << std::string("Server");
    packet << text;

    BroadcastPacket(packet);
}

// TRIGGER FINAL RESULTS

void Game::TriggerResults()
{
    SendChatMessage("Game over! Final scores:");

    for (auto& player : players)
    {
        SendChatMessage(
            player->GetName() +
            ": " +
            std::to_string(player->GetScore()) +
            " points"
        );
    }

    sf::Packet roundPacket;

    roundPacket << PacketType::RoundNumberUpdate;

    std::string roundString =
        "Round " +
        std::to_string(numberOfRounds) +
        " of " +
        std::to_string(numberOfRounds);

    roundPacket << roundString;

    BroadcastPacket(roundPacket);

    sf::Packet drawerPacket;

    drawerPacket << PacketType::DrawerUpdate;
    drawerPacket << "";

    BroadcastPacket(drawerPacket);

    sf::Packet gameEndPacket;

    gameEndPacket << PacketType::GameEnd;

    BroadcastPacket(gameEndPacket);

    // ---------------------------------------------------------
    // Find winner
    // ---------------------------------------------------------

    int maxScore = 0;
    std::string maxScorePlayer;

    for (auto& player : players)
    {
        if (player->GetScore() > maxScore)
        {
            maxScore = player->GetScore();
            maxScorePlayer = player->GetName();
        }
    }

    if (!maxScorePlayer.empty())
    {
        SendChatMessage(
            maxScorePlayer +
            " won with a score of " +
            std::to_string(maxScore) +
            "."
        );
    }
}

// =============================================================
// UPDATE GAME
// =============================================================

void Game::Update(sf::Time deltaTime)
{
    // =========================================================
    // Waiting before the first round
    // =========================================================

    if (gameState == GameState::Start)
    {
        timeSinceBreak += deltaTime;

        if (timeSinceBreak >= timeAfterRound)
        {
            StartRound();
            timeSinceBreak = sf::seconds(0);
        }

        return;
    }

    // =========================================================
    // Active round
    // =========================================================

    if (gameState == GameState::Round)
    {
        timeSinceRoundStart += deltaTime;

        // -----------------------------------------------------
        // Send remaining time once every second
        // -----------------------------------------------------

        sf::Time remainingTime =
            totalGuessTime - timeSinceRoundStart;

        int remainingSeconds =
            std::max(
                0,
                static_cast<int>(
                    std::ceil(remainingTime.asSeconds())
                )
            );

        if (remainingSeconds != lastTimeLeftSent)
        {
            lastTimeLeftSent = remainingSeconds;

            sf::Packet packet;

            packet << PacketType::TimeLeft;
            packet << remainingSeconds;

            BroadcastPacket(packet);
        }

        // -----------------------------------------------------
        // First hint
        // -----------------------------------------------------

        if (!firstHintGiven &&
            timeSinceRoundStart >= firstHintTime)
        {
            firstHintGiven = true;

            GiveHint();
            SendHint();
        }

        // -----------------------------------------------------
        // Second hint
        // -----------------------------------------------------

        if (!secondHintGiven &&
            timeSinceRoundStart >= secondHintTime)
        {
            secondHintGiven = true;

            GiveHint();
            SendHint();
        }

        // -----------------------------------------------------
        // Round ends
        // -----------------------------------------------------

        if (everyoneGuessed ||
            timeSinceRoundStart >= totalGuessTime)
        {
            gameState = GameState::Break;
            timeSinceBreak = sf::seconds(0);

            // Make sure clients receive 0
            if (lastTimeLeftSent != 0)
            {
                lastTimeLeftSent = 0;

                sf::Packet packet;

                packet << PacketType::TimeLeft;
                packet << 0;

                BroadcastPacket(packet);
            }

            SendRoundEnd();
        }

        return;
    }

    // =========================================================
    // Break between rounds
    // =========================================================

    if (gameState == GameState::Break)
    {
        timeSinceBreak += deltaTime;

        if (timeSinceBreak >= timeAfterRound)
        {
            // -------------------------------------------------
            // All rounds have finished
            // -------------------------------------------------

            if (currentRound > numberOfRounds)
            {
                gameState = GameState::Results;
                return;
            }

            StartRound();

            timeSinceBreak = sf::seconds(0);
        }

        return;
    }

    // =========================================================
    // Results
    // =========================================================

    if (gameState == GameState::Results)
    {
        if (!resultsTriggered)
        {
            TriggerResults();
            resultsTriggered = true;
        }

        return;
    }
}

// =============================================================
// START ROUND
// =============================================================

void Game::StartRound()
{

    sf::Packet clearScreenPacket;
    clearScreenPacket << PacketType::ClearScreen;
    BroadcastPacket(clearScreenPacket);

    // =========================================================
    // Make sure the game hasn't already finished
    // =========================================================

    if (currentRound > numberOfRounds)
    {
        gameState = GameState::Results;
        return;
    }

    // =========================================================
    // Select word
    // =========================================================

    currentWord = words.at(dist(randomGenerator));

    // =========================================================
    // Reset guessing state
    // =========================================================

    for (auto& player : players)
    {
        player->SetGuessed(false);
    }

    everyoneGuessed = false;

    // =========================================================
    // Get next drawer
    // =========================================================

    if (!GetNextDrawer())
    {
        gameState = GameState::Results;
        return;
    }

    drawer->SetDrawer(true);

    // =========================================================
    // Reset round timers / hints
    // =========================================================

    timeSinceRoundStart = sf::seconds(0);

    firstHintGiven = false;
    secondHintGiven = false;

    firstHintIndex = -1;
    secondHintIndex = -1;

    lastTimeLeftSent = -1;

    // =========================================================
    // Send round number
    // =========================================================

    sf::Packet roundPacket;

    roundPacket << PacketType::RoundNumberUpdate;

    std::string roundString =
        "Round " +
        std::to_string(currentRound) +
        " of " +
        std::to_string(numberOfRounds);

    roundPacket << roundString;

    BroadcastPacket(roundPacket);

    // =========================================================
    // Tell everyone who is drawing
    // =========================================================

    sf::Packet drawerPacket;

    drawerPacket << PacketType::DrawerUpdate;
    drawerPacket << drawer->GetName();

    BroadcastPacket(drawerPacket);

    // =========================================================
    // Give actual word to drawer
    // =========================================================

    sf::Packet wordPacket;

    wordPacket << PacketType::WordUpdate;
    wordPacket << currentWord;

    drawer->Send(wordPacket);

    // =========================================================
    // Give blank hint to everyone else
    // =========================================================

    sf::Packet hintPacket;

    hintPacket << PacketType::HintUpdate;

    std::string hint;

    for (size_t i = 0; i < currentWord.length(); ++i)
    {
        hint += "_";

        if (i + 1 < currentWord.length())
        {
            hint += " ";
        }
    }

    hintPacket << hint;

    for (auto& player : players)
    {
        if (player.get() != drawer)
        {
            player->Send(hintPacket);
        }
    }

    // =========================================================
    // Send initial timer
    // =========================================================

    int initialTime =
        static_cast<int>(
            std::ceil(totalGuessTime.asSeconds())
        );

    lastTimeLeftSent = initialTime;

    sf::Packet timePacket;

    timePacket << PacketType::TimeLeft;
    timePacket << initialTime;

    BroadcastPacket(timePacket);

    // =========================================================
    // Finally enter round state
    // =========================================================

    gameState = GameState::Round;
}

// =============================================================
// GIVE HINT
// =============================================================

void Game::GiveHint()
{
    if (currentWord.empty())
    {
        return;
    }

    std::uniform_int_distribution<int> charDist(
        0,
        static_cast<int>(currentWord.length()) - 1
    );

    // ---------------------------------------------------------
    // First hint
    // ---------------------------------------------------------

    if (firstHintIndex == -1)
    {
        firstHintIndex =
            charDist(randomGenerator);
    }

    // ---------------------------------------------------------
    // Second hint
    // ---------------------------------------------------------

    else if (secondHintIndex == -1)
    {
        // Can't have two different hints
        // if the word has only one character.
        if (currentWord.length() <= 1)
        {
            return;
        }

        do
        {
            secondHintIndex =
                charDist(randomGenerator);

        } while (secondHintIndex == firstHintIndex);
    }
}

// =============================================================
// SEND HINT
// =============================================================

void Game::SendHint()
{
    std::string hint;

    for (size_t i = 0; i < currentWord.length(); ++i)
    {
        if (static_cast<int>(i) == firstHintIndex ||
            static_cast<int>(i) == secondHintIndex)
        {
            hint += currentWord[i];
        }
        else
        {
            hint += "_";
        }

        if (i + 1 < currentWord.length())
        {
            hint += " ";
        }
    }

    sf::Packet packet;

    packet << PacketType::HintUpdate;
    packet << hint;

    // Drawer already knows the word.
    for (auto& player : players)
    {
        if (player.get() != drawer)
        {
            player->Send(packet);
        }
    }
}

// =============================================================
// PROCESS GUESS
// =============================================================

bool Game::ProcessGuess(
    Player* player,
    const std::string& guess)
{
    // =========================================================
    // Drawer cannot guess
    // =========================================================

    if (player == drawer)
    {
        return false;
    }

    // =========================================================
    // Guess must happen during a round
    // =========================================================

    if (gameState != GameState::Round)
    {
        return false;
    }

    // =========================================================
    // Player can only score once
    // =========================================================

    if (player->HasGuessed())
    {
        return false;
    }

    // =========================================================
    // Check guess
    // =========================================================

    if (guess == currentWord)
    {
        player->SetGuessed(true);

        sf::Packet wordPacket;
        wordPacket << PacketType::WordUpdate;
        wordPacket << currentWord;
        player->Send(wordPacket);

        // =====================================================
        // Calculate score based on speed
        // =====================================================

        float progress =
            timeSinceRoundStart.asSeconds() /
            totalGuessTime.asSeconds();

        progress = std::clamp(progress, 0.0f, 1.0f);

        // Maximum = 200
        int score =
            static_cast<int>(
                200.0f * (1.0f - progress)
            );

        // Minimum score for a correct guess
        score = std::max(score, 10);

        player->AddScore(score);

        // =====================================================
        // Tell everyone
        // =====================================================

        SendChatMessage(
            player->GetName() +
            " guessed the word! +" +
            std::to_string(score) +
            " points"
        );

        // =====================================================
        // Check whether everyone guessed
        // =====================================================

        CheckEveryoneGuessed();

        return true;
    }

    // Wrong guess
    return false;
}

// =============================================================
// CHECK EVERYONE GUESSED
// =============================================================

void Game::CheckEveryoneGuessed()
{
    for (auto& player : players)
    {
        // Drawer doesn't need to guess
        if (player.get() == drawer)
        {
            continue;
        }

        // Someone hasn't guessed yet
        if (!player->HasGuessed())
        {
            return;
        }
    }

    // Every non-drawer guessed correctly
    everyoneGuessed = true;
}

// =============================================================
// SEND ROUND END
// =============================================================

void Game::SendRoundEnd()
{
    // ---------------------------------------------------------
    // Tell everyone how the round ended
    // ---------------------------------------------------------

    if (everyoneGuessed)
    {
        SendChatMessage(
            "Everyone guessed the word! Round ended. The word was " +
            currentWord
        );
    }
    else
    {
        SendChatMessage(
            "Time's up! The word was: " +
            currentWord
        );
    }

    // ---------------------------------------------------------
    // Send current scores
    // ---------------------------------------------------------

    SendChatMessage("Current scores:");

    for (auto& player : players)
    {
        SendChatMessage(
            player->GetName() +
            ": " +
            std::to_string(player->GetScore()) +
            " points"
        );
    }

    sf::Packet wordPacket;
    wordPacket << PacketType::WordUpdate;
    wordPacket << currentWord;
    BroadcastPacket(wordPacket);

    SendLeaderboardUpdate();
}

// =============================================================
// BROADCAST PACKET
// =============================================================

void Game::BroadcastPacket(sf::Packet packet)
{
    for (auto& player : players)
    {
        player->Send(packet);
    }
}

// =============================================================
// SEND LEADERBOARD
// =============================================================

void Game::SendLeaderboardUpdate()
{
    sf::Packet packet;

    packet << PacketType::LeaderboardUpdate;
    packet << static_cast<int>(players.size());

    for (const auto& player : players)
    {
        std::string playerName = player->PlayerName();

        if (player->IsAdmin())
        {
            playerName += " (Admin) ";
        }

        packet << playerName;
        packet << player->GetScore();
    }

    BroadcastPacket(packet);
}