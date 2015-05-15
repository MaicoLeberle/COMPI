CC=g++

LEX=/usr/local/Cellar/flex/2.5.39/bin/flex
YACC=/usr/local/Cellar/bison/3.0.4/bin/bison

LDFLAGS=-L/usr/local/opt/bison/lib -L/usr/local/opt/flex/lib
CPPFLAGS=-I/usr/local/opt/flex/include -lfl

all: parser.cpp
	$(CC) -ocompi main.cpp parser.cpp parser.tab.cpp $(LDFLAGS) $(CPPFLAGS)

test: parser.cpp
	$(CC) -otest test.cpp parser.cpp parser.tab.cpp $(LDFLAGS) $(CPPFLAGS)

parser.cpp: parser/parser.l parser.tab.hpp
	$(LEX) -oparser.cpp parser/parser.l

parser.tab.hpp: parser/parser.y
	$(YACC) -d -oparser.tab.cpp parser/parser.y

clean:
	rm -f *~
	rm -f compi
	rm -f parser.cpp parser.tab.* parser.output
	rm -f test
