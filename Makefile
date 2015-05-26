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

SRC=src
BUILD=build
TARGET=bin/compi
TEST=bin/test

MAIN=$(SRC)/main.cpp
TESTSRC=$(SRC)/test.cpp

LEXER=$(BUILD)/lexer.cpp
PARSER=$(BUILD)/parser.cpp
PARSERH=$(BUILD)/parser.hpp
LEXERSRC=$(SRC)/parser/lexer.l
PARSERSRC=$(SRC)/parser/parser.y

all: compi

compi: $(TARGET)

test: clean $(TESTSRC)
	$(TEST)

$(TARGET): $(LEXER)
	$(CC) -o$(TARGET) $(MAIN) $(LEXER) $(PARSER) $(LDFLAGS) $(CPPFLAGS)

$(TESTSRC): $(LEXER)
	$(CC) -o$(TEST) $(TESTSRC) $(LEXER) $(PARSER) $(LDFLAGS) $(CPPFLAGS)

$(LEXER): $(LEXERSRC) $(PARSERH)
	$(LEX) -o$(LEXER) $(LEXERSRC)

$(PARSERH): $(PARSERSRC)
	$(YACC) -d -o$(PARSER) $(PARSERSRC)

clean:
	rm -f $(SRC)/*~
	rm -f $(TARGET)
	rm -f $(LEXER) $(PARSER) $(PARSERH)
	rm -f $(TEST)
