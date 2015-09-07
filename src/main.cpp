#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <cassert>
#include "node.h"
#include "semantic_analysis.h"

extern program_pointer ast;
extern int yyparse();
extern FILE *yyin;

enum target{
      PARSE = 1
    , SEMANTICS = 2
    , INTERCODE = 3
    , ASSEMBLY = 4
};

void parseArguments(char**, char**, FILE*&, FILE*&, enum target&, bool&);
bool endsWith(std::string const&, std::string const&);


int main(int argc, const char* argv[]) {
    /*  If only program name has been given to command line interface, then
        print this help message.                                             */
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [OPTIONS] FILE" << std::endl;
        std::cout << std::endl << "OPTIONS:" << std::endl;
        std::cout << "  -o OUTPUT      Renames the executable file to OUTPUT." << std::endl;
        std::cout << "  -target STAGE  STAGE is either \"parse\", \"semantics\", \"intercode\", or \"assembly\"." << std::endl;
        std::cout << "  -debug         Prints debugging information." << std::endl;
        exit(EXIT_SUCCESS);
    }

    FILE* input_file = NULL;
    FILE* output_file = NULL;
    enum target stage;
    bool debugging;
    parseArguments(argv, argv+argc, input_file, output_file, stage, debugging);

    /*  Set flex to read from it instead of defaulting to STDIN.             */
    yyin = input_file;

    if(stage >= PARSE) {
        /*  Parse through all the input.                                         */
        do {
            yyparse();
        } while (!feof(yyin));
    }

    fclose(input_file);

    if(stage >= SEMANTICS) {
        /*  Perform semantic analysis.                                       */
        semantic_analysis v1;
        ast->accept(v1);
    } else exit(EXIT_SUCCESS);

    if(stage >= INTERCODE) {
        /*  Generate intermediate code.                                      */
    } else exit(EXIT_SUCCESS);

    if(stage == ASSEMBLY) {
        /*  Generate assembly code from intermediate representation.         */
    } else exit(EXIT_SUCCESS);

    return 0;
}

void parseArguments(char** beginIt, char** endIt, FILE*& input_file, FILE*& output_file, enum target& stage, bool& debugging) {
    char** itr = beginIt;
    itr++; // First argument is program name

    debugging = false;

    bool input_file_done = false;
    bool output_file_done = false;
    bool stage_done = false;
    while(itr != endIt) {
        if(std::string("-o").compare(*itr) == 0) {
            if(output_file_done) {
                std::cout << "Output file specified twice." << std::endl;
                exit(EXIT_SUCCESS);
            } else if(++itr == endIt) {
                std::cout << "Missing mandatory argument for -o option." << std::endl;
                exit(EXIT_SUCCESS);
            } else {
                output_file = fopen(*itr, "w");
                if(!output_file) {
                    std::cout << "Unable to open output file(" << *itr << ")." << std::endl;
                    exit(EXIT_SUCCESS);
                }
            }
            output_file_done = true;
        } else if (std::string("-target").compare(*itr) == 0) {
            if(stage_done) {
                std::cout << "Target stage specified twice." << std::endl;
                exit(EXIT_SUCCESS);
            } else if(++itr == endIt) {
                std::cout << "Missing mandatory argument for -target option." << std::endl;
                exit(EXIT_SUCCESS);
            } else {
                if (std::string("parse").compare(*itr) == 0) {
                    stage = PARSE;
                } else if (std::string("semantics").compare(*itr) == 0) {
                    stage = SEMANTICS;
                } else if (std::string("intercode").compare(*itr) == 0) {
                    stage = INTERCODE;
                } else if (std::string("assembly").compare(*itr) == 0) {
                    stage = ASSEMBLY;
                } else {
                    std::cout << "Invalid target stage." << std::endl;
                    std::cout << "Options are: \"parse\", \"semantics\", \"intercode\", or \"assembly\"." << std::endl;
                    exit(EXIT_SUCCESS);
                }
            }
            stage_done = true;
        } else if (std::string("-debug").compare(*itr) == 0) {
            debugging = true;
        } else {
            input_file = fopen(*itr, "r");
            if((*itr)[0] == '-') {
                std::cout << "Invalid option." << std::endl;
                exit(EXIT_SUCCESS);
            }

            if(!input_file) {
                std::cout << "Unopenable file." << std::endl;
                exit(EXIT_SUCCESS);
            }

            if(!endsWith(std::string(*itr), std::string(".compi"))) {
                std::cout << "Invalid extension of input file." << std::endl;
                exit(EXIT_SUCCESS);
            }
            input_file_done = true;
        }
        ++itr;
    }

    if(!input_file_done) {
        std::cout << "Missing input file." << std::endl;
        exit(EXIT_SUCCESS);
    }

    if(!output_file_done) 
        output_file = fopen("a.out", "w");

    if (!stage_done) 
        stage = ASSEMBLY;
}

bool endsWith(std::string const& fullString, std::string const& ending) {
    if (fullString.length() >= ending.length())
        return (fullString.compare (fullString.length() - ending.length(), ending.length(), ending) == 0);
    return false;
}