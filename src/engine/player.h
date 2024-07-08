#pragma once
// #include "card.h"
// #include "card.h"
#include "card_collections.h"

struct Card;

enum class PlayerState : uint32_t{
  DISCARD_2,
  SELECT_CARD,
  RESIGN_BONUS_SELECT,
  SELECT_PAYMENT,
  UPDATE_STATS,
  FINISHED
};
const std::string PlayerState_names[]
{  
  
  "DISCARD_2",
  "SELECT_CARD",
  "RESIGN_BONUS_SELECT",
  "SELECT_PAYMENT",
  "UPDATE_STATS",
  "FINISHED"
};

class Player
{
  public:
    CardDeck handDeck;
    CardDeck builtArea;
    CardDeck eventSelectDeck;
  private:
    // CardPool& masterPool;
      
    size_t victoryPoints{0};
    size_t currentIncome{0};
  
    // size_t selectedCount{0}; 
    const CardType* toBeBuild{}; 
    
    mutable PlayerState state{PlayerState::DISCARD_2};
    bool canProgress{false};
    bool eval_can_progress();
  public:
    Player(/*CardPool& masterPool*/):
      // masterPool(masterPool),
      handDeck(),
      builtArea(),
      eventSelectDeck(){}
    Player(/*CardPool& masterPool, */CardDeck& deck):/*Player(masterPool)*/handDeck(deck){}
    
    void draw_from(CardPool& src, std::size_t n);

    // non mutable return
    // const CardDeck& built_view() const; 
    // const CardDeck& hand_view() const;
    const CardType* view_toBeBuild() const;
    size_t get_income() const;
    size_t get_victoryPoints() const;
    size_t get_money() const;
    PlayerState get_state() const;
    bool can_progress() const;
    
    // mutable return 
    // CardDeck& get_hand();
    // CardDeck& get_event_select();

    // action
    void progress(CardPool& masterPool, const std::vector<Player>& otherDecks, const CardDeck& selected);
    void progress_light();
    void select_card(Card& card);
    void select_card(cardIdT id);
    bool pass(CardPool& masterPool);
    
    void cancel_select_mode();
  
    // void print_hand() const;
};

