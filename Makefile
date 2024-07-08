
CC = g++
CFLAGS =  -std=c++17

SRCDIR = ./src
LIBSRCDIR = $(SRCDIR)/engine
OBJDIR = ./obj

ENGINE = build/engine.a
APPNAME = build/BuildYourCity


LIBSOURCES  := $(filter-out $(LIBSRCDIR)/test.cpp, $(wildcard $(LIBSRCDIR)/*.cpp))
LIBOBJECTS  := $(LIBSOURCES:$(LIBSRCDIR)/%.cpp=$(OBJDIR)/%.o)

INCLUDES := $(wildcard $(SRCDIR)/*.h)

.PHONY: engine debug release

debug: FLAGS := ${FLAGS} -Wall -ggdb
debug: all

release: FLAGS := ${FLAGS} -O2
release: all
	
.PHONY: all
all: $(APPNAME)

$(LIBOBJECTS): $(OBJDIR)/%.o : $(LIBSRCDIR)/%.cpp $(LIBSRCDIR)/*.h*
	@$(CC) $(CFLAGS) -c $< -o $@ 
	@echo "Compiled "$<" successfully!"

$(ENGINE): $(LIBOBJECTS) 
	@ar -rcs $@ $(LIBOBJECTS)
	@echo "Compiled game-engine successfully!"


$(APPNAME): $(ENGINE) $(SRCDIR)/*.cpp $(SRCDIR)/*.h*
	@$(CC) -o $@ ${CFLAGS} -I./include -lraylib -limgui ./libraries/rlImGui/rlImGui.cpp $(SRCDIR)/*.cpp $(ENGINE)
	@echo "Compiled $@ successfully!"


engine: $(ENGINE)
