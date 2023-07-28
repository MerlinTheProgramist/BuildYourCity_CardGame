#pragma once

#include <cstdint>
#include <string>
#include "card.h"
#include "net_frame/connection.h"


struct PlayerInfo
{
  std::string nickname{};
};


struct EnemyView
{
  uint32_t id{};
  PlayerInfo info{};
  std::vector<cardIdT> builtCards{};
  size_t handSize{};
};

enum class ClientState
{
  IN_MENU,
  CONNECTING,
  IN_LOBBY,
  READY,    // @TODO
  PLAYING,
};

enum class ServerStatus{
  // UNAVALIABLE,        // If its unavaliable its just dead
  WAITING_FOR_PLAYERS,
  GAME_IN_PROGRESS,
};

enum GameMsg : uint32_t{
  // Connection management
  Client_Accepted,
  Client_AssignID,
  Client_RegisterWithServer,
  Client_UnreginsterWithServer,

  // informative
  // Server_GetStatus,
  // Clinet_LobbyFull,
  // Clinet_GameStarted,
  Server_RemovePlayer,
  Server_AddPlayer,
  Game_ClientState,        //           

  // Game actions
  Game_SelectCard,         // (card id)
  Game_BuildCard,          // (player id, card id)
  Game_Discard,            // (player id)
  // Game_CanProgress,        
  Game_Progress,           // ()
  Game_DealCards,          // (card id)

  // Game info from server
  Game_Player_Built,       // other player built some card (card id)
  Game_Discount,           // (card id, new price)
  Game_PlayerState,
};

struct Client{
  uint32_t id{};
  // info provided by the client
  PlayerInfo info{};
  // game related data
  Player* player{};

  // network client
  std::weak_ptr<net_frame::connection<GameMsg>> client{};
};

