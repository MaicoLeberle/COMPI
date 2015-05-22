CC=g++

LEX=/usr/local/Cellar/flex/2.5.39/bin/flex
YACC=/usr/local/Cellar/bison/3.0.4/bin/bison

LDFLAGS=-L/usr/local/opt/bison/lib -L/usr/local/opt/flex/lib
CPPFLAGS=-I/usr/local/opt/flex/include -lfl

SRC=src
BUILD=build
TARGET=bin/compi
TEST=bin/test

MAIN=$(SRC)/main.cpp
TESTSRC=$(SRC)/test.cpp

LEXER=$(SRC)/parser/lexer.l
PARSER=$(SRC)/parser.cpp
PARSERH=$(SRC)/parser.tab.hpp
PARSERTAB=$(SRC)/parser.tab.cpp
PARSERSRC=$(SRC)/parser/parser.y

all: compi

compi: $(TARGET)

test: $(TESTSRC)
	$(TEST)

$(TARGET): $(PARSER)
	$(CC) -o$(TARGET) $(MAIN) $(PARSER) $(PARSERTAB) $(LDFLAGS) $(CPPFLAGS)

$(TESTSRC): $(PARSER)
	$(CC) -o$(TEST) $(TESTSRC) $(PARSER) $(PARSERTAB) $(LDFLAGS) $(CPPFLAGS)

$(PARSER): $(LEXER) $(PARSERH)
	$(LEX) -o$(PARSER) $(LEXER)

$(PARSERH): $(PARSERSRC)
	$(YACC) -d -o$(PARSERTAB) $(PARSERSRC)

clean:
	rm -f $(SRC)/*~
	rm -f $(TARGET)
	rm -f $(PARSER) $(PARSERTAB) $(PARSERH) parser.output
	rm -f $(TEST)
