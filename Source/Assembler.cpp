#include "Assembler.h"
#include "Exceptions.h"

const string assembler::default_output_name = "output.txt";
const string assembler::machine_code_file_name = "temp";

assembler::assembler() : source_name_("") {
}

assembler::assembler(const string& source_name, const string& output_name, long start_address) :
	source_name_(source_name), output_name_(output_name), start_address_(start_address) {
}

void assembler::set(const string& source_name, const string& output_name, long start_address) {
	source_name_ = source_name;
	output_name_ = output_name;
	start_address_ = start_address;
}

bool assembler::assemble() {
	if (source_name_ == "") {
		return false;
	}

	source_.close();
	code_.close();

	source_.open(source_name_);
	code_.open(machine_code_file_name);
	if (!source_.is_open() || !code_.is_open()) {
		throw file_streams_exception();
	}

	sym_table_.clear();
	sym_table_.insert(sym_table_.end(), symbol("UND", 0, 0, symbol::global)); // 'Undefined' label
	rel_table_.clear();

	cout << ">>> Attempting first pass..." << endl;
	first_pass();
	cout << ">>> Attempting second pass..." << endl;
	second_pass();

	source_.close();
	code_.close();

	convert_to_text();
	cout << ">>> Assembling successful!" << endl;

	return true;
}


void assembler::get_formatted_line() {
	words_.clear();
	string line;

	do {
		getline(source_, line);
		++line_number_;

		remove_whitespaces(line);
		split(line);
	}
	while (words_.size() < 1 && !source_.eof());

	if (words_.size() < 1) return;

	head_ = words_[0];
	type_ = get_line_type();

	if (!is_op()) {
		return;
	}
	cond_type_ = get_cond_type();
	operand_count_ = operand_count();

	if (words_.size() != op_word_count()) {
		throw wrong_word_number_exception(line_number_);
	}
	for (int i = 0; i < operand_count_; ++i) {
		operand_type_[i] = get_operand_type(words_[i + 1]);
	}
}


void assembler::get_from_line() {
	words_.erase(words_.begin());
	head_ = words_[0];
	type_ = get_line_type();

	if (!is_op()) {
		return;
	}
	cond_type_ = get_cond_type();
	operand_count_ = operand_count();

	if (words_.size() != op_word_count()) {
		throw wrong_word_number_exception(line_number_);
	}
	for (int i = 0; i < operand_count_; ++i) {
		operand_type_[i] = get_operand_type(words_[i + 1]);
	}
}


void assembler::remove_whitespaces(string& line) {
	string::iterator i;
	for (i = line.begin(); i < line.end() && is_whitespace(*i); ++i);
	line.erase(line.begin(), i);

	for (i = line.end(); i >= line.begin() && (i == line.end() || is_whitespace(*i)); --i) {
		if (i == line.begin()) {
			line.erase(line.begin(), line.end());
			return;
		}
	}
	line.erase(++i, line.end());

	for (i = line.begin(); i < line.end();) { // Leave exactly 1 whitespace between each 2 words in a line
		while (i < line.end() && !is_whitespace(*i)) ++i;
		if (i == line.end()) break;
		string::iterator start = ++i;

		while (i < line.end() && is_whitespace(*i)) ++i;
		line.erase(start, i);
	}
	if (line.size() > 0 && (*(line.end() - 1) == '\r' || *(line.end() - 1) == '\n')) {
		line.erase(line.end() - 1, line.end());
	}
}


void assembler::split(string& line) {
	for (string::iterator i = line.begin(); i < line.end(); ++i) {
		string::iterator start = i;
		while (i < line.end() && !is_whitespace(*i)) ++i;
		words_.insert(words_.end(), string(start, i));

		if (i == line.end()) break;
	}
	if (words_.size() > 2) { // remove commas
		for (strings::iterator i = words_.begin() + 1; i < words_.end() - 1; ++i) {
			if (*(i->end() - 1) == ',') {
				i->erase(i->end() - 1);
			}
		}
	}
}


lang::line_type assembler::get_line_type() {
	if (*(head_.end() - 1) == ':') {
		if (is_label()) {
			head_.erase(head_.end() - 1);
			return lang::label;
		}
		throw unknown_line_exception(line_number_);
	}
	
	map<string, lang::line_type>::const_iterator it = lang::dir_map.find(head_);
	if (it != lang::dir_map.end()) {
		return it->second;
	}
	
	if (head_.size() < 3) {
		throw unknown_line_exception(line_number_);
	}
	string opName = string(head_.begin(), head_.end() - 2); // Remove condition (eq, ne, gt, al)
	it = lang::op_map.find(opName);
	if (it != lang::op_map.end()) {
		return it->second;
	}
	
	throw unknown_line_exception(line_number_);
}


lang::cond_type assembler::get_cond_type() {
	if (!is_op()) {
		throw unknown_line_exception(line_number_);
	}
	string condName = string(head_.end() - 2, head_.end());
	
	map<string, lang::cond_type>::const_iterator it = lang::cond_map.find(condName);
	if (it != lang::cond_map.end()) {
		return it->second;
	}
	throw unexpected_content_exception(line_number_);
}


lang::operand_type assembler::get_operand_type(const string& word) {
	if (is_integer(word)) {
		return lang::immed;
	}
	if (word == "psw") {
		return lang::psw;
	}

	if (word.size() > 1) {
		char sign = word[0];
		string data = string(word.begin() + 1, word.end());

		if ((sign == '&' || sign == '$') && is_decl_label(data)) {
			return lang::immed;
		}
		if ((sign == '*') && is_integer(data)) {
			return lang::immed_memory;
		}
	}

	if (word.size() > 5) {
		char sign[] = { word[2], *(word.end() - 1) }; // rX[sym]
		string data = string(word.begin() + 3, word.end() - 1);
		if (sign[0] == '[' && sign[1] == ']' && word[0] == 'r' && isdigit(word[1]) && (is_integer(data) || is_decl_label(data))) {
			return lang::reg_memory;
		}
	}
	if (word.size() == 2) {
		if (word[0] == 'r' && isdigit(word[1])) {
			return lang::reg;
		}
	}
	if (is_decl_label(word)) {
		return lang::immed_memory;
	}

	throw unexpected_content_exception(line_number_);
}


void assembler::check_symbol_definitions() {
	for (symbol_table::iterator i = sym_table_.begin() + 1; i < sym_table_.end(); ++i) {
		if (i->type != symbol::external && i->section == 0) {
			throw undefined_symbol_exception(i->name);
		}
	}
}


bool assembler::is_label() const {
	if (*(head_.end() - 1) != ':') {
		return false;
	}
	for (string::const_iterator i = head_.begin(); i < head_.end() - 1; ++i) {
		if (!is_ok(*i)) {
			return false;
		}
	}
	return true;
}


bool assembler::is_decl_label(const string& word) {
	for (string::const_iterator i = word.begin(); i < word.end(); ++i) {
		if (!is_ok(*i)) {
			return false;
		}
	}
	return true;
}


bool assembler::is_integer(const string& word) {
	for (string::const_iterator i = word.begin(); i < word.end(); ++i) {
		if (!isdigit(*i)
			&& !(i == word.begin() && (*i == '+' || *i == '-')))
			return false;
	}
	return true;
}


bool assembler::exists_data_operand() {
	if (!is_op() || operand_count_ == 0) {
		return false;
	}

	bool oneExists = false;
	for (int i = 0; i < operand_count_; ++i) {
		if (is_data_operand(i)) {
			if (oneExists) {
				throw two_data_operands_exception(line_number_);
			}
			oneExists = true;
		}
	}
	return oneExists;
}


void assembler::handle_operand(word& op_word_1, data_word& op_word_2, bool is_first_operand) {
	int index = is_first_operand ? 0 : 1;
	int offset = is_first_operand ? 1 : 2;
	string& operandWord = words_[offset];

	if (is_data_operand(index)) {
		const string data = get_data_word(operandWord);
		if (is_integer(data)) {
			op_word_2 = atoi(data.c_str());
		}
		else {
			relocation newRelocation(location_counter_, relocation::absolute, sizeof(word), 0);
			if (words_[offset][0] == '$') {
				newRelocation.type = relocation::pc_relative;
				op_word_2 = -(data_byte)(sizeof(data_word));
			}
			else {
				op_word_2 = 0;
			}

			symbol_table::const_iterator i;
			for (i = sym_table_.begin(); i < sym_table_.end(); ++i) {
				if (i->name == data) {
					if (i->type == symbol::local) {
						newRelocation.value = i->section;
						op_word_2 += i->value - sym_table_[i->section].value;
					}
					else {
						newRelocation.value = i - sym_table_.begin();
					}
					break;
				}
			}
			if (i == sym_table_.end()) {
				throw undefined_symbol_exception(data);
			}
			rel_table_.insert(rel_table_.end(), newRelocation);
		}
	}
	offset = is_first_operand ? 0 : lang::operand2_offset;
	if (type_ == lang::jmp_op) {
		op_word_1 |= (7 << lang::operand1_start_bit) | lang::operand_code_map.find(lang::reg)->second;
		offset = lang::operand2_offset;
	}

	op_word_1 |= lang::operand_code_map.find(operand_type_[index])->second >> offset;
	if (operand_type_[index] == lang::reg || operand_type_[index] == lang::reg_memory) {
		int registerNumber = atoi(string(operandWord.begin() + 1, operandWord.begin() + 2).c_str());
		if (registerNumber >= lang::num_of_registers) {
			throw register_number_overflow_exception(line_number_);
		}
		op_word_1 |= registerNumber << (lang::operand1_start_bit - offset);
	}
	if (operand_type_[index] == lang::psw) {
		op_word_1 |= 7 << (lang::operand1_start_bit - offset);
	}
}


const string assembler::get_data_word(const string& word) {
	if (is_integer(word)) {
		return word;
	}
	if (word.size() > 1) {
		char sign = word[0];
		string data = string(word.begin() + 1, word.end());

		if ((sign == '&' || sign == '$') && is_decl_label(data)) {
			return data;
		}
		if ((sign == '*') && is_integer(data)) {
			return data;
		}
	}

	if (word.size() > 5) {
		char sign[] = { word[2], *(word.end() - 1) }; // rX[sym]
		string data = string(word.begin() + 3, word.end() - 1);
		if (sign[0] == '[' && sign[1] == ']' && word[0] == 'r' && isdigit(word[1]) && (is_integer(data) || is_decl_label(data))) {
			return data;
		}
	}
	if (is_decl_label(word)) {
		return word;
	}
	return string(word.begin(), word.begin());
}


void assembler::convert_to_text() {
	output_.close();
	output_.open(output_name_);
	code_read_.open(machine_code_file_name);

	if (!output_.is_open() || !code_read_.is_open()) {
		throw file_streams_exception();
	}

	for (int i = start_address_; !code_read_.eof(); ++i) {
		byte c;
		c = code_read_.get();
		if (code_read_.eof()) break;
		bitset<8> x(c);
		output_ << i << ' ' << x << endl;
	}
	output_ << endl;
	code_read_.close();
	
	for (symbol_table::iterator i = sym_table_.begin(); i < sym_table_.end(); ++i) {
		output_ << i - sym_table_.begin() << ' ' << i->name << ' ' << i->section << ' ';
		if (i->type == symbol::local) {
			output_ << "local";
		}
		else if (i->type == symbol::global) {
			output_ << "global";
		}
		else {
			output_ << "extern";
		}
		output_ << ' ' << i->value << endl;
	}
	output_ << endl;

	for (relocation_table::iterator i = rel_table_.begin(); i < rel_table_.end(); ++i) {
		output_ << i - rel_table_.begin() << ' ' << i->offset << ' ';
		if (i->type == relocation::absolute) {
			output_ << "absolute";
		}
		else {
			output_ << "pc_relative";
		}
		output_ << ' ' << i->size << ' ' << i->value << endl;
	}
	output_.close();
}
