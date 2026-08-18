#include <iostream>
#include <SFML/Network.hpp>

#include "Client.h"

#define NETWORK_ERROR 1

Client::Client()
{
    deviceIpAddress = sf::IpAddress::getLocalAddress();
}

bool Client::Connect(sf::IpAddress serverIp, std::string name)
{
    // Make sure there isn't an old connection
    Disconnect();

    sf::Socket::Status connectionStatus =
        socket.connect(serverIp, PORT, sf::seconds(3.f));

    if (connectionStatus != sf::Socket::Status::Done)
    {
        std::cerr << "Error : Error connecting to the server\n";
        return NETWORK_ERROR;
    }

    connected = true;

    receiveThread =
        std::thread(&Client::ReceiveThread, this);

    sendThread =
        std::thread(&Client::SendThread, this);

    sf::Packet joinPacket;
    joinPacket << PacketType::PlayerJoin;
    joinPacket << name;

    Send(joinPacket);

    return 0;
}

void Client::Disconnect()
{
    // Tell both threads to stop
    connected = false;

    // Wake send thread if it is sleeping
    sendCV.notify_all();

    // Disconnecting the socket causes receive()
    // to return, allowing ReceiveThread to finish.
    socket.disconnect();

    // Wait for both threads to finish
    if (sendThread.joinable())
        sendThread.join();

    if (receiveThread.joinable())
        receiveThread.join();

    // Clear old outgoing messages
    {
        std::lock_guard<std::mutex> lock(sendMutex);

        while (!sendQueue.empty())
            sendQueue.pop();
    }

    // Clear old incoming messages
    {
        std::lock_guard<std::mutex> lock(receiveMutex);

        while (!receiveQueue.empty())
            receiveQueue.pop();
    }
}

void Client::Send(sf::Packet packet)
{

    std::cout << "2. SendDrawingData called\n";

    if (!connected)
        return;

    {
        std::lock_guard<std::mutex> lock(sendMutex);
        sendQueue.push(std::move(packet));
    }

    sendCV.notify_one();
}

void Client::SendThread()
{
    while (connected)
    {
        std::unique_lock<std::mutex> lock(sendMutex);

        sendCV.wait(lock, [this]
        {
            return !sendQueue.empty() || !connected;
        });

        if (!connected && sendQueue.empty())
            break;

        if (sendQueue.empty())
            continue;

        sf::Packet packet = std::move(sendQueue.front());
        sendQueue.pop();

        lock.unlock();

        std::cout << "3. SendDrawingData called\n";

        if (socket.send(packet) != sf::Socket::Status::Done)
        {
            connected = false;
            break;
        }
    }
}

void Client::ReceiveThread()
{
    while (connected)
    {
        sf::Packet packet;

        sf::Socket::Status status =
            socket.receive(packet);

        if (status != sf::Socket::Status::Done)
        {
            connected = false;
            break;
        }

        IncomingMessage message;

        int type;
        packet >> type;

        message.packetType =
            static_cast<PacketType>(type);

        message.packet = std::move(packet);

        {
            std::lock_guard<std::mutex> lock(receiveMutex);
            receiveQueue.push(std::move(message));
        }
    }
}

bool Client::GetNextMessage(IncomingMessage& message)
{
    std::lock_guard<std::mutex> lock(receiveMutex);

    if (receiveQueue.empty())
        return false;

    message = std::move(receiveQueue.front());
    receiveQueue.pop();

    return true;
}

Client::~Client()
{
    Disconnect();
}

void Client::SendChatMessage(std::string message){
    // Chat packet just contains the Chat label and the message nothing else
    
    if(!message.empty()){
        sf::Packet chatPacket;
        chatPacket << PacketType::Chat;
        chatPacket << message;

        Send(chatPacket);
    }
    
}

void Client::SendDrawingData(const DrawingData& data)
{

    std::cout << "1. SendDrawingData called\n";

    sf::Packet packet;

    packet << PacketType::Draw
           << static_cast<uint8_t>(data.type)
           << data.x1
           << data.y1
           << data.x2
           << data.y2
           << data.r
           << data.g
           << data.b
           << data.a
           << data.thickness;

    Send(packet);
}