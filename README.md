# Multiplayer-Turn-Based-Game-Server-C-
A TCP multiplayer server written in C++ that matches players into turn-based games and enforces server-authoritative rules.

Features
Concurrent clients over TCP
JSON line-based protocol
Matchmaking queue (2 players -> session)
Server-authoritative Connect 4 game logic
Robust error handling and disconnect detection
Build
mkdir build && cd build cmake .. make

Run
./server --port 4000 ./client --host 127.0.0.1 --port 4000

Protocol
HELLO {name}
QUEUE
MOVE {col}
STATE, ERROR, GAME_OVER
