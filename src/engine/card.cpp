#include "card.h"
#include "player.h"

#include <algorithm>
#include <cassert>

int Gain::eval(ConditionArgument arg) const
{
  int sum = base;
  for(auto&& req : multipliers)
    sum+= req(arg);
  return sum;
}

CardType::CardType(size_t count,
           const std::string& name, 
           int cost,
           Tags tags, 
           Gain moneyRevenue,
           Gain victoryPoints,
           std::initializer_list<Requirement> requirements,
           bool one_per_player,
           int value)
  : count(count)
  , name(name)
  , cost(cost)
  , tags(tags)
  , moneyRevenue(moneyRevenue)
  , victoryPoints(victoryPoints)
  , requirements(requirements)
  //,  special_effects(effects)
  , max_one_per_player(one_per_player)
  , value(value
  ){
    
  }

bool CardType::canBuild(const Player& state) const
{
  // check money(card_count) is e higher than card
  if(state.get_money()-1 < cost) return false;

  // check requirements
  for(auto req : requirements)
  {
    if(req(state.handDeck) == false) return false;
  }

  // check uniqness
  if(max_one_per_player)
  {
    // check if player already owns this
    for(const Card& card : state.builtArea.lookup())
      if(card.type == this)
        return false;
  }
  
  return true;
}


const std::list<Card>& CardDeck::lookup() const
{
  return cards;
}
std::list<Card>& CardDeck::get_cards()
{
  return cards;
}

std::vector<cardIdT> CardDeck::get_card_ids(const CardSet& offset) const
{
  std::vector<cardIdT> res;

  for(const Card& card : lookup())
  {
    res.emplace_back(card.type - offset.cards.data());
  }
  return res;
}

std::random_device rd;
std::mt19937 g(rd());

void CardDeck::shuffle()
{
    std::vector< std::reference_wrapper< const Card > > vec( cards.begin(), cards.end() ) ;

    // shuffle (the references in) the vector
    std::shuffle( vec.begin(), vec.end(), std::mt19937{ std::random_device{}() } ) ;

    // copy the shuffled sequence into a new list
    std::list<Card> shuffled_list {  vec.begin(), vec.end() } ;

    // swap the old list with the shuffled list
    cards.swap(shuffled_list) ;
}

size_t CardDeck::transfer(CardDeck& dest, std::size_t n)
{
  n = std::min(n, cards.size());

  auto range_end = cards.begin();
  std::advance(range_end, n);    // take exacly n cards

  // transfer from this->cards range to dest.cards  
  dest.cards.splice(dest.cards.end(),this->cards, this->cards.begin(), range_end);
  
  return n;
}

Card CardDeck::pop()
{
  auto temp = cards.front();
  cards.pop_front();
  return temp;
}
void CardDeck::add(Card card)
{
  cards.push_back(card);
}
void CardDeck::add(const CardType* cardType)
{
  cards.push_back(Card{cardType});
}
void CardDeck::add(cardIdT id, const CardSet& offset)
{
  // std::cout << "Loaded card id: "<< id << std::endl;
  cards.push_back(Card{&offset.cards[id]});
}

size_t CardDeck::size() const{ return cards.size();}

// *********
// CARD_POOL
// *********
void CardPool::take(CardDeck& dst, std::size_t n)
{
  if(avaliable.size() <= n)
  {
    shuffleDiscard();
  }
  avaliable.transfer(dst, n);
}

void CardPool::shuffleDiscard()
{
  discarded.shuffle();
  discarded.transfer(avaliable, discarded.size());
}

void CardPool::discard_card(const CardType* card)
{
  discarded.add(Card{card});
}


void CardDeck::unselect_all()
{
  for(Card& card : get_cards())
    card.selected = false;
}

int CardDeck::sumMoney(const std::vector<Player>& otherDecks) const
{
  int sum = 0;
  for(Card card : lookup())
  {
    sum += card.type->moneyRevenue.eval({*this, otherDecks});
  }
  return sum;
}
int CardDeck::sumVictory(const std::vector<Player>& otherDecks) const
{
  int sum = 0;
  for(Card card : lookup())
  {
    sum += card.type->victoryPoints.eval({*this, otherDecks});
  }
  return sum;
}

CardDeck CardDeck::get_selected() const
{
  CardDeck res{};
  for(const Card& card : cards)
    if(card.isSelected())
      res.add(card.type);
  return res;
}

void CardDeck::pop_selected(CardDeck& dest)
{
  auto iter = cards.begin();
  while(iter != cards.end())
  {
    if(iter->isSelected())
    {
      dest.add(iter->type);
      iter = cards.erase(iter);
    }else
    iter++;
  }
}

const CardType* CardDeck::pop_single_selected()
{
  CardDeck temp;
  pop_selected(temp);
  assert(temp.size()==1 && "more than 1 card was selected");
  return temp.get_cards().begin()->type;
}

int CardDeck::count_selected() const 
{
  int res{0};
  for(const Card& card : cards)
    if(card.isSelected())
      res++;
  return res;
}
