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
TEST_INTER_CODE_GEN_VISITORSRC=$(SRC)/tests/test_inter_code_gen_visitor.cpp
SEMANTIC_ANALYSISSRC=$(SRC)/semantic_analysis.cpp
INTER_CODE_GEN_VISITORSRC=$(SRC)/inter_code_gen_visitor.cpp
SYMTABLESRC=$(SRC)/symtable.cpp
LEXERSRC=$(SRC)/parser/lexer.l
PARSERSRC=$(SRC)/parser/parser.y
ASTSRC=$(SRC)/node.h
THREE_ADDRESS_CODE_SRC=$(SRC)/three_address_code.cpp

LEXERC=$(BUILD)/lexer.cpp
PARSERC=$(BUILD)/parser.cpp
PARSERH=$(BUILD)/parser.hpp

MAIN=$(BUILD)/main.o
TEST_MAIN=$(BUILD)/test_main.o
TEST_INTER_CODE_GEN_VISITOR=$(BUILD)/test_inter_code_gen_visitor.o
SEMANTIC_ANALYSIS=$(BUILD)/semantic_analysis.o
INTER_CODE_GEN_VISITOR=$(BUILD)/inter_code_gen_visitor.o
THREE_ADDRESS_CODE=$(BUILD)/three_address_code.o
SYMTABLE=$(BUILD)/symtable.o
LEXER=$(BUILD)/lexer.o
PARSER=$(BUILD)/parser.o

all: compi test_suite

compi: $(TARGET)

test_suite: $(TEST_SUITE)

$(TARGET): $(MAIN) $(SEMANTIC_ANALYSIS) $(SYMTABLE) $(LEXER) $(PARSER) $(INTER_CODE_GEN_VISITOR) $(THREE_ADDRESS_CODE)
	$(CC) -o$(TARGET) $(MAIN) $(SEMANTIC_ANALYSIS) $(INTER_CODE_GEN_VISITOR) $(THREE_ADDRESS_CODE) $(SYMTABLE) $(LEXER) $(PARSER) $(LDFLAGS) $(CPPFLAGS) $(LINKERFLAGS)

$(TEST_SUITE): $(TEST_MAIN) $(SEMANTIC_ANALYSIS) $(SYMTABLE) $(LEXER) $(PARSER) $(TEST_INTER_CODE_GEN_VISITOR)
	$(CC) -o$(TEST_SUITE) $(TEST_MAIN) $(SEMANTIC_ANALYSIS) $(INTER_CODE_GEN_VISITOR) $(THREE_ADDRESS_CODE) $(SYMTABLE) $(TEST_INTER_CODE_GEN_VISITOR) $(LEXER) $(PARSER) $(LDFLAGS) $(CPPFLAGS) $(LINKERFLAGS)

$(TEST_INTER_CODE_GEN_VISITOR): $(TEST_INTER_CODE_GEN_VISITORSRC) $(INTER_CODE_GEN_VISITOR)
	$(CC) -o$(TEST_INTER_CODE_GEN_VISITOR) -c $(TEST_INTER_CODE_GEN_VISITORSRC) $(FLAGS)

$(TEST_MAIN): $(TESTSRC)
	$(CC) -o$(TEST_MAIN) -c $(TESTSRC) $(FLAGS)
	
$(MAIN): $(MAINSRC)
	$(CC) -o$(MAIN) -c $(MAINSRC) $(FLAGS)
	
$(INTER_CODE_GEN_VISITOR): $(INTER_CODE_GEN_VISITORSRC) $(THREE_ADDRESS_CODE_SRC)
	$(CC) -o$(INTER_CODE_GEN_VISITOR) -c $(INTER_CODE_GEN_VISITORSRC) $(FLAGS) -D__DEBUG
	
$(THREE_ADDRESS_CODE): $(THREE_ADDRESS_CODE_SRC)
	$(CC) -o$(THREE_ADDRESS_CODE) -c $(THREE_ADDRESS_CODE_SRC) $(FLAGS)
	
$(SYMTABLE): $(SYMTABLESRC)
	$(CC) -o$(SYMTABLE) -c $(SYMTABLESRC) $(FLAGS)

$(SEMANTIC_ANALYSIS): $(SEMANTIC_ANALYSISSRC) $(PARSER)
	$(CC) -o$(SEMANTIC_ANALYSIS) -c $(SEMANTIC_ANALYSISSRC) $(FLAGS) -D__DEBUG

$(LEXER): $(LEXERC)
	$(CC) -o$(LEXER) $(LEXERC) -c $(FLAGS)

$(LEXERC): $(LEXERSRC) $(PARSER)
	$(LEX) -o$(LEXERC) $(LEXERSRC)

$(PARSER): $(PARSERH)
	$(CC) -o$(PARSER) $(PARSERC) -c $(FLAGS)

$(PARSERH): $(PARSERSRC) $(ASTSRC)
	$(YACC) -d -o$(PARSERC) $(PARSERSRC)

clean:
	rm -f $(SRC)/*~
	rm -rf $(BIN)/*
	rm -f $(BUILD)/*
