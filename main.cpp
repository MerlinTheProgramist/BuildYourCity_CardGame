#include "card.h"
#include "engine.h"

#include <cstddef>
#include <raylib.h>
#include <RaylibOpOverloads.hpp>

#include <unordered_map>

#include "ui.h"

int main()
{
const int screenWidth = GetScreenWidth();
const int screenHeight = GetScreenHeight();
InitWindow(screenWidth, screenHeight, "Build Your City - The card game");
SetConfigFlags(FLAG_VSYNC_HINT);
SetConfigFlags(FLAG_WINDOW_RESIZABLE);
SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

// Layers
// RenderTexture uiLayer = LoadRenderTexture(screenWidth, screenHeight);


// Gameplay setup
GameEngine gameState(1);

// Generate Textures
for(const auto& cardType : gameState.masterSet.cards)
    generateCardTex(cardType.second);

{
    Image cardBackPattern = LoadImage("./textures/cardReverse.png");
    Image cardBackImage = ImageFromImage(cardBackPattern, {0,0,CARD_SIZE.x, CARD_SIZE.y});
    ImageDrawRectangleLines(&cardBackImage, Rectangle{0,0,CARD_SIZE.x, CARD_SIZE.y}, 4, WHITE);
    cardReversImg = LoadTextureFromImage(cardBackImage);   
}


// Main game loop
while (!WindowShouldClose())    // Detect window close button or ESC key
{
    const Vector2 screenCenter = {(float)GetScreenWidth()/2,(float)GetScreenHeight()/2};

    auto& currentPlayer = gameState.getPlayer(0); 

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(GRAY);
    
        DrawCardPile(
            {(float)GetScreenWidth()-CARD_SIZE.x*1.5f,screenCenter.y-CARD_SIZE.y*0.5f},
            std::min(4, gameState.masterDeckSize())
        );

        DrawCardPile(
            {(float)GetScreenWidth()-CARD_SIZE.x*3.5f,screenCenter.y-CARD_SIZE.y*0.5f},
            std::min(4, gameState.discardedDeckSize())
        );
        const CardType* zoom = ShowHand(
            {screenCenter.x, GetScreenHeight()-CARD_SIZE.y/2-20},
            -40, 
            currentPlayer
        );
        {
            auto new_zoom = ShowBuilt(
                {screenCenter.x, screenCenter.y - CARD_SIZE.y/2},
                10,
                currentPlayer
            );
            zoom = (zoom==nullptr)?new_zoom:zoom;
    
            if(currentPlayer.get_state() == PlayerState::RESIGN_BONUS_SELECT)
                new_zoom = ShowEventSelect(screenCenter, -10, currentPlayer);
            zoom = (zoom==nullptr)?new_zoom:zoom;
        }

    
        if(zoom!=nullptr)
            DrawZoom(zoom);
    
        switch(DrawGameUI(currentPlayer))
        {
            case PROGESS:
            {
                currentPlayer.progress(gameState.masterPool, gameState.players);      
            }
            break;
            case PASS:
            {
                currentPlayer.pass();
            }
            break;
            case NOTHING: break;
        }
    
        // switch(currentPlayer.get_state())
        // {
        //     case PlayerState::DISCARD_2:
                    
        //     break;
        //     case PlayerState::SELECT_CARD:
        //     break;
        //     case PlayerState::SELECT_PAYMENT:
        //     break;
        //     case PlayerState::UPDATE_STATS:
        //     break;
        //     default:
        //     break;
        // }
    
    EndDrawing();
    //----------------------------------------------------------------------------------
}

// De-Initialization
//--------------------------------------------------------------------------------------
CloseWindow();        // Close window and OpenGL context
//--------------------------------------------------------------------------------------

return 0;   
}
