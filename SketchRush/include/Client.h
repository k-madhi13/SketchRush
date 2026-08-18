#pragma once

#include <SFML/Network.hpp>

#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "CommunicationUtilities.h"
#include "Gamescreen.h"

class Client
{
private:
    static constexpr unsigned int PORT = 55555;

    std::optional<sf::IpAddress> deviceIpAddress;

    sf::TcpSocket socket;

    std::atomic<bool> connected{ false };

    std::string name;

    // Receiving
    std::queue<IncomingMessage> receiveQueue;
    std::mutex receiveMutex;

    // Sending
    std::queue<sf::Packet> sendQueue;
    std::mutex sendMutex;
    std::condition_variable sendCV;

    std::thread receiveThread;
    std::thread sendThread;

    


public:
    Client();
    ~Client();

    std::string GetIpString()
    {
        return deviceIpAddress->toString();
    }

    bool Connect(sf::IpAddress serverIp, std::string name);

    void Send(sf::Packet packet);

    bool GetNextMessage(IncomingMessage& message);

    void Disconnect();

    bool IsConnected() const{
        return connected;
    };

    void SendChatMessage(std::string message);

    void SendDrawingData(const DrawingData& data);

private:
    void ReceiveThread();
    void SendThread();
};