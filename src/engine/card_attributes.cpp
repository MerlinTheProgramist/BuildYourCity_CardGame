#include "card_attributes.hpp"
#include "card.h"
#include "card_collections.h"
#include "player.h"

bool Requirement::operator()(const CardDeck& arg) const
{
    for(const Card& card : arg.lookup())
      if(card.type->name == name)
      return true;
  return false;
}


int PerTag::operator()(ConditionArgument arg) const
{
    int res = 0;
    for(const Card& card : arg.that.lookup())
      res += card.type->tags * reqs;
    return res;  
}

int PerEnemyTag::operator()(ConditionArgument arg) const
{
    int maxRev{0};
    for(auto& enemy : arg.other)
    {
      int enemyRev{0};
      for(auto card : enemy.builtArea.cards)
        enemyRev += card.type->tags * reqs;
      maxRev = std::max(maxRev, enemyRev);
    }
    return maxRev;
}

int PerBuild::operator()(ConditionArgument arg) const
{
    int res = 0;
    for(const Card& card : arg.that.lookup())
      if(card.type->name == name) res += amount;
    return res - amount; // munus one for itself 
}

int PerGameBuild::operator()(ConditionArgument arg) const
{
  int totalRev{0};
  for(auto& enemy : arg.other)
  {
    for(auto card : enemy.builtArea.cards)
      totalRev += (card.type->name == name) * amount;
  }
  return totalRev;
}

int WithBuild::operator()(ConditionArgument arg) const
{
  for(const Card& card : arg.that.lookup())
    if(card.type->name == name) return amount;
  return 0;
}
