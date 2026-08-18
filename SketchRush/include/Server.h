#pragma once

#include <SFML/Network.hpp>

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>

#include "Player.h"
#include "CommunicationUtilities.h"
#include "Game.h"

class Server
{
private:
    static constexpr unsigned int PORT = 55555;

    std::optional<sf::IpAddress> serverIpAddress;

    sf::TcpListener serverListener;

    std::vector<std::unique_ptr<Player>> connectedPlayers;

    // Shared by all Player receive threads
    std::queue<IncomingMessage> receiveQueue;
    std::mutex receiveMutex;

    // Protects connectedPlayers because the listener thread
    // and server thread can access it
    std::mutex playersMutex;

    std::thread listenerThread;

    int nextPlayerId = 0;

    Game game;
    bool gameRunning = false;

public:
    Server();
    ~Server();

    bool InitializeServerListener();

    bool ListenForClients();
    void StartListening();

    void RemovePlayer(Player* player);

    std::string GetIpString()
    {
        return serverIpAddress->toString();
    }

    void StartGame();
    void UpdateGame(sf::Time deltaTime);

    bool GameRunning(){
        return gameRunning;
    }

    void ProcessMessages();

    void SendLeaderboardUpdate();

    void BroadcastChatMessage(std::string, std::string);

    void BroadcastPacket(sf::Packet packet);

    void SendDrawingData(Player& player, const DrawingData& data);

    

};