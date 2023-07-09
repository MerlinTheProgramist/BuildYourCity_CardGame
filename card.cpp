#include "card.h"

#include <algorithm>
#include <iostream>
#include <cassert>


bool CardType::canBuild(const Player& state) const
{
  // check money(card_count)
  if(state.get_money() < cost) return false;

  // check requirements
  for(auto req : requirements)
  {
    if(req(state.hand_view()) == false) return false;
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

void CardPool::push_discarded(const CardType* card)
{
  discarded.add(Card{card});
}

void Player::draw_from(CardPool& src, std::size_t n)
{
  src.take(handDeck, n);
}

const CardDeck& Player::built_view() const{ return builtArea;}
const CardDeck& Player::hand_view() const{ return handDeck;}
const CardType* Player::view_toBeBuild() const{assert(toBeBuild!=nullptr); return toBeBuild;}
int Player::get_income() const {return currentIncome;}
int Player::get_victoryPoints() const {return victoryPoints;}
int Player::get_money() const {return handDeck.size();}
PlayerState Player::get_state() const {return state;}
bool Player::can_progress() const {return canProgress;}
CardDeck& Player::get_hand(){ return handDeck;}

void Player::cancel_select_mode()
{
  unselect_all(); 
  state = PlayerState::SELECT_CARD;
  handDeck.add(Card{toBeBuild});
}

void Player::progress()
{
  if(!eval_can_progress()) return;
  
  switch(state)
  {
    case PlayerState::DISCARD_2:
    {
        for(const CardType* card : handDeck.pop_selected())
          masterPool.push_discarded(card);
        // Progress to next state
        state = PlayerState::SELECT_CARD; 
    }
    break;
    case PlayerState::SELECT_CARD:
    {
        toBeBuild = handDeck.pop_selected()[0];
        state = PlayerState::SELECT_PAYMENT;
    }
    break;
    case PlayerState::SELECT_PAYMENT:
    {
        for(const CardType* card : handDeck.pop_selected())
          masterPool.push_discarded(card);
        state = PlayerState::UPDATE_STATS;
    }
    break;
    default:
    break;
  }
  eval_can_progress();
}

bool Player::eval_can_progress()
{
  switch(state)
  {
    case PlayerState::DISCARD_2:
    {
      return canProgress = (handDeck.count_selected() == 2);      
    }
    break;
    case PlayerState::SELECT_CARD:
    {
        return canProgress = (toBeBuild!=nullptr && toBeBuild->canBuild(*this));
    }
    break;
    case PlayerState::SELECT_PAYMENT:
    {
      return canProgress = (handDeck.count_selected() == toBeBuild->cost);  
    }
    break;
    default:
      return canProgress = false;
  }
}

void Player::select_card(Card& card)
{
  if(card.selected)
  {
    if(state == PlayerState::SELECT_CARD) toBeBuild=nullptr;
    card.selected = false;
    eval_can_progress();
    return;
  }
  
  switch(state)
  {
    case PlayerState::SELECT_CARD:
      // firstly unselect all onther cards, because only one can be build
      unselect_all();
      toBeBuild = card.type;
      card.selected = true;
    break;
    default:
      card.selected = true;    
  }

  eval_can_progress();
}

void Player::unselect_all()
{
  for(Card& card : handDeck.get_cards())
    card.selected = false;
}

void Player::print_hand() const
{
  for(const Card& instance : handDeck.lookup())
  {
    const CardType& card = *instance.type;
    std::cout << card.cost<< " " << card.name << " " << card.cost << std::endl;
    std::cout << ((card.tags.Transport!=0)?
                 std::to_string(card.tags.Transport)+ "x .[]}.":"") << std::endl
              << ((card.tags.Shopping!=0)?
                 std::to_string(card.tags.Shopping)+  "x *\\./":"") << std::endl
              << ((card.tags.Recreation!=0)?
                 std::to_string(card.tags.Recreation)+"x  _|_ ":"") << std::endl;
    // requirements
    // 

    // money gain
    std::cout << "Money gain:" << std::endl;
    for(auto rev : card.moneyRevenue)
    {
      std::cout << rev.amount << " "; 
    }
    std:: cout << std::endl;

    // victory points
    std::cout << "Victory points: :" << std::endl;
    for(auto vic : card.victoryPoints)
    {
      std::cout << vic.amount << " ";
    }
    std::cout << std::endl << std::endl;
  
    
  }
}

int CardDeck::sumGain() const
{
  int sum = 0;
  for(Card card : lookup())
  {
    for(const Gain& gain : card.type->moneyRevenue)
      sum += gain.gain(*this);
  }
  return sum;
}
int CardDeck::sumVictory() const
{
  int sum = 0;
  for(Card card : lookup())
  {
    for(const Gain& gain : card.type->victoryPoints)
      sum += gain.gain(*this);
  }
  return sum;
}
int Gain::gain(const CardDeck& state) const
{
  return this->multimlier(state);
}

[[nodiscard]] std::vector<const CardType*> CardDeck::pop_selected()
{
  std::vector<const CardType*> popp;
  auto iter = cards.begin();
  while(iter != cards.end())
  {
    if(iter->isSelected())
    {
      popp.push_back(iter->type);
      iter = cards.erase(iter);
    }else
    iter++;
  }
  return popp;
}

int CardDeck::count_selected() const 
{
  int res{0};
  for(const Card& card : cards)
    if(card.isSelected())
      res++;
  return res;
}