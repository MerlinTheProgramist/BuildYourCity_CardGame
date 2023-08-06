#include "card.h"

#include <algorithm>
#include <iostream>
#include <cassert>

int Gain::eval(ConditionArgument arg) const
{
  int sum = base;
  for(auto&& req : multipliers)
    sum+= req(arg);
  return sum;
}

bool CardType::canBuild(const Player& state) const
{
  // check money(card_count) is one higher than card
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
    res.emplace_back((std::pair<std::size_t, CardType>*)(card.type - sizeof(size_t)) - &offset.cards[0]);
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
  cards.push_back(Card{&(offset.cards.begin() + id)->second});
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

void Player::draw_from(CardPool& src, std::size_t n)
{
  src.take(handDeck, n);
}

// const CardDeck& Player::built_view()     const{return builtArea;}
// const CardDeck& Player::hand_view()      const{return handDeck;}
const CardType* Player::view_toBeBuild() const{assert(toBeBuild!=nullptr); return toBeBuild;}

int Player::get_income()                 const{return currentIncome;}
int Player::get_victoryPoints()          const{return victoryPoints;}
int Player::get_money()                  const{return handDeck.size();}
PlayerState Player::get_state()          const{return state;}
bool Player::can_progress()              const{return canProgress;}

// CardDeck& Player::get_hand()             {return handDeck;}
// CardDeck& Player::get_event_select()     {return eventSelectDeck;}

void Player::cancel_select_mode()
{
  handDeck.unselect_all(); 
  state = PlayerState::SELECT_CARD;
  handDeck.add(Card{toBeBuild});
}

void Player::progress(CardPool& masterPool, const std::vector<Player>& otherDecks)
{
  // if(!eval_can_progress()) return;
  
  switch(state)
  {
    case PlayerState::DISCARD_2:
    {
        // move 2 selcted to discarded
        handDeck.pop_selected(masterPool.discarded);

        
        // Progress to next state
        state = PlayerState::SELECT_CARD; 
    }
    break;
    case PlayerState::RESIGN_BONUS_SELECT:
    {
        // add selected to hand
        eventSelectDeck.pop_selected(handDeck);
        // discard all other
        eventSelectDeck.transfer(masterPool.discarded, eventSelectDeck.size());
          
        state = PlayerState::UPDATE_STATS; 
        progress(masterPool, otherDecks); // @TEMPORARY__TEMPORARY
    }
    break;
    case PlayerState::SELECT_CARD:
    {
        // move build to temporary (will not show in hand) 
        handDeck.pop_selected(toBeBuild);
        // Progress to next state
        state = PlayerState::SELECT_PAYMENT;
    }
    break;
    case PlayerState::SELECT_PAYMENT:
    {
        //@RULES remove payment
        handDeck.pop_selected(masterPool.discarded);
        
        //@RULES build selected
        builtArea.add(toBeBuild);
        
        // Progress to next state
        state = PlayerState::UPDATE_STATS;
    }
    // break; // @TEMPORARY__TEMPORARY
    case PlayerState::UPDATE_STATS:
    {        
        //@RULES update victoryPoints
        victoryPoints = builtArea.sumVictory(otherDecks);
        //@RULES add cards revenue to hand
        masterPool.take(        
          handDeck,
          currentIncome = builtArea.sumMoney(otherDecks)
        );
        state = PlayerState::FINISHED;
    }
    case PlayerState::FINISHED:
    {
        state = PlayerState::SELECT_CARD;
    }
    break;
    default:
    break;
  }
  eval_can_progress();
}

/*
void Player::pass()
{
  assert(state == PlayerState::SELECT_CARD && "can only pass in build select stage");

  //@RULES give player 5 to choose 1 from
  masterPool.take(eventSelectDeck, 5);
  if(eventSelectDeck.size()==0)
    // if cant draw more cards
    state = PlayerState::UPDATE_STATS;
  else
    state = PlayerState::RESIGN_BONUS_SELECT;
  toBeBuild = nullptr;
}
*/

bool Player::eval_can_progress()
{
  switch(state)
  {
    case PlayerState::DISCARD_2:
    {
      return canProgress = (handDeck.count_selected() == 2);      
    }
    break;
    case PlayerState::RESIGN_BONUS_SELECT:
    {
      //@RULES player must to select one card
      return canProgress = (eventSelectDeck.count_selected() == 1);      
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


void Player::select_card(cardIdT id)
{ 
  auto card = handDeck.get_cards().begin();
  std::advance(card, id);
  select_card(*card);
}

void Player::select_card(Card& card)
{
  if(card.isSelected())
  {
    if(state == PlayerState::SELECT_CARD) toBeBuild=nullptr;
    card.selected = false;
    eval_can_progress();
    return;
  }
  
  // unselect all, if only one can be selected
  if(state == PlayerState::RESIGN_BONUS_SELECT){
      eventSelectDeck.unselect_all();
  } 
  else if(state==PlayerState::SELECT_CARD){
      handDeck.unselect_all();
    toBeBuild = card.type;
  }
  
  card.selected = true;   
  eval_can_progress();
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

void CardDeck::pop_selected(const CardType*& dest)
{
  CardDeck temp;
  pop_selected(temp);
  assert(temp.size()==1 && "more than 1 card was selected");
  dest = temp.get_cards().begin()->type;
}



int CardDeck::count_selected() const 
{
  int res{0};
  for(const Card& card : cards)
    if(card.isSelected())
      res++;
  return res;
}