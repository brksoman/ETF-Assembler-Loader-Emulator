#ifndef __ASSEMBLER_H__
#define __ASSEMBLER_H__

#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <algorithm>
using namespace std;

#include "Language_specification.h"
#include "Symbol_table.h"
#include "Relocation_table.h"

class assembler {
public:
																	// Interface
	assembler();
	assembler(const string& source_name, const string& output_name = default_output_name, long start_location = 0);
	
	void set(const string& source_name, const string& output_name, long start_address);

	bool assemble();

	const string& output_name() const { return output_name_; }
	void output_name(const string& output_name_) { this->output_name_ = output_name_; }

	const string& source_name() const { return source_name_; }
	void source_name(const string& source_name_) { this->source_name_ = source_name_; }

	const symbol_table& sym_table() const { return sym_table_; }

private:
																	// Main methods
	void first_pass();
	void second_pass();

	void convert_to_text();

	void get_formatted_line();
		// If line is like <label>: <operation>
	void get_from_line();

		// Checks if all non-extern symbols are defined
	void check_symbol_definitions();

																	// Line formatting methods
	static void remove_whitespaces(string& line);
	void split(string& line);
	
	lang::line_type get_line_type();
	lang::cond_type get_cond_type();
	lang::operand_type get_operand_type(const string&);

																	// Line type
	bool is_section() const;
	bool is_var_dir() const;
	bool is_op() const;

																	// Operand
	bool is_data_operand(int operand_index);
	bool is_memory_operand(int operand_index);
	bool is_legal_destination_operand(int operand_index);
	bool exists_data_operand();
	int operand_count() const;

	void handle_operand(word& first_op_word, data_word& second_op_word, bool is_first_operand);
	const static string get_data_word(const string&);

																	// Operation
	bool is_destination_op() const;
	bool is_pcrelative_op() const;
	bool is_memory_op() const;
	int op_word_count() const;

																	// Checks
	static bool is_ok(char c) { return isalpha(c) || isdigit(c) || c == '_'; }
	static bool is_whitespace(char c) { return c == ' ' || c == '\t'; }

	bool is_label() const; // Label with ':'
	static bool is_decl_label(const string&); // Label without ':'
	static bool is_integer(const string&);

																	// Private fields
	string source_name_;
	string output_name_;

	long start_address_;

	mutable ifstream source_;
	mutable ofstream code_;
	mutable ifstream code_read_;
	mutable ofstream output_;

	mutable symbol_table sym_table_;
	mutable relocation_table rel_table_;

																	// Current line info
	mutable strings words_;
	mutable string head_;

	mutable long line_number_;
	mutable long location_counter_;
	mutable long current_section_;

	mutable lang::line_type type_;
	mutable lang::cond_type cond_type_;

	mutable int operand_count_;
	mutable lang::operand_type operand_type_[2];

																	// Constants
	static const string default_output_name;
	static const string machine_code_file_name;
};


inline bool assembler::is_section() const {
	return find(lang::sections.begin(), lang::sections.end(), type_) != lang::sections.end();
}
inline bool assembler::is_var_dir() const {
	return find(lang::vars.begin(), lang::vars.end(), type_) != lang::vars.end();
}
inline bool assembler::is_op() const {
	return find(lang::operations.begin(), lang::operations.end(), type_) != lang::operations.end();
}
inline bool assembler::is_pcrelative_op() const {
	return find(lang::pcrelative_operations.begin(), lang::pcrelative_operations.end(), type_) != lang::pcrelative_operations.end();
}

inline int assembler::operand_count() const {
	if (find(lang::operations_with_0operand.begin(), lang::operations_with_0operand.end(), type_) != lang::operations_with_0operand.end())
		return 0;
	if (find(lang::operations_with_1operand.begin(), lang::operations_with_1operand.end(), type_) != lang::operations_with_1operand.end())
		return 1;
	if (find(lang::operations_with_2operand.begin(), lang::operations_with_2operand.end(), type_) != lang::operations_with_2operand.end())
		return 2;
	return -1;
}

inline int assembler::op_word_count() const {
	return operand_count() + 1;
}
inline bool assembler::is_data_operand(int i) {
	return find(lang::data_operands.begin(), lang::data_operands.end(), operand_type_[i]) != lang::data_operands.end();
}
inline bool assembler::is_memory_operand(int i) {
	return find(lang::memory_operands.begin(), lang::memory_operands.end(), operand_type_[i]) != lang::memory_operands.end();
}
inline bool assembler::is_legal_destination_operand(int i) {
	return find(lang::legal_destination_operands.begin(), lang::legal_destination_operands.end(), operand_type_[i]) != lang::legal_destination_operands.end();
}
inline bool assembler::is_destination_op() const {
	return find(lang::operations_with_destination_operand.begin(), lang::operations_with_destination_operand.end(), type_) != lang::operations_with_destination_operand.end();
}
inline bool assembler::is_memory_op() const {
	return find(lang::operations_with_memory.begin(), lang::operations_with_memory.end(), type_) != lang::operations_with_memory.end();
}

#endif
