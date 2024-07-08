#pragma once

#include "card.h"
#include "card_collections.h"
#include "card_type.h"
#include "player.h"
#include <asio/thread_pool.hpp>

class GameEngine
{
public:
static const CardSet masterSet;  

const CardType& architectCardType = *masterSet.cards.begin();

  CardPool masterPool;
  std::vector<Player> players;
  int round{0};
private:
  
  void startRound()
  {
    for(auto&& p : players) 
      p.progress(masterPool, players, CardDeck{});
    round++;
  }
public:
  
  GameEngine(int player_num)
  {
    masterPool.avaliable = CardDeck(masterSet);
    masterPool.avaliable.shuffle();
    
    for(int i=0;i<player_num;i++)
    {            
      players.push_back(Player());
      // @RULES each player starts with 7 cards
      players.back().draw_from(masterPool, 7);
      players.back().handDeck.add(&architectCardType);
    }
  }

  int masterDeckSize(){return masterPool.avaliable.size();}
  int discardedDeckSize(){return masterPool.discarded.size();}
  
  Player& getPlayer(int n){return players[n];}

  bool progressIfAllDone()
  {
    for(Player& p : players)
    {
      if(p.get_state() != PlayerState::FINISHED)
        return false;
    }

    startRound();
    
    return true;    
  }
  // Player& getCurrentPlayer()
  // {
  //   if(currentPlayer->get_state() == PlayerState::FINISHED)
  //     if(++currentPlayer == players.end())  // if this is the last player
  //       startRound();
  //   return *currentPlayer;
  // }

};


