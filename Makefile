
FLAGS := 

debug: FLAGS := ${FLAGS} -Wall -ggdb
debug: all

release: FLAGS := ${FLAGS} -O2
release: all

all: engine exe

engine:
	g++ card.cpp ${FLAGS} -I. -c -o ./build/engine.o 

exe:
	g++ client.cpp ./libraries/rlImGui/rlImGui.cpp ./build/engine.o ${FLAGS} -I. -I./include -lraylib -limgui -o ./build/run -std=c++20

