#include "Assembler.h"
#include "Exceptions.h"

void assembler::second_pass() {
	current_section_ = -1;
	location_counter_ = start_address_;
	line_number_ = 0;

	source_.clear();
	source_.seekg(0, ios::beg);
	get_formatted_line();

	while (type_ != lang::end_dir) {
		if (is_section()) {
			for (symbol_table::iterator i = sym_table_.begin(); i < sym_table_.end(); ++i) {
				if (i->name == head_) {
					current_section_ = i - sym_table_.begin();
					break;
				}
			}
		}

		else if (is_var_dir()) {
			int size = lang::var_size_map.find(type_)->second;

			for (strings::iterator i = words_.begin() + 1; i < words_.end(); ++i) {
				long value = 0;

				if (is_integer(*i)) {
					value = atoi((*i).c_str());
					if (size < sizeof(long) && value > 1 << (size * 8)) {
						throw overflow_value_exception(line_number_);
					}
				}
				else if (is_decl_label(*i)) {
					symbol_table::iterator it;
					for (it = sym_table_.begin(); it < sym_table_.end(); ++it) {
						if (it->name == *i) {
							if (it->type == symbol::local) {
								value = it->value;
								rel_table_.insert(rel_table_.end(), relocation(location_counter_, relocation::absolute, size, it->section));
							}
							else {
								rel_table_.insert(rel_table_.end(), relocation(location_counter_, relocation::absolute, size, it - sym_table_.begin()));
							}
							break;
						}
					}
					if (it == sym_table_.end()) {
						throw undeclared_symbol_exception(line_number_, *i);
					}

				}
				else {
					throw unexpected_content_exception(line_number_);
				}

				byte* bytes = new byte[size];
				for (int dig = 0; dig < size; ++dig) {
					bytes[dig] = (byte)value;
					value = ((unsigned long)value) >> 8;
				}
				code_.write((char*)bytes, size);
				delete[] bytes;

				location_counter_ += size;
			}
		}

		else if (type_ == lang::skip_dir) {
			long size = atoi((words_[1]).c_str());
			
			byte* bytes = new byte[size];
			code_.write((char*)bytes, size);
			delete[] bytes;
			
			location_counter_ += size;
		}

		else if (type_ == lang::align_dir) {
			long size = atoi((words_[1]).c_str());
			size = ((location_counter_ >> size) + 1) << size;

			byte* bytes = new byte[size];
			code_.write((char*)bytes, size);
			delete[] bytes;

			location_counter_ += size;
		}

		else if (is_op()) {
			word opWord = lang::op_code_map.find(type_)->second | lang::cond_code_map.find(cond_type_)->second; // Generate binary file
			location_counter_ += 2;

			if (operand_count_ == 0) {
				if (type_ == lang::ret_op) {
					opWord |= (7 << lang::operand1_start_bit) | lang::operand_code_map.find(lang::reg)->second;
				}

				byte bytes[] = { (byte)(opWord >> 8), (byte)opWord };
				code_.write((char*)bytes, 2);
			}
			else if (operand_count_ == 1) {
				data_word dataWord;

				if (!is_memory_op() && is_memory_operand(0)) {
					//throw operation_without_memory_access(line_number_);
				}
				if (is_destination_op() && !is_legal_destination_operand(0)) {
					throw operation_with_illegal_destination_exception(line_number_);
				}

				handle_operand(opWord, dataWord, true);

				byte bytes[] = { (byte)(opWord >> 8), (byte)(opWord) };
				code_.write((char*)bytes, 2);
				if (is_data_operand(0)) {
					location_counter_ += 2;
					byte dataBytes[] = { (byte)((word)dataWord), (byte)(((word)dataWord) >> 8) };
					code_.write((char*)dataBytes, 2);
				}
			}
			else {
				data_word dataWord;

				if (!is_memory_op() && is_memory_operand(0)) {
					//throw operation_without_memory_access(line_number_);
				}
				if (!is_memory_op() && is_memory_operand(1)) {
					//throw operation_without_memory_access(line_number_);
				}
				if (is_destination_op() && !is_legal_destination_operand(0)) {
					throw operation_with_illegal_destination_exception(line_number_);
				}

				handle_operand(opWord, dataWord, true);
				handle_operand(opWord, dataWord, false);

				byte bytes[] = { (byte)(opWord >> 8), (byte)(opWord) };
				code_.write((char*)bytes, 2);
				if (exists_data_operand()) {
					location_counter_ += 2;
					byte dataBytes[] = { (byte)((word)dataWord), (byte)(((word)dataWord) >> 8) };
					code_.write((char*)dataBytes, 2);
				}
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
}