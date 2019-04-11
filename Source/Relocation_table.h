#ifndef __RELOCATION_TABLE__
#define __RELOCATION_TABLE__

#include <string>
#include <vector>
#include <map>
using namespace std;

struct relocation {
	enum relocation_type {
		absolute, pc_relative
	};

	long offset;
	relocation_type type;
	int size;
	long value;

	relocation(long offset_, relocation_type type_, int size_, long value_) :
		offset(offset_), type(type_), size(size_), value(value_) {
	}

	static relocation_type string_to_type(string word) {
		map<string, relocation_type>::const_iterator i = string_type.find(word);
		return (i != string_type.end()) ? i->second : absolute;
	}

private:
	static const map<string, relocation_type> string_type;
};

typedef vector<relocation> relocation_table;
//using relocation_table = vector<relocation>;

#endif
