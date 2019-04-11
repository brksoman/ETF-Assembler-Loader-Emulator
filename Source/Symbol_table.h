#ifndef __SYMBOL_TABLE__
#define __SYMBOL_TABLE__

#include <string>
#include <map>
#include <vector>
using namespace std;

struct symbol {
	enum symbol_type {
		local, global, external
	};

	string name;
	int section;
	int value;
	symbol_type type;
	
	symbol(string name_, int section_, int value_, symbol_type type_) :
		name(name_), section(section_), value(value_), type(type_) {
	}

	static symbol_type string_to_type(string word) {
		map<string, symbol_type>::const_iterator i = string_type.find(word);
		return (i != string_type.end()) ? string_type.find(word)->second : local;
	}

private:
	static const map<string, symbol_type> string_type;
};

typedef vector<symbol> symbol_table;
//using symbol_table = vector<symbol>;

#endif
