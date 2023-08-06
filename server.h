#include "card.h"
#include "engine.h"
#include "net_frame/server.h"
#include "network_common.h"

#include <net/if.h>
#include <unordered_map>
#include <vector>
#include <iostream>

class GameServer : public net_frame::server_interface<GameMsg>
{  
  
  
public:
  GameServer(uint16_t port, size_t maxPlayerCount=2)
  : net_frame::server_interface<GameMsg>(port)
  , engine(maxPlayerCount)
  , maxPlayerCount(maxPlayerCount)
  {
    Start();
    updateThread = std::thread([&]()
    {
      for(;;) Update(-1, true);  // wait for next message
    });
  }

  
private:
  // uodate loop thread
  std::thread updateThread;
  
  size_t maxPlayerCount;
  // Netrowking
  ServerStatus status{ServerStatus::WAITING_FOR_PLAYERS};
  
  std::unordered_map<uint32_t, Client> clientRoster{};
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
    std::cout << "Client validated call" << std::endl;
    // Send a custom message that tells the client he, can now communicate
    message msg{GameMsg::Client_Accepted};
    // MessageClient(client, msg);
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
      case GameMsg::Client_RegisterWithServer:
      {
        
        // receive client info from his message
        PlayerInfo newInfo{}; 
        msg >> newInfo.nickname;

        std::cout << "[SERVER] Player with nick: " << newInfo.nickname.data() << std::endl;

        // inform other players about this client joining the lobby
        message msgAddPlayer{GameMsg::Server_AddPlayer};
        msgAddPlayer << newInfo.nickname << client->GetID();
        MessageAllClients(msgAddPlayer, client);

        // Inform this player about the other players that have already been in the lobby
        for(const auto& cli : clientRoster)
        {
          message msgAddExistingPlayer{GameMsg::Server_AddPlayer};
          msgAddExistingPlayer << cli.second.info.nickname << client->GetID();
          MessageClient(client, msgAddExistingPlayer);    
        }

        // bind to that client 
        clientRoster[client->GetID()].id = client->GetID();
        clientRoster[client->GetID()].info = newInfo;
        clientRoster[client->GetID()].client = client;

        // check if server is full
        if(clientRoster.size() == maxPlayerCount)
            StartGame();
      }
      break;

      case GameMsg::Client_UnreginsterWithServer:
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

      // Select card
      case GameMsg::Game_SelectCard:
      {
        Player& player = *clientRoster.at(client->GetID()).player; 
          
        cardIdT selectedId;
        msg >> selectedId;

        // Check if id is valid
        if(selectedId >= player.handDeck.size()){
          std::cout << "[SERVER] Received cardId out of bounds ("<<selectedId<<"), from ["<< client->GetID() <<"]" << std::endl;
          return;
        }
        
        player.select_card(selectedId);
      }

      // no contents, server sends boolean
      // case GameMsg::Game_CanProgress:
      // {
      //   Player& player = clientRoster.at(client->GetID()).player; 
      //    bool canProgress = player.can_progress();
      //    msg << canProgress;
      //    MessageClient(client, msg);
      // }

      case GameMsg::Game_Progress:
      {
        Player& player = *clientRoster.at(client->GetID()).player; 
        player.progress();

        message updateState{GameMsg::Game_PlayerState};
        updateState << player.get_state();
        MessageClient(client, updateState);
      }
      break;
      
      // Unexpected for a server
      case GameMsg::Game_ClientState:
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

  // called when game can be started 
  void StartGame()
  {
    // map player pointers from the engine to clientRoster with proper ids
    // so they can be accessed easly later
    for(int index{};index<clientRoster.size();++index)
    {
      // bind client to player
      auto id = clientRoster.begin();
      std::advance(id, index);
      clientRoster[id->first].player = &engine.getPlayer(index);

      // Deal cards to clients
      message startingCards{GameMsg::Game_DealCards};
      startingCards << std::vector<size_t>{};//clientRoster[id->first].player->hand_view().get_card_ids(engine.masterSet); 
      MessageClient(
        clientRoster[id->first].client.lock(), 
        startingCards
      );
    }

    // Reedem all players that the game has officially started
    message startPlaying{GameMsg::Game_ClientState};
    startPlaying << ClientState::PLAYING;
    MessageAllClients(startPlaying);

    for(auto client : clientRoster)
    {
      for(cardIdT cardId : client.second.player->handDeck.get_card_ids(GameEngine::masterSet))
      {
        message dealCard{GameMsg::Game_DealCards};
        dealCard << cardId;
        MessageClient(client.second.client.lock(), dealCard);
      }
    }
  }
};