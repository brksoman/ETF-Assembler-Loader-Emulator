#include "Assembler.h"
#include "Exceptions.h"

void assembler::first_pass() {
	current_section_ = -1;
	location_counter_ = start_address_;
	line_number_ = 0;

	get_formatted_line();
	while (type_ != lang::end_dir && !source_.eof()) {
		if (current_section_ < 0 && !is_section()) {
			throw outside_section_exception(line_number_);
		}

		// Switch
		if (type_ == lang::label) {
			symbol_table::iterator i;
			for (i = sym_table_.begin(); i < sym_table_.end(); ++i) {
				if (i->name == head_) {
					if (i->section != 0) { // Is defined
						throw double_symbol_exception(head_);
					}
					if (i->type == symbol::external) {
						throw extern_symbol_defined_exception(line_number_, head_);
					}

					i->section = current_section_;
					i->value = location_counter_;
					break;
				}
			}
			if (i == sym_table_.end()) {
				sym_table_.insert(sym_table_.end(), symbol(head_, current_section_, location_counter_, symbol::local));
			}
		}

		else if (is_section()) {
			if (words_.size() != 1) {
				throw wrong_word_number_exception(line_number_);
			}

			for (symbol_table::iterator i = sym_table_.begin(); i < sym_table_.end(); ++i) {
				if (i->name == head_) {
					throw double_symbol_exception(head_);
				}
			}
			current_section_ = sym_table_.size();
			sym_table_.insert(sym_table_.end(), symbol(head_, current_section_, location_counter_, symbol::global));
		}

		else if (type_ == lang::extern_dir) {
			if (words_.size() < 2) {
				throw wrong_word_number_exception(line_number_);
			}
			for (strings::iterator i = words_.begin() + 1; i < words_.end(); ++i) {
				if (!is_decl_label(*i)) {
					throw unexpected_content_exception(line_number_);
				}
				for (symbol_table::iterator it = sym_table_.begin(); it < sym_table_.end(); ++it) {
					if (it->name == *i) { // Already exists
						throw declaration_conflict_exception(line_number_, *i);
					}
				}
				sym_table_.insert(sym_table_.end(), symbol(*i, 0, 0, symbol::external));
			}
		}

		else if (type_ == lang::global_dir) {
			if (words_.size() < 2) {
				throw wrong_word_number_exception(line_number_);
			}
			for (strings::iterator i = words_.begin() + 1; i < words_.end(); ++i) {
				if (!is_decl_label(*i)) {
					throw unexpected_content_exception(line_number_);
				}
				symbol_table::iterator it;
				for (it = sym_table_.begin(); it < sym_table_.end(); ++it) {
					if (it->name == *i) { // Already exists
						if (it->type != symbol::local) {
							throw declaration_conflict_exception(line_number_, *i);
						}
						it->type = symbol::global;
						break;
					}
				}
				if (it == sym_table_.end()) {
					sym_table_.insert(sym_table_.end(), symbol(*i, 0, 0, symbol::global));
				}
			}
		}
		
		else if (is_var_dir()) {
			if (sym_table_[current_section_].name == ".bss") {
				throw invalid_section_exception(line_number_);
			}
			int size = lang::var_size_map.find(type_)->second;
			location_counter_ += (words_.size() - 1) * size; // Skip the defined amount of variables
		}

		else if (type_ == lang::skip_dir) {
			if (words_.size() != 2) {
				throw wrong_word_number_exception(line_number_);
			}
			if (!is_integer(words_[1])) {
				throw unexpected_content_exception(line_number_);
			}
			location_counter_ += atoi((words_[1]).c_str());
		}

		else if (type_ == lang::align_dir) {
			if (words_.size() != 2) {
				throw wrong_word_number_exception(line_number_);
			}
			if (!is_integer(words_[1])) {
				throw unexpected_content_exception(line_number_);
			}
			long size = atoi((words_[1]).c_str());

			location_counter_ = ((location_counter_ >> size) + 1) << size;
		}

		else if (is_op()) {
			if (sym_table_[current_section_].name != ".text") {
				throw invalid_section_exception(line_number_);
			}
			location_counter_ += 2;

			if (exists_data_operand()) {
				location_counter_ += 2;
			}
		} // End of switch

		if (source_.eof()) {
			break;
		}
		else if (type_ == lang::label && words_.size() > 1) {
			get_from_line();
		}
		else {
			get_formatted_line();
		}
	}
	if (source_.eof() && !(type_ == lang::end_dir)) {
		throw end_not_found_exception();
	}
	check_symbol_definitions();
}
