#ifndef __EXCEPTIONS__
#define __EXCEPTIONS__

#include <exception>
#include <string>
using namespace std;

class file_streams_exception : public exception {
public:
	virtual const char* what() const throw() {
		return "*** Error! Files cannot open.";
	}
	virtual ~file_streams_exception() throw() {}
};

class start_outside_text_exception : public exception {
public:
	start_outside_text_exception() :
		msg("*** Error! Start symbol points to an address outside of text section.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~start_outside_text_exception() throw() {}
private:
	string msg;
};

class double_symbol_exception : public exception {
public:
	double_symbol_exception(const string& symbol) : symbol_(symbol),
		msg("*** Error! Symbol '" + symbol + "' is defined more than once.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~double_symbol_exception() throw() {}
private:
	string symbol_;
	string msg;
};

class outside_section_exception : public exception {
public:
	outside_section_exception(int line_num) : line_num_(line_num),
		msg("*** Error! Non-empty line " + to_string(line_num) + " is outside of any section.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~outside_section_exception() throw() {}
private:
	int line_num_;
	string msg;
};

class wrong_word_number_exception : public exception {
public:
	wrong_word_number_exception(int line_num) : line_num_(line_num),
		msg("*** Error! Line " + to_string(line_num) + " has an incorrect number of words in it.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~wrong_word_number_exception() throw() {}
private:
	int line_num_;
	string msg;
};

class unexpected_content_exception : public exception {
public:
	unexpected_content_exception(int line_num) : line_num_(line_num),
		msg("*** Error! Line " + to_string(line_num) + " contains an unexpected word.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~unexpected_content_exception() throw() {}
private:
	int line_num_;
	string msg;
};

class extern_symbol_defined_exception : public exception {
public:
	extern_symbol_defined_exception(int line_num, const string& symbol) : line_num_(line_num), symbol_(symbol),
		msg("*** Error! Symbol '" + symbol + "' found at line " + to_string(line_num) + " is defined but declared as extern.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~extern_symbol_defined_exception() throw() {}
private:
	int line_num_;
	string symbol_;
	string msg;
};

class declaration_conflict_exception : public exception {
public:
	declaration_conflict_exception(int line_num, const string& symbol) : line_num_(line_num), symbol_(symbol),
		msg("*** Error! Declaration at line " + to_string(line_num) + " is in conflict with another declaration.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~declaration_conflict_exception() throw() {}
private:
	int line_num_;
	string symbol_;
	string msg;
};

class unknown_line_exception : public exception {
public:
	unknown_line_exception(int line_num) : line_num_(line_num),
		msg("*** Error! Content at line " + to_string(line_num) + " is unknown.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~unknown_line_exception() throw() {}
private:
	int line_num_;
	string msg;
};

class invalid_section_exception : public exception {
public:
	invalid_section_exception(int line_num) : line_num_(line_num),
		msg("*** Error! Content at line " + to_string(line_num) + " is invalid for its section.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~invalid_section_exception() throw() {}
private:
	int line_num_;
	string msg;
};

class end_not_found_exception : public exception {
public:
	end_not_found_exception() :
		msg("*** Error! File doesn't contain the directive indicating the end of the program.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~end_not_found_exception() throw() {}
private:
	string msg;
};

class two_data_operands_exception : public exception {
public:
	two_data_operands_exception(int line_num) : line_num_(line_num),
		msg("*** Error! Operation at line " + to_string(line_num) + " has 2 operands which require additional bytes.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~two_data_operands_exception() throw() {}
private:
	int line_num_;
	string msg;
};

class undefined_symbol_exception : public exception {
public:
	undefined_symbol_exception(const string& symbol) : symbol_(symbol),
		msg("*** Error! Symbol " + symbol + " is declared, but not defined.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~undefined_symbol_exception() throw() {}
private:
	string symbol_;
	string msg;
};

class overflow_value_exception : public exception {
public:
	overflow_value_exception(int line_num) : line_num_(line_num),
		msg("*** Error! Variable at line " + to_string(line_num) + " is too large for its type.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~overflow_value_exception() throw() {}
private:
	int line_num_;
	string msg;
};

class undeclared_symbol_exception : public exception {
public:
	undeclared_symbol_exception(int line_num, const string& symbol) : line_num_(line_num), symbol_(symbol),
		msg("*** Error! Symbol " + symbol + " used at line " + to_string(line_num) + " is not declared.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~undeclared_symbol_exception() throw() {}
private:
	int line_num_;
	string symbol_;
	string msg;
};

class register_number_overflow_exception : public exception {
public:
	register_number_overflow_exception(int line_num) : line_num_(line_num),
		msg("*** Error! Operation at line " + to_string(line_num) + " uses a register that doesn't exist.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~register_number_overflow_exception() throw() {}
private:
	int line_num_;
	string msg;
};

class operation_without_memory_access : public exception {
public:
	operation_without_memory_access(int line_num) : line_num_(line_num),
		msg("*** Error! Operation at line " + to_string(line_num) + " has a memory operand, but the operation doesn't have access to the memory.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~operation_without_memory_access() throw() {}
private:
	int line_num_;
	string msg;
};

class operation_with_illegal_destination_exception : public exception {
public:
	operation_with_illegal_destination_exception(int line_num) : line_num_(line_num),
		msg("*** Error! Operation at line " + to_string(line_num) + " is a destination operation, but doesn't have a destination operand.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~operation_with_illegal_destination_exception() throw() {}
private:
	int line_num_;
	string msg;
};

class out_of_bounds_exception : public exception {
public:
	out_of_bounds_exception(int index) : index_(index),
		msg("*** Error! Memory location " + to_string(index) + " is out of bounds or two files overlap on that location.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~out_of_bounds_exception() throw() {}
private:
	int index_;
	string msg;
};

class start_not_defined_exception : public exception {
public:
	start_not_defined_exception() :
		msg("*** Error! Start is not defined.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~start_not_defined_exception() throw() {}
private:
	string msg;
};

class end_not_defined_exception : public exception {
public:
	end_not_defined_exception() :
		msg("*** Error! End is not defined.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~end_not_defined_exception() throw() {}
private:
	string msg;
};

class double_start_exception : public exception {
public:
	double_start_exception() :
		msg("*** Error! There is more than one 'start' label.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~double_start_exception() throw() {}
private:
	string msg;
};

class runtime_exception : public exception {
public:
	runtime_exception() :
		msg("*** Error! Uncaught runtime exception.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~runtime_exception() throw() {}
private:
	string msg;
};

class timer_not_defined_exception : public exception {
public:
	timer_not_defined_exception() :
		msg("*** Error! Timer bit is set, but timer interrupt is not defined.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~timer_not_defined_exception() throw() {}
private:
	string msg;
};

class multiple_emulator_exception : public exception {
public:
	multiple_emulator_exception() :
		msg("*** Error! Only one emulator can exist at a time.") {}

	virtual const char* what() const throw() {
		return msg.c_str();
	}
	virtual ~multiple_emulator_exception() throw() {}
private:
	string msg;
};

#endif
