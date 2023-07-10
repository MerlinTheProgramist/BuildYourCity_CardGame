
#pragma once
#include <cmath>
#include <cstddef>
#include <algorithm>
#include <functional>
#include <list>
#include <random>
#include <raylib.h>
#include <sys/types.h>
#include <type_traits>
#include <vector>
#include <span>

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
  const std::vector<Condition<int>> multipliers;
  const int base;
  Gain(int base):
    base(base),
    multipliers({}){}
  Gain(int base, std::initializer_list<Condition<int>> multipliers):
    base(base),
    multipliers(multipliers){}

  int eval(const CardDeck& state) const;
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
  const Gain moneyRevenue; 
  const Gain victoryPoints; 
  
  // optional traits
  const std::vector<Requirement> requirements;
  // const std::list<CardEffect> special_effects;
  const bool max_one_per_player;

public: 
  CardType(const char* name, 
           int cost,
           Tags tags, 
           Gain moneyRevenue,
           Gain victoryPoints,
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
friend CardDeck;
friend Player;
  bool selected = false;
public:
  const CardType* type;
  Card(const CardType* type):type(type),selected(false)
  {
    
  }
  
  Card(const Card& card) // Copy constructor, copy only type_ptr
  :type(card.type){}
  
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
  void add(const CardType* cardType);
  [[nodiscard]] Card pop();
  size_t size() const;

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
  private:
    CardPool& masterPool;
    
    CardDeck handDeck;
    CardDeck builtArea;
    CardDeck eventSelectDeck;
    
    int victoryPoints{0};
    int currentIncome{0};
  
    int selectedCount = 0; 
    const CardType* toBeBuild = nullptr; 
    
    mutable PlayerState state = PlayerState::DISCARD_2;
    bool canProgress = false;
    bool eval_can_progress();
  public:
    Player(CardPool& masterPool):masterPool(masterPool),handDeck(),builtArea(),eventSelectDeck(){}
    Player(CardPool& masterPool, CardDeck& deck):Player(masterPool){handDeck = deck;}
    
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
    CardDeck& get_event_select();

    void progress();
    void select_card(Card& card);
    void pass();
    
    void cancel_select_mode();
    
    // void print_hand() const;
};



struct Requirement : Condition<bool>
{
  const std::vector<const CardType*> reqs;
  Requirement(std::vector<const CardType*> req):reqs(req){}
  bool operator()(const CardDeck& deck) const
  {
    for(const CardType* req : reqs)
      for(const Card& card : deck.lookup())
        if(card.type == req)
        return true;
    return false;
  }
};

struct TagMultiplier : Condition<int>{
  const Tags reqs;
  TagMultiplier(Tags tags):reqs(tags){}
  int operator()(const CardDeck& state) const
  {
    int res = 0;
    for(const Card& card : state.lookup())
      res += card.type->tags * reqs;
    return res;  
  }
}; 
