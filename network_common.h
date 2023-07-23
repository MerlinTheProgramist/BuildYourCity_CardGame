#include <cstdint>
#include <string>

struct PlayerInfo
{
  std::string nickname;
};

enum class ServerStatus{
  UNAVALIABLE,
  WAITING_FOR_PLAYERS,
  GAME_IN_PROGRESS,
};

enum GameMsg : uint32_t{
  // Connection management
  Client_Accepted,
  Client_AssignID,
  Clinet_RegisterWithServer,
  Clinet_UnreginsterWithServer,

  // informative
  // Server_GetStatus,
  // Clinet_LobbyFull,
  // Clinet_GameStarted,
  Server_RemovePlayer,
  Server_AddPlayer,

  // Game actions
  Game_SelectCard,
  Game_BuildCard,
  Game_Discard,
  Game_CanProgress,
  Game_Progress,

  // Game info from server
  Game_Player_Built,
  Game_GetState,
};

