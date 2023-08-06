
#pragma once
#include <cmath>
#include <cstddef>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <list>
#include <random>
#include <raylib.h>
#include <type_traits>
#include <vector>

struct CardDeck;
class Player;
class CardType;

template<typename RET>
struct Condition{
  virtual RET operator()(const CardDeck&) const{return 1;}
};


// using Requirement = Condition<bool>;
// using multiplier = Condition<int>;

struct Multiplier : Condition<int>
{
  const int val;
  Multiplier(int val):val(val){}
  int operator()(const CardDeck&) const{return val;}
};

// require ONE of the list
struct Requirement;// : Condition<bool>



struct Gain
{
  const int base;
  const std::vector<Condition<int>> multipliers;
  Gain(int base):
    base(base),
    multipliers({}){}
  template<typename... T>
  Gain(int base, T... multipliers):
    base(base),
    multipliers{multipliers...}{}

  int eval(const CardDeck& state) const;
};

typedef struct Tags
{
  std::uint8_t Transport {0};
  std::uint8_t Recreation{0};
  std::uint8_t Shopping  {0};

  int operator* (Tags other) const
  {
    return Transport*other.Transport + Recreation*other.Recreation + Shopping*other.Shopping;
  }
} Tags;

struct CardType
{
  // standard parameters 
  const std::string name;
  const int cost;  
  const Tags tags; 
  const Gain moneyRevenue; 
  const Gain victoryPoints; 
  
  // optional traits
  const std::vector<Requirement> requirements;
  // const std::list<CardEffect> special_effects;
  const bool max_one_per_player;

  const int value;
  CardType(const std::string& name, 
           int cost,
           Tags tags, 
           Gain moneyRevenue,
           Gain victoryPoints,
           // std::list<CardEffect> effects = {},
           std::initializer_list<Requirement> requirements = {},
           bool one_per_player = false,
           int value = 1)
  : name(name),
    cost(cost),
    tags(tags),
    moneyRevenue(moneyRevenue),
    victoryPoints(victoryPoints),
    requirements(requirements),
    // special_effects(effects),
    max_one_per_player(one_per_player),
    value(value)
  {
    
  }

  void add_requirement(const size_t card_id);
  bool canBuild(const Player& state) const;
};




class Card
{
friend CardDeck;
friend Player;
public:
  const CardType* type;
private:
  bool selected = false;
public:
  Card(const CardType* type):type(type),selected(false){}
  
  Card(const Card& card) // Copy constructor, copy only type_ptr
  :type(card.type){}
  
  bool isSelected() const{return selected;} 
  
  bool operator==(Card& other){
    return type == other.type;
  }
  bool operator==(const CardType* other_type){
    return type == other_type;
  }
};


using CardSet = std::vector<std::pair<std::size_t, CardType>>;
using cardIdT = size_t;

class CardCollection
{
  
};

class cardHand : CardCollection
{

};


class CardDeck : CardCollection
{
  std::list<Card> cards;

public:
  CardDeck(const CardSet& set)
  {
    for(auto&& type : set)
    {
       for(int i = type.first; i>=0;--i)
        cards.emplace_back(&type.second); 
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
  void pop_selected(const CardType*& dest);

    
  int sumMoney() const;
  int sumVictory() const;
};

struct CardPool
{
  CardDeck avaliable{};
  CardDeck discarded{};

  
  void shuffleDiscard();

  void discard_card(const CardType* type);
  void take(CardDeck& dst, std::size_t n);
};

enum class PlayerState{
  DISCARD_2,
  SELECT_CARD,
  RESIGN_BONUS_SELECT,
  SELECT_PAYMENT,
  UPDATE_STATS,
  FINISHED
};

class Player
{
  public:
    CardDeck handDeck;
    CardDeck builtArea;
    CardDeck eventSelectDeck;
  private:
    CardPool& masterPool;
      
    int victoryPoints{0};
    int currentIncome{0};
  
    int selectedCount = 0; 
    const CardType* toBeBuild = nullptr; 
    
    mutable PlayerState state = PlayerState::DISCARD_2;
    bool canProgress = false;
    bool eval_can_progress();
  public:
    Player(CardPool& masterPool):
      masterPool(masterPool),
      handDeck(),
      builtArea(),
      eventSelectDeck(){}
    Player(CardPool& masterPool, CardDeck& deck):Player(masterPool){handDeck = deck;}
    
    void draw_from(CardPool& src, std::size_t n);

    // non mutable return
    // const CardDeck& built_view() const; 
    // const CardDeck& hand_view() const;
    const CardType* view_toBeBuild() const;
    int get_income() const;
    int get_victoryPoints() const;
    int get_money() const;
    PlayerState get_state() const;
    bool can_progress() const;
    
    // mutable return 
    // CardDeck& get_hand();
    // CardDeck& get_event_select();

    // action
    void progress();
    void select_card(Card& card);
    void select_card(cardIdT id);
    void pass();
    
    void cancel_select_mode();
  
    // void print_hand() const;
};



struct Requirement : Condition<bool>
{
  const std::string name;
  Requirement(const std::string& name):name(name){}
  bool operator()(const CardDeck& deck) const
  {
      for(const Card& card : deck.lookup())
        if(card.type->name == name)
        return true;
    return false;
  }
};

struct PerTag : Condition<int>{
  const Tags reqs;
  PerTag(Tags taqs):reqs(taqs){}
  int operator()(const CardDeck& state) const
  {
    int res = 0;
    for(const Card& card : state.lookup())
      res += card.type->tags * reqs;
    return res;  
  }
}; 

// @TODO 
struct PerEnemyTag : Condition<int>{
  const Tags reqs;
  PerEnemyTag(Tags taqs):reqs(taqs){}
  int operator()(const CardDeck& state) const
  {
    return 0;
  }
}; 

struct PerBuild : Condition<int>{
  const int amount;
  const std::string name;
  PerBuild(int amount, const std::string& name):amount(amount),name(name){}
  int operator()(const CardDeck& state) const
  {
    int res = 0;
    for(const Card& card : state.lookup())
      if(card.type->name == name) res += amount;
    return res - amount; // munus one for itself 
  }
};

// @TODO
struct PerGameBuild : Condition<int>{
  const int amount;
  const std::string name;
  PerGameBuild(int amount, const std::string& name):amount(amount),name(name){}
  int operator()(const CardDeck& state) const
  {
    return 0;
  }
};

struct WithBuild : Condition<int>{
  const int amount;
  const std::string name;
  WithBuild(int amount, const std::string& name):amount(amount),name(name){}
  int operator()(const CardDeck& state) const
  {
    int res = 0;
    for(const Card& card : state.lookup())
      if(card.type->name == name) return amount;
    return 0;
  }
};


