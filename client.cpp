#include <cstdint>
#include <raylib.h>
#include <memory>
// UI
#include <imgui.h>
#include <unordered_map>
#include "libraries/rlImGui/rlImGui.h"

#include "net_frame/client.h"
#include "net_frame/server.h"
#include "network_common.h"

#include "server.h"

class GameClient : net_frame::client_intefrace<GameMsg>
{
public:
  GameClient()
  {
    ui_ip_address.resize(sizeof("000.000.000.000"));
    ui_port.reserve(sizeof("0000"));
    ui_server_port.reserve(sizeof("0000"));
    ui_max_players.reserve(1);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(100, 100, "Game");
    SetTargetFPS(60);
    rlImGuiSetup(true);

    ImGuiIO& io = ImGui::GetIO();
    
    while(!WindowShouldClose())
    {
      CheckMsg();
      BeginDrawing();
        Update(GetFrameTime());
        rlImGuiBegin();
        DrawUI();
        rlImGuiEnd();
        DrawFPS(10, 10);
      EndDrawing();
    }

    // Notify server that we are disconnecting
    message disconnect{GameMsg::Client_UnreginsterWithServer};
    Send(disconnect);
    
    Disconnect();
    rlImGuiShutdown();
  }
private:
  ClientState state{ClientState::IN_MENU};
  
  GameServer* gameServerInstance{};

  // GameState
  Client myPlayer{};
  std::unordered_map<uint32_t, EnemyView> otherPlayers;

private:
  char ui_nickname[255];
  std::string ui_ip_address{"localhost"};
  std::string ui_port{"60000"};
  std::string ui_server_port{"60000"};
  std::string ui_max_players{"2"};
  void DrawUI()
  {
      
    static ImGuiWindowFlags flags = 
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
    	| /*ImGuiWindowFlags_NoResize |*/ ImGuiWindowFlags_NoMove
    	| ImGuiWindowFlags_NoScrollbar /*| ImGuiWindowFlags_NoSavedSettings*/
      | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;
    bool open = true;

    switch(state)
    {
      case ClientState::IN_MENU:
      {
        ImGui::SetNextWindowPos({300,300});
        ImGui::Begin("Menu", &open, flags);
    
        ImGui::Text("The Game!");
        ImGui::InputTextWithHint("Nickname", "player1", ui_nickname, sizeof(ui_nickname)); 
        ImGui::InputTextWithHint("server address", "255.255.255.255", &ui_ip_address[0], sizeof("000.000.000.000")); 
        ImGui::InputTextWithHint("server port", "60000", &ui_port[0], sizeof("00000"));
        if(ImGui::Button("Connect"))
        {
            myPlayer.info.nickname = std::string(ui_nickname);
            if(Connect(ui_ip_address, static_cast<uint16_t>(std::stoi(ui_port))))
              state = ClientState::CONNECTING;
        }
        ImGui::InputTextWithHint("host port", "60000", &ui_server_port[0], sizeof("00000"));
        ImGui::InputTextWithHint("max players", "2", &ui_max_players[0], 1); // one digit count
          
        if(ImGui::Button("Start server"))
        {
          myPlayer.info.nickname = std::string(ui_nickname);
          uint16_t port = static_cast<uint16_t>(std::stoi(ui_server_port));
          size_t maxPlayers = static_cast<size_t>(std::stoi(ui_max_players));

          std::cout << "Hosting on port: " << port << std::endl;
          // Create server and connect to it
          gameServerInstance = new GameServer(port, maxPlayers);
          if(Connect("localhost", port))
            state = ClientState::CONNECTING;
        }
        
        ImGui::End();
      }
      break;

      case ClientState::CONNECTING:
      break;
                
      case ClientState::IN_LOBBY:
      {
        ImGui::SetNextWindowPos({300,300});
        ImGui::Begin("Lobby", &open, flags);

        // List all connected players
        for(int i=0;i<otherPlayers.size();++i)
          ImGui::Text("%d. %s",i, otherPlayers[i].info.nickname.c_str());
                
        ImGui::End();
          
      }
      break;
      case ClientState::PLAYING:
      {
        
      }
      break;
    }
  }
  
  void Update(float deltaTime)
  {

    if(state == ClientState::CONNECTING)
    {
      ClearBackground(BLUE);
      DrawText("Connecting", 0, 0, 50, WHITE);
      return;
    }

    if(state == ClientState::IN_LOBBY)
    {
      ClearBackground(RED);
      return;
    }

    
    ClearBackground(DARKGRAY);

    // Draw the accual game

  }
private:
  void CheckMsg()
  {
    // Check for incoming messages
    if(IsConnected())
    {
      while(!Incoming().is_empty())
      {
        auto msg = Incoming().pop_front().msg;

        switch(msg.header.id)
        {
          case GameMsg::Client_Accepted:
          {
            state = ClientState::IN_LOBBY;
            std::cout << "Server Accepted you!" << std::endl;
            message msgSend{GameMsg::Client_RegisterWithServer};
            std::cout << "Sending nickname: " << myPlayer.info.nickname.data() << " size:" << myPlayer.info.nickname.size() <<std::endl;
            msgSend << myPlayer.info.nickname;
            Send(msgSend);
          }
          break;
          case GameMsg::Client_AssignID:
          {
            msg >> myPlayer.id;
            std::cout << "Server provided your id: " << myPlayer.id << std::endl;
          }
          break;
          case GameMsg::Game_ClientState:
          {
            msg >> state;
            std::cout << "Game state updated to: " << (int)(state) << std::endl; 
          }
          break;
          case GameMsg::Server_RemovePlayer:
          {            
            uint32_t id;
            msg >> id;
            if(id == myPlayer.id)
            {
              std::cout << "[Client] Error, Server removed me!" << std::endl;
              return;    
            }
            otherPlayers.erase(id);
          }
          break;
          case GameMsg::Server_AddPlayer:
          {
             uint32_t id;
             msg >> id >> otherPlayers[id].info.nickname;
          }
          break;
          
          case GameMsg::Client_RegisterWithServer:
          case GameMsg::Client_UnreginsterWithServer:
            std::cout << "[Client] Recaived unexpexted request ["<< msg.header.id << "] from the server" << std::endl;
          break;
        }
      }
    }
  }
  
};

int main()
{
  GameClient client{};

  return 0;
}