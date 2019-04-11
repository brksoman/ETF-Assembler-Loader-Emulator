#include <initializer_list>
using namespace std;
#include "Symbol_table.h"
#include "Relocation_table.h"

const map<string, symbol::symbol_type> symbol::string_type = {
	{ "local", local },
	{ "global", global },
	{ "extern", external }
};

const map<string, relocation::relocation_type> relocation::string_type = {
	{ "absolute", absolute },
	{ "pc_relative", pc_relative }
};
