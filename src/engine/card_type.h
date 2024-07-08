#pragma once

#include "card_attributes.hpp"
#include <cstddef>
#include <string>

struct Player;

struct CardType
{
  const size_t count;
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
  CardType(size_t count,
           const std::string& name, 
           int cost,
           Tags tags, 
           Gain moneyRevenue,
           Gain victoryPoints,
           // std::list<CardEffect> effects = {},
           std::initializer_list<Requirement> requirements = {},
           bool one_per_player = false,
           int value = 1);

  void add_requirement(const size_t card_id);
  bool canBuild(const Player& state) const;
};

