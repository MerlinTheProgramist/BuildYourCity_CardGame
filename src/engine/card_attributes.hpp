#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct CardDeck;
struct Player;

struct ConditionArgument{
  const CardDeck& that; 
  const std::vector<Player>& other{};
};

template<typename RET>
struct Condition{
  virtual RET operator()(const CardDeck&) const{return 1;}
};

// using Requirement = Condition<bool>;
// using multiplier = Condition<int>;

struct Multiplier
{
  const int amount{};
  Multiplier(int val):amount(val){}
  virtual int operator()(ConditionArgument) const
  {return amount;}
};

// require ONE of the list
struct Requirement;// : Condition<bool>


struct Gain
{
  const int base;
  const std::vector<Multiplier> multipliers;
  Gain(int base):
    base(base),
    multipliers({}){}
  template<typename... T>
  Gain(int base, T... multipliers):
    base(base),
    multipliers{multipliers...}{}

  int eval(ConditionArgument arg) const;
};


typedef struct Tags
{
  uint8_t Transport {0};
  uint8_t Recreation{0};
  uint8_t Shopping  {0};

  int operator* (Tags other) const
  {
    return Transport*other.Transport + Recreation*other.Recreation + Shopping*other.Shopping;
  }

} Tags;

constexpr Tags operator"" _T(unsigned long long nums){
  return Tags{
    static_cast<uint8_t>(nums&0b100),
    static_cast<uint8_t>(nums&0b010),
    static_cast<uint8_t>(nums&0b001),
  };
}
struct Requirement : Condition<bool>
{
  const std::string name;
  Requirement(const std::string& name)
  : name(name)
  {}
  
  bool operator()(const CardDeck& arg) const override;
};

struct PerTag : Multiplier{
  const Tags reqs;
  PerTag(Tags taqs)
  : Multiplier(1)
  , reqs(taqs)
  {}

  int operator()(ConditionArgument arg) const override;
}; 

// Per tag from enemy deck, auto choose the best option
struct PerEnemyTag : Multiplier{
  const Tags reqs;
  PerEnemyTag(Tags taqs)
  : Multiplier(0)
  , reqs(taqs)
  {}
  
  int operator()(ConditionArgument arg) const override;
}; 

// Per building build by self
struct PerBuild : Multiplier{
  const std::string name;
  PerBuild(int amount, const std::string& name)
  : Multiplier(amount),name(name)
  {}

  int operator()(ConditionArgument arg) const override;
};

// Per building in the entire game
struct PerGameBuild : Multiplier{
  const std::string name;
  PerGameBuild(int amount, const std::string& name)
  : Multiplier(amount),name(name)
  {}

  int operator()(ConditionArgument arg) const override;
};

struct WithBuild : Multiplier{
  const std::string name;
  WithBuild(int amount, const std::string& name)
  : Multiplier(amount),name(name)
  {}
  
  int operator()(ConditionArgument arg) const override;
};

