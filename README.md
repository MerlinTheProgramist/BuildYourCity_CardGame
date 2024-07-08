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
- Progress = 
