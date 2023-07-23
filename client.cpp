#include <raylib.h>

// UI
#include <imgui.h>
#include "libraries/rlImGui/rlImGui.h"

#include "net_frame/client.h"
#include "network_common.h"

namespace ImGui{
  void inline SetNextPosCenter(const size_t& size) {
      auto windowWidth = ImGui::GetWindowSize().x;
      auto textWidth   = size;

      ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
  }
}

class GameClient : net_frame::client_intefrace<GameMsg>
{
  enum class ClientState
  {
    IN_MENU,
    IN_LOBBY,
    PLAYING,
  } state;

  std::string ui_ip_address{};
  std::string ui_port{};

public:
  GameClient()
  {
    ui_ip_address.resize(sizeof("000.000.000.000"));
    ui_port.reserve(sizeof("0000"));

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(100, 100, "Game");
    SetTargetFPS(60);
    rlImGuiSetup(true);

    ImGuiIO& io = ImGui::GetIO();
    
    while(!WindowShouldClose())
    {
      BeginDrawing();
        Update(GetFrameTime());
        rlImGuiBegin();
        DrawUI();
        rlImGuiEnd();
        DrawFPS(10, 10);
      EndDrawing();
    }

    Disconnect();
    rlImGuiShutdown();
  }

private:
  void DrawUI()
  {
    static ImGuiWindowFlags flags = 
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
    	| /*ImGuiWindowFlags_NoResize |*/ ImGuiWindowFlags_NoMove
    	| ImGuiWindowFlags_NoScrollbar /*| ImGuiWindowFlags_NoSavedSettings*/
      | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;
    bool open = true;
    ImGui::SetNextWindowPos({300,300});
    ImGui::Begin("Client", &open, flags);
    
    // ImGui::SetNextPosCenter(ImGui::CalcTextSize("The Game").x);
    ImGui::Text("The Game!");
    ImGui::InputTextWithHint("server ip address", "255.255.255.255", &ui_ip_address[0], sizeof("000.000.000.000")); 
    ImGui::InputTextWithHint("server port", "6000", &ui_port[0], sizeof("0000"));
    
    ImGui::End();       
  }
  
  void Update(float deltaTime)
  {
    ClearBackground(DARKGRAY);
    
  }
};

int main()
{
  GameClient client{};

  return 0;
}