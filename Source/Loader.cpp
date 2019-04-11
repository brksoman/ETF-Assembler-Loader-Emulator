#include "Loader.h"
#include "Exceptions.h"

void loader::load() {
	for (int i = 0; i < lang::memory_size; ++i) is_free_[i] = true;
	sym_table_.clear();
	sym_table_.insert(sym_table_.end(), symbol("UND", 0, 0, symbol::global)); // 'Undefined' label
	rel_table_.clear();

	for (strings::iterator i = source_names_.begin(); i < source_names_.end(); ++i) {
		ifstream source;
		source.open(*i);

		if (!source.is_open()) {
			throw file_streams_exception();
		}

		string line;

		// Machine code
		for (getline(source, line); line != ""; getline(source, line)) {
			int index;
			bitset<8> x;
			stringstream ss(line);
			
			ss >> index >> x;
			
			if (index < lang::min_code_address || index >= lang::max_code_address || !is_free_[index]) {
				throw out_of_bounds_exception(index);
			}
			memory_[index] = (byte)x.to_ulong();
			is_free_[index] = false;
		}

		// Symbol table
		local_sym_table_.clear();
		local_starting_symbol_ = sym_table_.end();

		for (getline(source, line); line != ""; getline(source, line)) {
			int index, section, value;
			string name, scope;
			stringstream ss(line);

			ss >> index >> name >> section >> scope >> value;

			symbol::symbol_type symType = symbol::string_to_type(scope);
			if (name[0] == '.') {
				name += to_string(i - source_names_.begin());
			}

			local_sym_table_.insert(local_sym_table_.end(), symbol(name, section, value, symType));
			
			if (name == "UND" || symType == symbol::local) {
				continue;
			}

			symbol_table::iterator it;
			for (it = sym_table_.begin(); it < sym_table_.end(); ++it) {
				if (it->name == name) {
					break;
				}
			}
			if (it == sym_table_.end()) {
				sym_table_.insert(sym_table_.end(), symbol(name, section, value, symType));
				continue;
			}

			if (symType == symbol::global) {
				if (it->type == symbol::global) {
					throw double_symbol_exception(name);
				}
				else {
					it->type = symbol::global;
					it->value = value;
				}
			}
		}

		// Relocation table
		for (getline(source, line); line != ""; getline(source, line)) {
			int index, offset, size, value;
			string type;
			stringstream ss(line);

			ss >> index >> offset >> type >> size >> value;
			relocation::relocation_type relType = relocation::string_to_type(type);

			string name = local_sym_table_[value].name;
			symbol_table::iterator it;
			for (it = sym_table_.begin(); it < sym_table_.end(); ++it) {
				if (it->name == name) {
					break;
				}
			}
			value = it - sym_table_.begin();
			rel_table_.insert(rel_table_.end(), relocation(offset, relType, size, value));
		}
	}
	check_symbol_definitions();
	relocate();
}


void loader::relocate() {
	for (relocation_table::iterator i = rel_table_.begin(); i < rel_table_.end(); ++i) {
		symbol sym = sym_table_[i->value];

		if (i->type == relocation::absolute) {
			*((word*)(memory_ + i->offset)) += sym.value;
		}
		else {
			*((word*)(memory_ + i->offset)) += sym.value - i->offset;
		}
	}
	// Relocate interrupts (implicit relocations)
	for (map<word, string>::const_iterator i = lang::ivt.begin(); i != lang::ivt.end(); ++i) {
		symbol_table::iterator symI;
		for (symI = sym_table_.begin(); symI < sym_table_.end(); ++symI) {
			if (symI->name == i->second) {
				break;
			}
		}
		if (symI == sym_table_.end()) {
			if (i->second == "_start") {
				throw start_not_defined_exception();
			}
			memory_[2 * i->first] = 0;
			memory_[2 * i->first + 1] = 0;
		}
		else {
			memory_[2 * i->first] = (byte)symI->value;
			memory_[2 * i->first + 1] = (byte)(symI->value >> (sizeof(byte) * 8));
		}
	}
}


void loader::check_symbol_definitions() {
	for (symbol_table::iterator i = sym_table_.begin(); i < sym_table_.end(); ++i) {
		if (i->type != symbol::global) {
			throw undefined_symbol_exception(i->name);
		}
	}
}


word loader::start_address() {
	for (symbol_table::iterator i = sym_table_.begin(); i < sym_table_.end(); ++i) {
		if (i->name == "_start") {
			return i->value;
		}
	}
	throw start_not_defined_exception();
}


word loader::end_address() {
	for (symbol_table::iterator i = sym_table_.begin(); i < sym_table_.end(); ++i) {
		if (i->name == "_end") {
			return i->value;
		}
	}
	throw end_not_defined_exception();
}