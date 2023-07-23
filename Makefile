
FLAGS := 

debug: FLAGS := ${FLAGS} -Wall -gdwarf
debug: all

release: FLAGS := ${FLAGS} -O2
release: all

all: engine exe

engine:
	g++ card.cpp ${FLAGS} -I. -c -o ./build/engine.o 

exe:
	g++ main.cpp ./build/engine.o ${FLAGS} -I. -lraylib -o ./build/run

# debug:
# 	@./build/run