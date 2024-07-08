#include "player.h"

void Player::draw_from(CardPool& src, std::size_t n)
{
  src.take(handDeck, n);
}

// const CardDeck& Player::built_view()     const{return builtArea;}
// const CardDeck& Player::hand_view()      const{return handDeck;}
const CardType* Player::view_toBeBuild() const{
  // assert(toBeBuild!=nullptr); 
  return toBeBuild;
}

size_t Player::get_income()                 const{return currentIncome;}
size_t Player::get_victoryPoints()          const{return victoryPoints;}
size_t Player::get_money()                  const{return handDeck.size();}
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

void Player::progress(CardPool& masterPool, const std::vector<Player>& otherDecks, const CardDeck& selected)
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
    }
    break;
    case PlayerState::SELECT_CARD:
    {
        // move build to temporary (will not show in hand) 
        toBeBuild = handDeck.pop_single_selected();
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
    break; 
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
  }
  eval_can_progress();
}

void Player::progress_light()
{
  switch(state)
  {
    case PlayerState::DISCARD_2:
    {
      CardDeck temp{};
      handDeck.pop_selected(temp);
      state = PlayerState::SELECT_CARD;
    }break;
    case PlayerState::RESIGN_BONUS_SELECT:
    {
        // add selected to hand
        eventSelectDeck.pop_selected(handDeck);
        // discard all other
        state = PlayerState::UPDATE_STATS; 
    }break;
    case PlayerState::SELECT_CARD:
    {
        // move build to temporary (will not show in hand) 
        toBeBuild = handDeck.pop_single_selected();
        // Progress to next state
        state = PlayerState::SELECT_PAYMENT;
    }
    break;
    case PlayerState::SELECT_PAYMENT:
    {
        CardDeck temp{};
        //@RULES remove payment
        handDeck.pop_selected(temp);
        
        //@RULES build selected
        builtArea.add(toBeBuild);
        
        // Progress to next state
        state = PlayerState::UPDATE_STATS;
    }
    break; 
    case PlayerState::UPDATE_STATS:
    {
        state = PlayerState::FINISHED;    
    }
    case PlayerState::FINISHED:
    {
        state = PlayerState::SELECT_CARD;
    }
    break;
  }
}

// /*
bool Player::pass(CardPool& masterPool)
{
  // can only pass in build select stage
  if(state!=PlayerState::SELECT_CARD){
    return false;
  }

  //@RULES give player 5 to choose 1 from
  masterPool.take(eventSelectDeck, 5);
  if(eventSelectDeck.size()==0)
    // if cant draw more cards
    state = PlayerState::UPDATE_STATS;
  else
    state = PlayerState::RESIGN_BONUS_SELECT;
  toBeBuild = nullptr;

  return true;
}
// */

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

