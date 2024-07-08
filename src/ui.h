#include "engine/engine.h"

#include <FileText.hpp>
#include <TextureUnmanaged.hpp>
#include <cstddef>
#include <imgui.h>
#include <raylib-cpp.hpp>

#include <unordered_map>

const raylib::Vector2 CARD_SIZE = {200,300};
const float CARD_ZOOM_SCALE = 2;

static Texture2D cardReversImg;
static std::unordered_map<const CardType*, Texture2D> cardImages;
static void generateCardTex(const CardType& type)
{    
    const int width = CARD_SIZE.x, height = CARD_SIZE.y;
    raylib::Image cardImage(width, height, RAYWHITE);

    const int border = 4;
    cardImage.DrawRectangle(border, border, CARD_SIZE.x-border*2, CARD_SIZE.y-border*2, BLUE);
    
    // green circle with cost
    {
    const int fontSize = 20;
    const int r = 15, b = 2;
    const raylib::Vector2 pos{20,20};
    cardImage.DrawCircle(pos, r+b, BLACK);
    cardImage.DrawCircle(pos, r, GREEN);
    
    cardImage.DrawCircle(width-pos.x, pos.y, r+b, BLACK);
    cardImage.DrawCircle(width-pos.x, pos.y, r, GREEN);

    auto costText = raylib::Text(TextFormat("%i", type.cost), fontSize);
    raylib::Vector2 LabelOffset = costText.Measure()/2.f;
    cardImage.DrawText(costText.GetText(), pos - LabelOffset, fontSize, WHITE);
    cardImage.DrawText(costText.GetText(), raylib::Vector2{CARD_SIZE.x,0} - pos - LabelOffset, fontSize, WHITE);
    }
    
    // Main Label
    {
        const int fontSize = 20;
        int LabelOffset = raylib::MeasureText(type.name, fontSize)/2;
        cardImage.DrawText(type.name, width/2-LabelOffset, 15, fontSize, BLACK);
    } 

    // Symbols
    {    
        int offset =0; 
        const int fontSize = 15;
        for(std::uint8_t i=0;i<type.tags.Transport;++i, offset+=fontSize+10)    
            cardImage.DrawText(".[]}.", 10, 50+offset, fontSize, ORANGE);
        for(std::uint8_t i=0;i<type.tags.Shopping;++i, offset+=fontSize+10)    
            cardImage.DrawText("*\\./", 10, 50+offset, fontSize, GREEN);
        for(std::uint8_t i=0;i<type.tags.Recreation;++i, offset+=fontSize+10)    
            cardImage.DrawText("_|_",   10, 50+offset, fontSize, BLUE);
    }

    // Requirements
    if(type.requirements.size()>0)
    {
        const int fontSize = 15;
        std::string label{"Requirements: \n"};
        for(Requirement req : type.requirements)
            label.append("\t"+req.name+"\n");
        cardImage.DrawText(label, 10, CARD_SIZE.y/2, fontSize, BLACK);        
    }    

    // @TODO: Special Section
    

    // Earn
    {
      const int fontSize = 25;
      cardImage.DrawText(TextFormat("$ %i", type.moneyRevenue.base), 10, height-65, fontSize, BLACK);
      cardImage.DrawText(TextFormat("* %i", type.victoryPoints.base), 10, height-40, fontSize, BLACK);
    }

    cardImages.insert({&type, LoadTextureFromImage(cardImage)});
}

static bool Button(Rectangle rec, Color color, const char* text, int fontSize, Color fontColor, bool active = true)
{
    DrawRectangleRounded(rec, 3, 10, (active)?color:Color{220,220,220,255});
    const raylib::Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
    DrawText(text, rec.x+(rec.width-textSize.x)/2, rec.y+(rec.height-textSize.y)/2, fontSize, fontColor);
    return (!active)?
        false:
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
            CheckCollisionPointRec(GetMousePosition(), rec);
}

inline void DrawCard(Texture cardTex, raylib::Vector2 pos, float rotation=0,float scale=1, Color tint=WHITE)
{
    DrawTextureEx(cardTex,pos,rotation,scale,tint);
}

enum UIAction{
    PASS,
    PROGESS, 
    NOTHING
};

static UIAction DrawGameUI(Player& player)
{
    const Color UI_BG    = Color{94,123,163,255};
    const Color UI_BG_HI = Color{71,126,204,255};
    const int   FONT_SIZE = 20;
    const int yOffset = 10;
    
    auto DrawStatus =[&](const char* text)
    {
        const auto textSize = MeasureTextEx(GetFontDefault(), text, FONT_SIZE, 2);
        DrawRectangle((GetScreenWidth()-textSize.x)/2, 0, textSize.x, textSize.y+2*yOffset, UI_BG_HI);
        DrawText(text,(GetScreenWidth()-textSize.x)/2, yOffset, FONT_SIZE, BLACK);
    };
    
    DrawRectangle(0, 0, GetScreenWidth(), FONT_SIZE+2*yOffset, UI_BG);

    int xOffset = 20;
    {
    auto text = TextFormat(" %d $$ ", player.get_income());
    raylib::Vector2 TextSize = MeasureTextEx(GetFontDefault(), text, FONT_SIZE, 2);
    DrawRectangle(xOffset, 0, TextSize.x, TextSize.y+2*yOffset, UI_BG_HI);
    DrawText(text,xOffset, yOffset, FONT_SIZE, BLACK);
    xOffset += TextSize.x + xOffset;
    }
    {
    auto text = TextFormat(" %d <victory> ", player.get_victoryPoints());
    raylib::Vector2 TextSize = MeasureTextEx(GetFontDefault(), text, FONT_SIZE, 2);
    DrawRectangle(xOffset, 0, TextSize.x, TextSize.y+2*yOffset, UI_BG_HI);
    DrawText( text, xOffset, yOffset, FONT_SIZE, BLACK);
    }
    
    const Rectangle buttonRect1{GetScreenWidth()-400.f, 200.f, 200,100}; 
    const Rectangle buttonRect2{buttonRect1.x, buttonRect1.y+buttonRect1.height+yOffset, buttonRect1.width, buttonRect1.height};
    
    // depended on player state
    switch(player.get_state())
    {
    case PlayerState::DISCARD_2:
    {
        DrawStatus(" Select 2 cards to be discarded ");

        if(Button(buttonRect1, GREEN, " Discard 2 ", 20, WHITE, player.can_progress()))
            return PROGESS;
    }
    break;
    case PlayerState::RESIGN_BONUS_SELECT:
    {
        DrawStatus(" Select 1 card as a bonus ");

        if(Button(buttonRect1, GREEN, " Select ", 20, WHITE, player.can_progress()))
            return PROGESS;
    }
    break;
    case PlayerState::SELECT_CARD:
    {
        DrawStatus(" Select card to be built ");

        if(Button(buttonRect1, GREEN, " Build ", 20, WHITE, player.can_progress()))
            return PROGESS;
        if(Button(buttonRect2, RED, " Pass ", 20, WHITE))
            return PASS;
    }
    break;
    case PlayerState::SELECT_PAYMENT:
    {
        
        const auto toBeBuilt = player.view_toBeBuild();
        DrawStatus(TextFormat("Select %d cards to pay for %s", toBeBuilt->cost, toBeBuilt->name.c_str()));

        if(Button(buttonRect1, GREEN, " Pay ", 20, WHITE, player.can_progress()))
            return PROGESS;
        
        if(Button(buttonRect2, RED, " Cancel ", 20, WHITE))
        {
            player.cancel_select_mode();
        }
        break;
    }
    default:
        break;
    }
    
    return NOTHING;
}


static const CardType* ShowHand(raylib::Vector2 pos, int spread, Player& player)
{
    const raylib::Vector2 mPos = GetMousePosition();
    const int cardOffset = CARD_SIZE.x + spread;
    
    const CardType* zoomedCard = nullptr;
    const bool zoomCard = IsKeyDown(KEY_LEFT_ALT);

    Card* clicked = nullptr;
     
    int cardX = (int)pos.x - ((player.handDeck.size()-1)*cardOffset)/2 - (int)CARD_SIZE.x/2;

    for(Card& card : player.handDeck.get_cards())
    {
        const Rectangle cardRect = {(float)cardX, pos.y-CARD_SIZE.y/2, CARD_SIZE.x, CARD_SIZE.y};
        const Color cardTint = (card.isSelected())?GREEN:WHITE;
        const float rotation = (card.isSelected())?20:0;
        // mouse collition
        
        cardX+=cardOffset;   
        if(CheckCollisionPointRec(mPos, cardRect))
        {
            if(zoomCard)
                zoomedCard = card.type;
            else if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                clicked = &card;
        }
        DrawTextureEx(cardImages.at(card.type), {cardRect.x, cardRect.y}, rotation, 1,  cardTint);
    }

    if(clicked != nullptr)
    {
        player.select_card(*clicked);
    }
    return zoomedCard;    
}

static const CardType* ShowBuilt(raylib::Vector2 pos, int spread, const Player& player)
{
    const raylib::Vector2 mPos = GetMousePosition();
    const CardType* zoomedCard = nullptr;
    const bool zoomCard = IsKeyDown(KEY_LEFT_ALT);

    constexpr float CARD_SCALE = 0.6;
    const int cardOffset = CARD_SIZE.x*CARD_SCALE + spread;

    float cardX = (int)pos.x - ((player.builtArea.size()-1)*cardOffset)/2.f - (CARD_SIZE.x/2*CARD_SCALE);
    for(const Card& card : player.builtArea.lookup())
    {
        const Rectangle cardRect = {cardX, pos.y-CARD_SIZE.y/2*CARD_SCALE, CARD_SIZE.x*CARD_SCALE, CARD_SIZE.y*CARD_SCALE};

        if(zoomCard && CheckCollisionPointRec(mPos, cardRect))
                zoomedCard = card.type;
        DrawTextureEx(cardImages.at(card.type), {cardRect.x,cardRect.y}, 0, CARD_SCALE, WHITE);

        cardX += cardOffset;
    }
    return zoomedCard;
}

static const CardType* ShowEventSelect(raylib::Vector2 pos, int spread, Player& player)
{
    const raylib::Vector2 mPos = GetMousePosition();
    const CardType* zoomedCard = nullptr;
    const bool zoomCard = IsKeyDown(KEY_LEFT_ALT);

    Card* clicked = nullptr;

    constexpr float CARD_SCALE = 0.6;
    const int cardOffset = CARD_SIZE.x*CARD_SCALE + spread;

    // Darken the background
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0,0,0,100});

    float cardX = (int)pos.x - ((player.eventSelectDeck.size()-1)*cardOffset)/2.f - (CARD_SIZE.x/2*CARD_SCALE);
    for(Card& card : player.eventSelectDeck.get_cards())
    {
        const Rectangle cardRect = {cardX, pos.y-CARD_SIZE.y/2*CARD_SCALE, CARD_SIZE.x*CARD_SCALE, CARD_SIZE.y*CARD_SCALE};
        const Color cardTint = (card.isSelected())?GREEN:WHITE;
        const float rotation = (card.isSelected())?20:0;

        if(CheckCollisionPointRec(mPos, cardRect))
        {
            if(zoomCard)
                zoomedCard = card.type;
            else if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                clicked = &card;
        }
        DrawTextureEx(cardImages.at(card.type), {cardRect.x,cardRect.y}, rotation, CARD_SCALE, cardTint);

        cardX += cardOffset;
    }
    
    if(clicked != nullptr)
    {
        player.select_card(*clicked);
    }
    
    return zoomedCard;
}

static void DrawZoom(const CardType* card)
{
    // Darken the screen
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0,0,0,100});
    
    const raylib::Vector2 centerCard = {((float)GetScreenWidth()-CARD_SIZE.x*CARD_ZOOM_SCALE)/2, ((float)GetScreenHeight()-CARD_SIZE.y*CARD_ZOOM_SCALE)/2};
    DrawTextureEx(cardImages.at(card), centerCard, 0,  CARD_ZOOM_SCALE, WHITE);
}

static bool DrawCardPile(raylib::Vector2 pos, int n = 3, int spread=10)
{    
    bool collide = false;
    for(int offset = 0;offset<n;offset++)
    {
        raylib::Vector2 cardPos = {pos.x+offset*spread, pos.y+offset*spread};
        DrawTextureV(cardReversImg, cardPos, WHITE);
        if(!collide && CheckCollisionPointRec(GetMousePosition(), {cardPos.x, cardPos.y, CARD_SIZE.x, CARD_SIZE.y}))
            collide = true;    
    }
    return collide;
}

static void LoadTextures()
{
    for(const auto& cardType : GameEngine::masterSet.cards)
        generateCardTex(cardType);
    Image cardBackPattern = LoadImage("./textures/cardReverse.png");
    Image cardBackImage = ImageFromImage(cardBackPattern, {0,0,CARD_SIZE.x, CARD_SIZE.y});
    ImageDrawRectangleLines(&cardBackImage, Rectangle{0,0,CARD_SIZE.x, CARD_SIZE.y}, 4, WHITE);
    cardReversImg = LoadTextureFromImage(cardBackImage);  
}

#ifdef OFFLINE
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
    LoadTextures();    

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        const raylib::Vector2 screenCenter = {(float)GetScreenWidth()/2,(float)GetScreenHeight()/2};
        
        auto& currentPlayer = gameState.getCurrentPlayer(); 
        
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
            
            DrawGameUI(currentPlayer);

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
#endif
