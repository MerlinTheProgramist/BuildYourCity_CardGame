# Build Your City (WIP)
multiplayer port of a card game "Build Your City" made with c++ and raylib.  
The multiplayer is implemented with my custom networking library [Soonic++](https://github.com/MerlinTheProgramist/Sonicpp).

# About 
the projects consists of:
1. Independent c++ game engine
2. Raylib client that can be a host or client (the host runs a server thread in the background)

# Features 
## Lobby creation
Become a host and create a lobby with specified port.
Join a lobby with ip address and the port.
## Individualized connections
The host only shares relevant information to each client.
## Server side player's action verification
the server (run by host) does not trust clients and verifies every action.
## Procedural Card Textures
Card textures are generated on the start of the client according to the list of card info.
# Development Informatoin
## Client

- Each client holds exact copy of its `Player` object, with server sync
- Each client holds brief info about other players `PlayerView[]`, entirely controlled by server
- client commits its every action to server
- client asks server for action verification

## Game State
- using state design pattern (finite state machine)

## Actions (sent to server)
- CardSelect = client selects a card (from thier hand) or (from eventHandler deck)
- CanProgress = ask server if player is allowed to progress

## To Do 
- make card texture generation a part of compilation  
- game event log
