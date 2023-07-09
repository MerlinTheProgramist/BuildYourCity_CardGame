#pragma once

#include "card.h"

class GameEngine
{
public:
  const CardType architectCardType = 
        CardType{"Architect",     0, {0,0,0}, {{1}}, {{0}}};
  
  const CardSet masterSet = {
    // normal cards, with no additional effects
    {4, CardType{"CarPark",       0, {1,1,1}, {{0}}, {{0}}}},
    {3, CardType{"Stadium",       6, {2,0,0}, {{2}}, {{3}}}},
    {3, CardType{"Train Station", 4, {1,1,1}, {{1}}, {{2}}}},
  };
private:
  CardPool masterPool;
  CardDeck architectDeck;
  
  std::vector<Player> players;
  std::vector<Player>::iterator currentPlayer;
public:
  
  GameEngine(int player_num , size_t starting_cards = 7)
  {
    masterPool.avaliable = CardDeck(masterSet);
    masterPool.avaliable.shuffle();
    
    for(int i=0;i<player_num;i++)
    {
      architectDeck.add(Card(&architectCardType));
            
      players.push_back(Player(masterPool));
      players.back().draw_from(masterPool, starting_cards);
    }
    currentPlayer = players.begin();
    
  }

  Player& getPlayer(int n);
  Player& getCurrentPlayer()
  {
    return *currentPlayer;
  }
};
