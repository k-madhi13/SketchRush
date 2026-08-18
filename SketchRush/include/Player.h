#pragma once
#include <string>
#include <SFML/Network.hpp>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "CommunicationUtilities.h"

// This class handles the players from the server side
class Player{

private:

    // Player Name
    std::string playerName;

    bool isAdmin;

    bool isDrawer;
    bool canChat;

    bool connected;

    bool hasGuessed;
    
    sf::TcpSocket* clientSocket;

    // Threads for managing input and output
    std::thread sendThread;
    std::thread receiveThread;

    // Receive queue
    std::queue<IncomingMessage>& receiveQueue;

    // Mutex for writing into the receiveQueue
    std::mutex& receiveMutex;

    // Sending
    std::queue<sf::Packet> sendQueue;
    std::mutex sendMutex;
    std::condition_variable sendCV;

    // Player ID
    int playerId;

    int gameScore=0;

    int timesDrawn = 0;


public:
    Player(int id, sf::TcpSocket* clientSocket, std::queue<IncomingMessage>& recvQueue, std::mutex& recvMutex);

    void SetAdmin(bool admin){
        isAdmin = admin;
    }

    std::string PlayerName(){
        return playerName;
    }

    int GetId(){
        return playerId;
    }

    void ReceiveThread();
    void SendThread();

    bool IsAdmin(){
        return isAdmin;
    }

    void Send(sf::Packet packet);

    ~Player();

    void AddScore(int points){
        gameScore += points;
    }
    void ClearScore(

    ){
        gameScore = 0;
    }
    int GetScore(){
        return gameScore;
    }

    int TimesDrawn(){
        return timesDrawn;
    }

    std::string GetName(){
        return playerName;
    }

    void ClearTimesDrawn(){
        timesDrawn = 0;
    }

    void AddTimesDrawn(){
        timesDrawn++;
    }

    void SetDrawer(bool drawer){
        isDrawer = drawer;
    }
    
    bool IsDrawer(){
        return isDrawer;
    }

    void SetName(std::string name){
        playerName = name;
    }

    bool HasGuessed(){
        return hasGuessed;
    }

    void SetGuessed(bool guessed){
        hasGuessed = guessed;
    }

};