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
TEST_SUITE=$(BIN)/test

MAINSRC=$(SRC)/main.cpp
TESTSRC=$(SRC)/test.cpp
SEMANTIC_ANALYSISSRC=$(SRC)/semantic_analysis.cpp
SYMTABLESRC=$(SRC)/symtable.cpp
LEXERSRC=$(SRC)/parser/lexer.l
PARSERSRC=$(SRC)/parser/parser.y

LEXERC=$(BUILD)/lexer.cpp
PARSERC=$(BUILD)/parser.cpp
PARSERH=$(BUILD)/parser.hpp

MAIN=$(BUILD)/main.o
TEST_MAIN=$(BUILD)/test_main.o
SEMANTIC_ANALYSIS=$(BUILD)/semantic_analysis.o
SYMTABLE=$(BUILD)/symtable.o
LEXER=$(BUILD)/lexer.o
PARSER=$(BUILD)/parser.o

all: compi test_suite

compi: $(TARGET)

test_suite: $(TEST_SUITE)

$(TARGET): $(MAIN) $(SEMANTIC_ANALYSIS) $(SYMTABLE) $(LEXER) $(PARSER)
	$(CC) -o$(TARGET) $(MAIN) $(SEMANTIC_ANALYSIS) $(SYMTABLE) $(LEXER) $(PARSER) $(LDFLAGS) $(CPPFLAGS) $(LINKERFLAGS)

$(TEST_SUITE): $(TEST_MAIN) $(SEMANTIC_ANALYSIS) $(SYMTABLE) $(LEXER) $(PARSER)
	$(CC) -o$(TEST_SUITE) $(TEST_MAIN) $(SEMANTIC_ANALYSIS) $(SYMTABLE) $(LEXER) $(PARSER) $(LDFLAGS) $(CPPFLAGS) $(LINKERFLAGS)

$(TEST_MAIN): $(TESTSRC)
	$(CC) -o$(TEST_MAIN) -c $(TESTSRC) $(FLAGS)

$(MAIN): $(MAINSRC)
	$(CC) -o$(MAIN) -c $(MAINSRC) $(FLAGS)
	
$(SYMTABLE): $(SYMTABLESRC)
	$(CC) -o$(SYMTABLE) -c $(SYMTABLESRC) $(FLAGS)

$(SEMANTIC_ANALYSIS): $(SEMANTIC_ANALYSISSRC)
	$(CC) -o$(SEMANTIC_ANALYSIS) -c $(SEMANTIC_ANALYSISSRC) $(FLAGS) -D__DEBUG

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
