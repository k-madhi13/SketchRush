#include <iostream>
#include <SFML/Network.hpp>

#include "Server.h"
#include "Client.h"
#include "Game.h"

#define NETWORK_ERROR 1


Server::Server():game(connectedPlayers)
{
    // IP address of the computer hosting the server
    serverIpAddress = sf::IpAddress::getLocalAddress();
}


Server::~Server()
{
    // Currently the listener is blocking inside accept(),
    // so this will only work cleanly if the listener thread
    // has already stopped. Will manage later.
    if (listenerThread.joinable())
    {
        listenerThread.join();
    }
}


bool Server::InitializeServerListener()
{
    sf::Socket::Status serverListenerStatus =
        serverListener.listen(PORT);

    if (serverListenerStatus != sf::Socket::Status::Done)
    {
        std::cerr << "Error : Server not listening\n";
        return NETWORK_ERROR;
    }
    else
    {
        std::cerr << "Server : Server listening on "
                  << GetIpString() << '\n';

        return 0;
    }
}


// Runs on the listener thread
bool Server::ListenForClients()
{
    while (true)
    {
        // Player will take ownership of this socket
        sf::TcpSocket* clientSocket = new sf::TcpSocket;

        if (serverListener.accept(*clientSocket)
            != sf::Socket::Status::Done)
        {
            std::cerr << "Error : Error accepting client\n";
            delete clientSocket;

            return NETWORK_ERROR;
        }

        std::cerr << "Server : Client connected\n";

        {
            std::lock_guard<std::mutex> lock(playersMutex);

            connectedPlayers.push_back(
                std::make_unique<Player>(
                    nextPlayerId,
                    clientSocket,
                    receiveQueue,
                    receiveMutex
                )
            );

            nextPlayerId++;

            std::cerr << "Server : Active connections = "
                      << connectedPlayers.size() << '\n';

            if (connectedPlayers.size() == 1)
            {
                connectedPlayers.at(0)->SetAdmin(true);
                // This gives no name because name isnt set it. Later fix this if needed.
                std::cerr << "Server : Player "
                          << connectedPlayers.at(0)->PlayerName()
                          << " is the admin.\n";
            }
        }
    }
}


void Server::StartListening()
{
    listenerThread =
        std::thread(&Server::ListenForClients, this);
}


void Server::RemovePlayer(Player* player)
{
    std::lock_guard<std::mutex> lock(playersMutex);

    bool needNewAdmin = player->IsAdmin();

    for (auto it = connectedPlayers.begin();
         it != connectedPlayers.end();
         ++it)
    {
        if (it->get() == player)
        {
            connectedPlayers.erase(it);
            break;
        }
    }

    if (gameRunning && connectedPlayers.size() < 2)
    {
        gameRunning = false;

        BroadcastChatMessage(
            "Server",
            "Game stopped because there are not enough players."
        );
    }


    // Give admin to another player if the admin left
    if (needNewAdmin && !connectedPlayers.empty())
    {
        connectedPlayers.front()->SetAdmin(true);

        std::cerr << "Server : Player "
                  << connectedPlayers.front()->PlayerName()
                  << " is the new admin.\n";
    }
}

void Server::StartGame(){
    gameRunning = true;
    game.StartGame();
}

void Server::UpdateGame(sf::Time deltaTime){
    game.Update(deltaTime);
    if(game.CanFinishGame()){
        gameRunning = false;
    }
}

int main()
{
    Server server;

    if (server.InitializeServerListener() == NETWORK_ERROR)
    {
        std::cerr << "Error : Server initialization failed\n";
        return NETWORK_ERROR;
    }

    // Start blocking accept() on another thread
    server.StartListening();

    

    sf::Clock clock;

    // FPS
    const float fixedDeltaTime = 1.0f / 30.0f;
    float accumulator = 0.0f;


    while (true){
        if(server.GameRunning()){
            accumulator += clock.restart().asSeconds();
        }
        clock.restart().asSeconds();

        server.ProcessMessages();

        while (accumulator >= fixedDeltaTime && server.GameRunning())
        {
            server.UpdateGame(sf::Time(sf::seconds(fixedDeltaTime)));
            accumulator -= fixedDeltaTime;
        }

        sf::sleep(sf::milliseconds(5));

    }

    return 0;
}


void Server::ProcessMessages()
{
    IncomingMessage message;

    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(receiveMutex);

            if (receiveQueue.empty())
                break;

            message = std::move(receiveQueue.front());
            receiveQueue.pop();
            
        }

        switch (message.packetType)
        {

            case PacketType::PlayerJoin:
            {   
                std::string playerName;
                message.packet >> playerName;
                message.player->SetName(playerName);

                SendLeaderboardUpdate();

                break;
            }

            case PacketType::PlayerLeave:
            {
                std::cerr << "Server: Player "
                          << message.player->PlayerName()
                          << " left\n";

                RemovePlayer(message.player);

                SendLeaderboardUpdate();

                break;
            }

            case PacketType::Chat:
            {
                std::string text;
                message.packet >> text;

                /* For debugging the server
                std::cout << "Player "
                          << message.player->PlayerName()
                          << ": " << text << '\n';
                */

                if(text == "/start" && message.player->IsAdmin() && !gameRunning && connectedPlayers.size()>=2){
                    
                    BroadcastChatMessage("Server", "Game starting....");
                    game.StartGame();
                    gameRunning = true;
                    break;
                }

                if(!game.ProcessGuess(message.player,text)){
                    BroadcastChatMessage(message.player->PlayerName(),text);
                }
                

                break;
            }

            case PacketType::Draw:
            {
                DrawingData data;

                std::uint8_t operationType;

                message.packet >> operationType
                    >> data.x1
                    >> data.y1
                    >> data.x2
                    >> data.y2
                    >> data.r
                    >> data.g
                    >> data.b
                    >> data.a
                    >> data.thickness;

                data.type =
                    static_cast<DrawingOperationType>(operationType);


                if (!message.player->IsDrawer()){
                    break;
                }


                for (auto& otherPlayer : connectedPlayers)
                {
                    if (otherPlayer.get() == message.player)
                        continue;

                    SendDrawingData(*otherPlayer, data);
                }

                break;
            }

            default:
            {
                std::cerr << "Server: Unknown packet type\n";
                break;
            }
        }
    }
}

void Server::BroadcastPacket(sf::Packet packet){
    for(auto& player : connectedPlayers){
        player->Send(packet);
    }
}

void Server::SendLeaderboardUpdate(){
    sf::Packet packet;

    packet << PacketType::LeaderboardUpdate;
    packet << static_cast<int>(connectedPlayers.size());

    for (const auto& player : connectedPlayers)
    {
        std::string playerName = player->PlayerName();
        if(player->IsAdmin()){
            playerName+="(Admin)";
        }
        packet << (playerName);
        packet << player->GetScore();
    }

    BroadcastPacket(packet);
}

void Server::BroadcastChatMessage(std::string name,std::string message){
    sf::Packet packet;

    packet << PacketType::ChatUpdate;
    packet << name << message;

    BroadcastPacket(packet);
}

void Server::SendDrawingData(
    Player& player,
    const DrawingData& data)
{
    sf::Packet packet;

    packet << PacketType::Draw
           << static_cast<std::uint8_t>(data.type)
           << data.x1
           << data.y1
           << data.x2
           << data.y2
           << data.r
           << data.g
           << data.b
           << data.a
           << data.thickness;

    player.Send(packet);
}