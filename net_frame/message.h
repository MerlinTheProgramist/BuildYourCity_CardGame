#pragma once

#include <asio/generic/datagram_protocol.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <asio.hpp>
#include <chrono>
#include <type_traits>


namespace net_frame
{
  template <typename T>
  struct message_header
  {
      T id{};
      // size of the ertire message including this header
      uint32_t size = 0;
  };

  template <typename T>
  struct message
  {
    message_header<T> header{};
    std::vector<uint8_t> body;

    message() = default;
    message(T id):header{id,0}{}

    // print message
    friend std::ostream& operator<<(std::ostream& os, const message<T>& msg)
    {
      os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
      return os;
    }

    // pipe any data inside
    template<typename DataType>
    friend message<T>& operator<<(message<T>& msg, const DataType& data)
    {
      // DataType cannot be a string
      static_assert(std::is_same<std::string, std::decay<DataType>>::value == false, "String has its own implementation, this is an misuse");
      
      // Check if datatype can be serialized
      static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be serialized");

      size_t prev_size = msg.body.size();
      
      // resize vector
      msg.body.resize(msg.body.size() + sizeof(DataType));

      // copy data to vector
      std::memcpy(msg.body.data() + prev_size, &data, sizeof(DataType));

      // update message size
      msg.header.size = msg.body.size();

      return msg;
    }

    // same as above but only for std::string
    friend message<T>& operator<<(message<T>& msg, const std::string& str)
    {
      std::cout << "accual string size: " << str.size() << std::endl;
      size_t prev_size = msg.body.size();
      // resize vector
      msg.body.resize(msg.body.size() + str.size() + sizeof(std::string::size_type));
      // copy data to vector
      std::memcpy(msg.body.data() + prev_size, str.c_str(), str.size());
      // additionaly add size_t indicating string size
      auto str_size = str.size();
      std::memcpy(msg.body.data() + prev_size + str.size(), &str_size, sizeof(std::string::size_type));
      // update message size
      msg.header.size = msg.body.size();

      std::cout << "send string size: " << *(size_t*)(msg.body.data() + msg.body.size() - sizeof(std::string::size_type)) << std::endl;
      
      return msg;
    }

    // extract data from message
    template<typename DataType>
    friend message<T>& operator>>(message<T>& msg, DataType& data)
    {
      // DataType cannot be a string
      static_assert(std::is_same<std::string, std::decay<DataType>>::value == false, "String has its own implementation, this is an misuse");

      // Check if datatype can be serialized
      static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be serialized");
      assert(msg.body.size()>= sizeof(DataType) && "Cant extract this datatype");
      
      size_t new_size = msg.body.size() - sizeof(DataType);

      // copy data
      std::memcpy(&data, msg.body.data() + new_size, sizeof(DataType));

      // update vector size 
      msg.body.resize(new_size);

      // update header
      msg.header.size = msg.body.size();

      return msg;
    }
    // same as above but only for std::string
    friend message<T>& operator>>(message<T>& msg, std::string& str)
    {
      // firstly extract size of the string
      std::string::size_type str_size = *(std::string::size_type*)(msg.body.data() + msg.body.size() - sizeof(std::string::size_type));
      // std::cout << "string size: " << str_size << std::endl;
      size_t new_size = msg.body.size() - str_size - sizeof(std::string::size_type);
      // std::cout << "new_size: " << new_size << std::endl;
      
      // copy data
      // str.resize(str_size);
      // str.c_str() = *(msg.body.data() + new_size);
      str = std::string(reinterpret_cast<const char*>(msg.body.data() + new_size), str_size);

      // update vector size 
      msg.body.resize(new_size);

      // update header
      msg.header.size = msg.body.size();

      return msg;
    }
  };

  // Forward declare the connection
  template<typename T>
  class connection;
  
  template<typename T>
  struct owned_message
  {
    std::shared_ptr<connection<T>> remote = nullptr;
    message<T> msg;
    
    // overloaf print message
    friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg)
    {
      os << msg;
      return os;
    }
  }; 

}
