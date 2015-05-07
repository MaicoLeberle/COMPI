CC=g++

LEX=/usr/local/Cellar/flex/2.5.39/bin/flex
YACC=/usr/local/Cellar/bison/3.0.4/bin/bison

LDFLAGS=-L/usr/local/opt/bison/lib -L/usr/local/opt/flex/lib
CPPFLAGS=-I/usr/local/opt/flex/include -lfl

all: lexer.c
	$(CC) -oflexer lexer.cpp $(LDFLAGS) $(CPPFLAGS)

lexer.c:
	$(LEX) -olexer.cpp lexer/lexer.l

# TODO: Write guard for bison

clean:
	rm -f *~
	rm -f flexer
	rm -f lexer.cpp
