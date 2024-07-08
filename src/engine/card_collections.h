#pragma once

#include "card.h"
#include "card_type.h"
#include <list>

using cardIdT = size_t;

class CardCollection
{
  
};

class cardHand : CardCollection
{

};


class CardDeck : CardCollection
{
public:
  std::list<Card> cards;
  CardDeck(const CardSet& set)
  {
    for(auto&& type : set.cards)
    {
       for(int i = type.count; i>=0;--i)
        cards.emplace_back(&type); 
    }
  }
  CardDeck():cards({}){}

  
  const std::list<Card>& lookup() const;
  std::list<Card>& get_cards();
  std::vector<cardIdT> get_card_ids(const CardSet& offset) const;

  void shuffle();
  size_t transfer(CardDeck& dst, std::size_t n);
  void add(Card card);
  void add(const CardType* cardType);
  void add(cardIdT id, const CardSet& offset);
  [[nodiscard]] Card pop();
  size_t size() const;
  int evauate() const;

  void unselect_all();
  
  int count_selected() const;
  std::vector<const CardType*> view_selected() const;
  void pop_selected(CardDeck& dest);
  CardDeck get_selected() const;

  const CardType* pop_single_selected();

    
  int sumMoney(const std::vector<Player>& otherDecks) const;
  int sumVictory(const std::vector<Player>& otherDecks) const;
};

struct CardPool
{
  CardDeck avaliable{};
  CardDeck discarded{};

  
  void shuffleDiscard();

  void discard_card(const CardType* type);
  void take(CardDeck& dst, std::size_t n);
};
