#ifndef __LOADER__
#define __LOADER__

#include <bitset>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

#include "Language_specification.h"
#include "Symbol_table.h"
#include "Relocation_table.h"

typedef vector<ifstream> files;
//using files = vector<ifstream>;

class loader {
public:
	loader() {
		construct();
	}
	loader(strings& source_names) {
		construct();
		source_names_.assign(source_names.begin(), source_names.end());
	}

	void source_names(string& source_name) { source_names_.insert(source_names_.end(), source_name); }
	const strings& source_names() const { return source_names_; }

	byte* memory() { return memory_; }
	symbol_table& sym_table() { return sym_table_; }
	word start_address();
	word end_address();

	void load();
	void relocate();

private:
	void construct() {
		memory_ = new byte[lang::memory_size];
		is_free_ = new bool[lang::memory_size];
	}
	void check_symbol_definitions();

	strings source_names_;
	
	byte* memory_;
	bool* is_free_;
	symbol_table sym_table_;
	relocation_table rel_table_;

	symbol_table::iterator local_starting_symbol_;
	symbol_table local_sym_table_;
};

#endif
