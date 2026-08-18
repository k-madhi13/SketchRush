#include "Player.h"
#include <thread>
#include "CommunicationUtilities.h"

// Data communicated between the server and client has been structured in the CommunicationUtilities header

Player::Player(int id, sf::TcpSocket* socket, std::queue<IncomingMessage>& recvQueue, std::mutex& recvMutex): clientSocket(socket),
    receiveMutex(recvMutex),
    receiveQueue(recvQueue),
    playerId(id)
{
    isAdmin = false;

    canChat = true;
    isDrawer = false;

    connected = true;

    sendThread = std::thread(&Player::SendThread,this);
    receiveThread = std::thread(&Player::ReceiveThread,this);


}

Player::~Player(){

    // Avoid dangling
    sendCV.notify_one();

    if(sendThread.joinable()){
        sendThread.join();
    }
    if(receiveThread.joinable()){
        receiveThread.join();
    }
}

void Player::Send(sf::Packet packet){
    {
        std::lock_guard<std::mutex> lock(sendMutex);
        sendQueue.push(std::move(packet));
    }
    sendCV.notify_one();
}

void Player::SendThread(){
    while(connected){

        std::unique_lock<std::mutex> lock(sendMutex);
        sendCV.wait(lock,[this]{
            return !sendQueue.empty() || !connected;
        });

        if(!connected && sendQueue.empty()){
            break;
        }

        sf::Packet packet = std::move(sendQueue.front());
        sendQueue.pop();

        lock.unlock();

        if (clientSocket->send(packet) != sf::Socket::Status::Done)
        {
            connected = false;
            break;
        }
    }
}

void Player::ReceiveThread()
{
    while (connected)
    {
        sf::Packet packet;

        auto status = clientSocket->receive(packet);

        IncomingMessage message;
        message.player = this;

        if (status != sf::Socket::Status::Done)
        {
            connected = false;
            message.packetType = PacketType::PlayerLeave;
        }
        else
        {
            int type;
            packet >> type;

            message.packetType = static_cast<PacketType>(type);
            message.packet = std::move(packet);
        }

        {
            std::lock_guard<std::mutex> lock(receiveMutex);
            receiveQueue.push(std::move(message));
        }

        if (!connected)
            break;
    }
}

