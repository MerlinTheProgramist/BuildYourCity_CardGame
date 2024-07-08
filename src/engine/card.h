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
// #include "player.h"
#include "card_attributes.hpp"
#include "card_type.h"

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


struct CardSet{
  std::vector<CardType> cards;

  CardSet(std::initializer_list<CardType> cards)
  :cards(cards)
  {
    for(auto cardQ : cards)
    {
      
    } 
  }
};





