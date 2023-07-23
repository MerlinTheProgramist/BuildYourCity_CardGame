#pragma once

#include "card.h"

class GameEngine
{
public:
  
  const CardSet masterSet = {
    // cards that will not generate in normal deck
    {0, {"Architect",     0, {0,0,0}, {1}, {0}, {}, true, 0}},
    // normal cards, with no additional effects
    {6, {"House",             1, {0,0,0}, {1}, {0}}},
    {4, {"CarPark",           0, {1,1,1}, {0}, {0}}},
    {3, {"Stadium",           6, {2,0,0}, {2}, {3}}},
    {3, {"Train Station",     4, {1,1,1}, {1}, {2}}},
    {4, {"Tower block",       3, {0,1,1}, {1}, {1}}},
    {3, {"Convention Center", 7, {0,1,0}, {1}, {5}}},
    {2, {"Hospital",          6, {0,0,1}, {1}, {4}}},
    {2, {"Theater",           4, {0,0,1}, {1}, {3}}},
    {2, {"Modern art",        4, {0,0,2}, {0}, {3}}},
    {6, {"Buissness centre",  3, {0,0,2}, {1}, {1}}},
    {4, {"Department store",  3, {0,1,1}, {2}, {0}}},
    {3, {"Airport",           9, {1,1,0}, {1}, {8}}},
    {3, {"Technology park",   5, {1,0,1}, {2}, {2}}},
    {4, {"Cinema",            2, {1,1,1}, {1}, {0}}},
    {3, {"Town hall",         6, {0,0,1}, {0}, {5}}},
    {2, {"Monument",          9, {0,0,3}, {1}, {8}}},
    {2, {"Discount retailer", 6, {1,2,0}, {3}, {1}}},
    {2, {"Multiplex",         4, {1,2,0}, {2}, {1}}},
    {3, {"Sport park",        4, {2,0,0}, {0}, {3}}},
    {5, {"Auditorium",        7, {0,0,1}, {0}, {7}}},
    {2, {"Skyscaper",         8, {0,0,1}, {1}, {7}}},
    {2, {"Opera",             8, {0,0,1}, {0}, {8}}},
    {2, {"Theme park",        8, {3,0,0}, {2}, {5}}},
    {4, {"Market centre",     3, {1,0,0}, {1}, {1}}},
    {4, {"Townhouse",         1, {0,0,1}, {0}, {1}}},
    {2, {"Casino",            6, {0,1,0}, {4}, {1}}},
    {3, {"Train station",     4, {1,1,1}, {1}, {2}}},
    // Special
    {4, {"City park",         1, {0,0,1}, {0}, Gain{0, PerTag{{0,0,1}}}, {}, true}},
    {4, {"Shopping arcade",   1, {0,0,1}, {0}, {0, PerTag{{0,0,1}}}, {}, true}},
    {2, {"Shopping centre",   7, {0,3,0}, {2}, {0, PerTag{{0,1,0}}}}},
    {2, {"University",        5, {0,0,1}, {0}, {4, PerBuild{1,"School"}}}},
    {6, {"Villa",             4, {0,0,0}, {0}, {3, PerBuild{1,"Villa"}}}},
    
    {4, {"Bridge",            4, {1,0,0}, {1, PerBuild{1,"Highway"}}, {2}}},
  
    {3, {"Supermarket",       1, {1,1,0}, {1}, {0}, {{"House"},{"Estate"}, {"Traffic interchange"}}}},
    {2, {"Park and eat",      11,{2,0,0}, {0}, {0, PerTag{{1,0,0}},PerEnemyTag{{1,0,0}}}}},
    {4, {"Office",            2, {0,0,0}, {1, WithBuild{1, "Tower block"}}, {2}}},
    {4,{"Traffic intersection",2,{0,0,0}, {0, PerTag{{1,0,0}}}, {0,PerTag{{1,0,0}}}, {}, true}},
    {3, {"Shopping mall",     8, {1,2,0}, {3}, {0,PerTag{{0,1,0}}}}},
    {4, {"Municipal Office",  2, {0,0,0}, {0, PerTag{{0,0,1}}}, {1}, {}, true}}, // cost -1 for every building with Recreation 
    {6, {"School",            1, {0,0,0}, {0}, {2}, {{"House"}, {"Estate"}, {"Villa"}}}}, // cost -2 for villa   
    {4, {"Industrial estate", 1, {0,0,0}, {1, WithBuild{1, "Research centre"}}, {0}}}, // cost -1 for research centre and industrial site
    {3, {"Boutique",          2, {0,1,1}, {1, WithBuild{1, "Buissness centre"}}, {0}}},
    {4, {"Resteurant",        1, {0,1,1}, {0, WithBuild{1, "Buissness centre"}}, {1}}},  
    {2, {"Industrial site",   2, {1,0,0}, {1}, {0}}}, // cost -1 for Highway and Road intersection
    {3, {"Highway",           3, {1,0,0}, {0}, {0, PerBuild{2, "Road intersection"}}}},
    {3, {"Museum",            5, {0,0,1}, {0, PerBuild{1,"School"}}, {4}}},
    {1, {"Tube",              11,{0,0,0}, {0}, {0,PerTag{{0,0,1}},PerEnemyTag{{0,0,1}}}}},
    {1, {"Coach station",     1, {0,1,1}, {0, WithBuild{1, "Supermarket"}}, {1}}},
    {1, {"Research centre",   4, {0,0,0}, {1}, {2, PerGameBuild{2, "University"}}}},
};
const CardType* architectCardType = &masterSet.begin()->second;
private:
  CardPool masterPool;
  
  std::vector<Player> players;
  std::vector<Player>::iterator currentPlayer;

  int round{0};

  
  void startRound()
  {
    for(auto&& p : players) 
      if(p.get_state()==PlayerState::FINISHED)
        p.progress();
    currentPlayer = players.begin();
    round++;
  }
public:
  
  GameEngine(int player_num , size_t starting_cards = 7)
  {
    masterPool.avaliable = CardDeck(masterSet);
    masterPool.avaliable.shuffle();
    
    for(int i=0;i<player_num;i++)
    {            
      players.push_back(Player(masterPool));
      players.back().draw_from(masterPool, starting_cards);
      players.back().get_hand().add(architectCardType);
    }
    startRound();
  }

  int masterDeckSize(){return masterPool.avaliable.size();}
  int discardedDeckSize(){return masterPool.discarded.size();}
  
  Player& getPlayer(int n);
  Player& getCurrentPlayer()
  {
    if(currentPlayer->get_state() == PlayerState::FINISHED)
      if(++currentPlayer == players.end())  // if this is the last player
        startRound();
    return *currentPlayer;
  }

};
