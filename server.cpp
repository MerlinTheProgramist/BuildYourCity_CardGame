#include "card.h"
#include "engine.h"
#include "net_frame/server.h"
#include "network_common.h"

#include <net/if.h>
#include <unordered_map>
#include <vector>
#include <iostream>

class GameServer : net_frame::server_interface<GameMsg>
{  
private:
  struct client{
    uint32_t id;
    // info provided by the client
    PlayerInfo info;
    // game related data
    Player& player;
  };
  
  
public:
  GameServer(uint16_t port, int player_number=2)
  : net_frame::server_interface<GameMsg>(port)
  , engine(player_number)
  {}

  
private:
  // Netrowking
  ServerStatus status{ServerStatus::UNAVALIABLE};
  
  std::unordered_map<uint32_t, client> clientRoster{};
  std::vector<uint32_t> garbageIDs{};

  // Gameplay
  GameEngine engine;

  
protected:
  bool OnClientConnect(std::shared_ptr<Connection> client)
  override
  {
    // Return if clinet can be conected
    return (status == ServerStatus::WAITING_FOR_PLAYERS);  
  }

  void OnClientValidated(std::shared_ptr<Connection> client)
  override
  {
    // Send a custom message that tells the client he, can now communicate
    message msg;
    msg.header.id = GameMsg::Client_Accepted;
    client->Send(msg);
  }

  void OnClientDisconnect(std::shared_ptr<Connection> client)
  override
  {
    if(!client) return;

    // if client present in roster
    if(clientRoster.find(client->GetID()) != clientRoster.end())
    {
        auto& pd = clientRoster[client->GetID()];
        std::cout << "[UNGRECEFULL REMOVAL]: " << pd.id << std::endl;
        clientRoster.erase(client->GetID());
        garbageIDs.push_back(client->GetID());
    }
  }

  void OnMessage(std::shared_ptr<Connection> client, message& msg)
  override
  {
    // If there are some clients that had disconnected
    // remind all clients that a client had disconnected
    if(!garbageIDs.empty())
    {
      for(auto pid : garbageIDs)
      {
        message m;
        m.header.id = GameMsg::Server_RemovePlayer;
        m << pid;
        std:: cout << "Removing: " << pid << std::endl;
        MessageAllClients(m);
      }
      garbageIDs.clear();        
    }

    switch(msg.header.id)
    {
      case GameMsg::Clinet_RegisterWithServer:
      {
        // receive client info from his message
        PlayerInfo info; 
        clientRoster[client->GetID()].id = client->GetID();
        msg >> clientRoster[client->GetID()];

        // inform other players about this client joining the lobby
        message msgAddPlayer{GameMsg::Server_AddPlayer};
        msgAddPlayer << info;
        MessageAllClients(msgAddPlayer, client);

        // Inform this player about the other players that have already been in the lobby
        for(const auto& cli : clientRoster)
        {
          message msgAddExistingPlayer{GameMsg::Server_AddPlayer};
          msgAddExistingPlayer << cli;
          MessageClient(client, msgAddExistingPlayer);    
        }
      }
      break;

      case GameMsg::Clinet_UnreginsterWithServer:
      {
        if(clientRoster.find(client->GetID()) != clientRoster.end())
        {
            auto& pd = clientRoster[client->GetID()];
            std::cout << "[Client quit]: " << pd.id << std::endl;
            clientRoster.erase(client->GetID());
            garbageIDs.push_back(client->GetID());
        }
      }
      break;

      /***********************/
      // GameEngine interface
      /***********************/
      case GameMsg::Game_SelectCard:
      {
        
        msg >>     
      }

      
      // Unexpected for a server
      case GameMsg::Client_Accepted:
      case GameMsg::Client_AssignID:
      case GameMsg::Server_AddPlayer:
      case GameMsg::Server_RemovePlayer:
      // case GameMsg::Clinet_LobbyFull:
      {
          std::cout << "[Server] Received unexpected request ["<< msg.header.id <<"] from: "<< client->GetID() << std::endl;
      }
      break;
    }
    
  }
};
