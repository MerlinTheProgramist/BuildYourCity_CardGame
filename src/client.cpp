#include <cstdint>
#include <raylib.h>
#include <imgui.h>
#include <sys/types.h>
#include <unordered_map>

#include "../libraries/rlImGui/rlImGui.h"
#include "../libraries/sonicpp/library/client.h"

#include "engine/engine.h"
#include "network_common.h"

#include "ui.h"
#include "server.h"

class GameClient : sonicpp::ClientIntefrace<GameMsg>
{
   
public:
  GameClient()
  : sonicpp::ClientIntefrace<GameMsg>{}
  {
    ui_ip_address.resize(sizeof("000.000.000.000"));
    ui_port.reserve(sizeof("0000"));
    ui_server_port.reserve(sizeof("0000"));
    ui_max_players.reserve(2);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(100, 100, "Game");
    SetTargetFPS(60);
    rlImGuiSetup(true);

    ImGuiIO& io = ImGui::GetIO();

    // Load Textures
    LoadTextures();
    
    while(!WindowShouldClose())
    {
      CheckMsg();
      BeginDrawing();
        Update(GetFrameTime());
        rlImGuiBegin();
        DrawUI();
        rlImGuiEnd();
        // DrawFPS(10, 10);
      EndDrawing();
    }

    // Notify server that we are disconnecting
    Message disconnect{GameMsg::Client_UnreginsterWithServer};
    Send(disconnect);
    
    Disconnect();
    rlImGuiShutdown();

    
    CloseWindow();
  }
private:
  bool awaiting_server{false};
  ClientState state{ClientState::IN_MENU};
  
  GameServer* gameServerInstance{};

  // GameState
  id_t my_id;
  Player player;
  PlayerInfo info;
  
  std::unordered_map<uint32_t, EnemyView> otherPlayers;
  struct
  {
    int masterDeckSize{};
    int distardDeckSize{};
  }publicInfo;
  

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
            info.nickname = std::string(ui_nickname);
            if(Connect(ui_ip_address, static_cast<uint16_t>(std::stoi(ui_port))))
              state = ClientState::CONNECTING;
        }
        ImGui::InputTextWithHint("host port", "60000", &ui_server_port[0], sizeof("00000"));
        ImGui::InputTextWithHint("max players", "2", &ui_max_players[0], 2); // one digit count
          
        if(ImGui::Button("Start server"))
        {
          info.nickname = std::string(ui_nickname);
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
      case ClientState::READY:
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

    if(state != ClientState::PLAYING) return;

    // Draw the acctual game
    const Vector2 screenCenter = {(float)GetScreenWidth()/2,(float)GetScreenHeight()/2};

    DrawCardPile(
        {(float)GetScreenWidth()-CARD_SIZE.x*1.5f,screenCenter.y-CARD_SIZE.y*0.5f},
        std::min(4, publicInfo.masterDeckSize)
    );

    DrawCardPile(
        {(float)GetScreenWidth()-CARD_SIZE.x*3.5f,screenCenter.y-CARD_SIZE.y*0.5f},
        std::min(4, publicInfo.distardDeckSize)
    );
    const CardType* zoom = ShowHand(
        {screenCenter.x, GetScreenHeight()-CARD_SIZE.y/2-20},
        -40, 
        player
    );
    {
        auto new_zoom = ShowBuilt(
            {screenCenter.x, screenCenter.y - CARD_SIZE.y/2},
            10,
            player
        );
        zoom = (zoom==nullptr)?new_zoom:zoom;
    
        if(player.get_state() == PlayerState::RESIGN_BONUS_SELECT)
            new_zoom = ShowEventSelect(screenCenter, -10, player);
        zoom = (zoom==nullptr)?new_zoom:zoom;
    }

    
    if(zoom!=nullptr)
        DrawZoom(zoom);

    if(awaiting_server)
      DrawGameUI(player);
    else
      switch(DrawGameUI(player))
      {
        case NOTHING: break;

        case PROGESS: 
        {
            awaiting_server = true;
            Message progressRequest{GameMsg::Game_Progress};

            progressRequest << player.handDeck.get_selected().get_card_ids(GameEngine::masterSet);
            Send(progressRequest);
        }
        break;
        case PASS: 
        {
            awaiting_server = true;
            Message passRequest{GameMsg::Game_Pass};
            Send(passRequest);
        }
        break;
      }
  }
private:
  void CheckMsg()
  {
    // Check for incoming messages
    if(IsConnected())
    {
      while(auto msg_opt = NextMessage())
      {
        Message& msg = *msg_opt;

        switch(msg.GetType())
        {
          case GameMsg::Client_Accepted:
          {
            state = ClientState::IN_LOBBY;
            std::cout << "Server Accepted you!" << std::endl;
            Message msgSend{GameMsg::Client_RegisterWithServer};
            std::cout << "Sending nickname: " << info.nickname.data() << " size:" << info.nickname.size() <<std::endl;
            msgSend << info.nickname;
            Send(msgSend);
          }
          break;
          case GameMsg::Client_AssignID:
          {
            msg >> my_id;
            std::cout << "Server provided your id: " << my_id << std::endl;
          }
          break;
          case GameMsg::Server_ClientState:
          {
            msg >> state;
            std::cout << "Game state updated to: " << (int)(state) << std::endl;
          }
          break;
          // Server allowed you to progress
          case GameMsg::Game_Progress:
          {
            awaiting_server = false;
            player.progress_light();
          }
          break;
          case GameMsg::Server_RemovePlayer:
          {            
            uint32_t id;
            msg >> id;
            if(id == my_id)
            {
              std::cout << "[Client] Error, Server removed me!" << std::endl;
              state = ClientState::IN_MENU;
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

          // Gameplay related
          case GameMsg::Game_DealCards:
          {
              std::vector<cardIdT> cardTypes;
              msg >> cardTypes;
              std::cout << "Server gave me cards num: "<< cardTypes.size() << std::endl;
              for(cardIdT id : cardTypes)
                player.handDeck.add(id, GameEngine::masterSet);   
          }
          break;

          default:
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
