#pragma once

#include <cmath>
#include <cstddef>
#include <functional>
#include <list>
#include <random>
#include <raylib.h>
#include <sys/types.h>
#include <type_traits>
#include <vector>
#include <span>

class CardDeck;
class Player;

template<typename RET>
struct Condition{
  virtual RET operator()(const CardDeck&) const{return 1;}
};
using Requirement = Condition<bool>;
using Multiplier = Condition<int>;

struct DefaultMultiplier : Multiplier
{
  int operator()(const CardDeck&) const{return 1;}
};


class Gain
{
  const Multiplier multimlier;
public:  
  const int amount;
  Gain(int amount):amount(amount),multimlier(DefaultMultiplier{}){}
  Gain(int amount, Multiplier multiplier):amount(amount),multimlier(multiplier){}

  int gain(const CardDeck& state) const;
};

typedef struct Tags
{
  unsigned Transport {0};
  unsigned Recreation{0};
  unsigned Shopping  {0};

  int operator* (Tags other) const
  {
    return Transport*other.Transport + Recreation*other.Recreation + Shopping*other.Shopping;
  }
} Tags;

class CardType
{
public:
  // standard parameters 
  const char* name;
  const int cost;  
  const Tags tags; 
  const std::vector<Gain> moneyRevenue; 
  const std::vector<Gain> victoryPoints; 
  
  // optional traits
  const std::vector<Requirement> requirements;
  // const std::list<CardEffect> special_effects;
  const bool max_one_per_player;

public: 
  CardType(const char* name, 
           int cost,
           Tags tags, 
           std::vector<Gain> moneyRevenue,
           std::vector<Gain> victoryPoints,
           // std::list<CardEffect> effects = {},
           std::vector<Requirement> requirements = {},
           bool one_per_player = false)
  : name(name),
    cost(cost),
    tags(tags),
    moneyRevenue(moneyRevenue),
    victoryPoints(victoryPoints),
    requirements(requirements),
    // special_effects(effects),
    max_one_per_player(one_per_player)
  {
    
  }

  void add_requirement(const size_t card_id);
  bool canBuild(const Player& state) const;
};




class Card
{
private:
friend Player;
  bool selected;
  bool canBuild;
public:
  const CardType* type;
  Card(const CardType* type):type(type),selected(false),canBuild(false)
  {
    
  }
  bool isSelected() const{return selected;} 
  
  bool operator==(Card& other){
    return &type == &other.type;
  }
};


using CardSet = std::vector<std::pair<std::size_t, CardType>>;

class CardDeck
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
  void shuffle();
  size_t transfer(CardDeck& dst, std::size_t n);
  void add(Card card);
  [[nodiscard]] Card pop();
  size_t size() const;
  
  int count_selected() const;
  std::vector<const CardType*> view_selected() const;
  std::vector<const CardType*> pop_selected();

    
  int sumGain() const;
  int sumVictory() const;
};

struct CardPool
{
  CardDeck avaliable{};
  CardDeck discarded{};

  
  void shuffleDiscard();

  void push_discarded(const CardType* type);
  void take(CardDeck& dst, std::size_t n);
};

enum class PlayerState{
  DISCARD_2,
  SELECT_CARD,
  SELECT_PAYMENT,
  UPDATE_STATS,
  FINISHED
}; 

class Player
{
  private:
    CardPool& masterPool;
    
    CardDeck handDeck;
    CardDeck builtArea;
    
    int victoryPoints{0};
    int currentIncome{0};
  
    int selectedCount = 0; 
    const CardType* toBeBuild = nullptr; 
    
    mutable PlayerState state = PlayerState::DISCARD_2;
    bool canProgress = false;
    bool eval_can_progress();
  public:
    Player(CardPool& masterPool, CardDeck& deck):masterPool(masterPool),handDeck(deck),builtArea(){}
    Player(CardPool& masterPool):masterPool(masterPool),handDeck(),builtArea(){}
    
    void play(Card& card);
    void draw_from(CardPool& src, std::size_t n);

    // non mutable return
    const CardDeck& built_view() const; 
    const CardDeck& hand_view() const;
    const CardType* view_toBeBuild() const;
    int get_income() const;
    int get_victoryPoints() const;
    int get_money() const;
    PlayerState get_state() const;
    bool can_progress() const;
    
    // mutable return 
    CardDeck& get_hand();

    void progress();
    void select_card(Card& card);
    void unselect_all();
    
    void cancel_select_mode();
    
    void print_hand() const;
};



// template<int x, int y, int z>
// struct TagMultiplier : Multiplier{
//   virtual int operator()(const CardDeck& state) const
//   {
//     int res = 0;
//     for(const Card& card : state.lookup())
//       res += card.type->tags * Tags{x,y,z};
//     return 1;  
//   }
// }; 
