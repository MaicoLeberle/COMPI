CC=g++

OS=$(shell uname -s)

ifeq ($(OS),Darwin)
	LEX=/usr/local/Cellar/flex/2.5.39/bin/flex
	YACC=/usr/local/Cellar/bison/3.0.4/bin/bison
	LDFLAGS=-L/usr/local/opt/bison/lib -L/usr/local/opt/flex/lib
	CPPFLAGS=-I/usr/local/opt/flex/include
endif

ifeq ($(OS),Linux)
	LEX=/usr/bin/flex
	YACC=/usr/bin/bison
endif

CPPFLAGS+=-lfl -g -std=c++11 -Wno-deprecated-register
FLAGS=-g -std=c++11 -Wno-deprecated-register

SRC=src
BUILD=build
BIN=bin
TARGET=$(BIN)/compi
TEST=$(BIN)/test

MAINSRC=$(SRC)/main.cpp
TESTSRC=$(SRC)/test.cpp
VISITORSRC=$(SRC)/visitor.cpp
LEXERSRC=$(SRC)/parser/lexer.l
PARSERSRC=$(SRC)/parser/parser.y

LEXERC=$(BUILD)/lexer.cpp
PARSERC=$(BUILD)/parser.cpp
PARSERH=$(BUILD)/parser.hpp

MAIN=$(BUILD)/main.o
VISITOR=$(BUILD)/visitor.o
LEXER=$(BUILD)/lexer.o
PARSER=$(BUILD)/parser.o

all: compi

compi: $(TARGET)

test: clean $(TESTSRC)
	$(TEST)

$(TARGET): $(MAIN) $(VISITOR) $(LEXER) $(PARSER)
	$(CC) -o$(TARGET) $(MAIN) $(VISITOR) $(LEXER) $(PARSER) $(LDFLAGS) $(CPPFLAGS) $(LINKERFLAGS)

$(MAIN): $(MAINSRC)
	$(CC) -o$(MAIN) -c $(MAINSRC) $(FLAGS)

$(VISITOR): $(VISITORSRC)
	$(CC) -o$(VISITOR) -c $(VISITORSRC) $(FLAGS)

$(TESTSRC): $(LEXER)
	$(CC) -o$(TEST) $(TESTSRC) $(LEXER) $(PARSER) $(LDFLAGS) $(CPPFLAGS)

$(LEXER): $(LEXERC)
	$(CC) -o$(LEXER) $(LEXERC) -c $(FLAGS)

$(LEXERC): $(LEXERSRC) $(PARSER)
	$(LEX) -o$(LEXERC) $(LEXERSRC)

$(PARSER): $(PARSERH)
	$(CC) -o$(PARSER) $(PARSERC) -c $(FLAGS)

$(PARSERH): $(PARSERSRC)
	$(YACC) -d -o$(PARSERC) $(PARSERSRC)

clean:
	rm -f $(SRC)/*~
	rm -rf $(BIN)/*
	rm -f $(BUILD)/*
