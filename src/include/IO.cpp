#include <iostream>
#include "prints.h"

using namespace std;

void print_string(const char* str) {
	cout << str;
}

void print_int(const int i) {
	cout << i;
}

int read_int() {
	int aux;
	cin >> aux;
	return (aux);
}
