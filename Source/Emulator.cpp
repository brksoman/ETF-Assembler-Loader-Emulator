#include <iostream>
using namespace std;
#include "Emulator.h"

bool emulator::constructed = false;
bool emulator::is_init_ = emu_init_terminal();

emulator::emulator(byte * memory, word end_address) :
	pc((word&)registers_[lang::num_of_registers - 1]), sp((word&)registers_[lang::num_of_registers - 2]) {
	if (constructed) {
		throw multiple_emulator_exception();
	}
	constructed = true;
	set(memory, end_address);
}

void emulator::set(byte * memory, word end_address) {
	memory_ = memory;
	end_address_ = end_address;
}

void emulator::initialize() {
	for (map<word, string>::const_iterator i = lang::ivt.begin(); i != lang::ivt.end(); ++i) {
		if (i->second == "_start") {
			pc = memory_[2 * i->first];
			pc += ((word)memory_[2 * i->first + 1]) << (sizeof(byte) * 8);
		}
		if (i->second == "_timer") {
			timerintr_location_ = memory_[2 * i->first];
			timerintr_location_ += ((word)memory_[2 * i->first + 1]) << (sizeof(byte) * 8);
		}
		if (i->second == "_taster") {
			kbintr_location_ = memory_[2 * i->first];
			kbintr_location_ += ((word)memory_[2 * i->first + 1]) << (sizeof(byte) * 8);
		}
		if (i->second == "_error") {
			errorintr_location_ = memory_[2 * i->first];
			errorintr_location_ += ((word)memory_[2 * i->first + 1]) << (sizeof(byte) * 8);
		}
	}
	psw = 0;
	set(timer_bit);
	sp = lang::max_code_address - 1;
	time_ = time(0);
	we_are_leaving_ = false;
}


void emulator::read_operation() {
	word condition, operation, operandType[2];

	condition = (memory_[pc] << 8) & ((1 << 16) - (1 << lang::cond_start_bit));
	operation = (memory_[pc] << 8) & ((1 << lang::cond_start_bit) - (1 << lang::op_start_bit));
	operandType[0] = (memory_[pc] << 8) & ((1 << lang::op_start_bit) - (1 << (lang::operand1_start_bit + lang::addressing_type_offset)));
	++pc;

	register_numbers_[0] = (memory_[pc] >> lang::operand1_start_bit) & ((1 << lang::addressing_type_offset) - 1);
	operandType[1] = (memory_[pc] << lang::operand2_offset) & ((1 << lang::op_start_bit) - (1 << (lang::operand1_start_bit + lang::addressing_type_offset)));
	register_numbers_[1] = memory_[pc] & ((1 << lang::addressing_type_offset) - 1);
	++pc;

	for (map<lang::cond_type, word>::const_iterator i = lang::cond_code_map.begin(); i != lang::cond_code_map.end(); ++i) {
		if (i->second == condition) {
			cond_type_ = i->first;
			break;
		}
	}

	for (map<lang::line_type, word>::const_iterator i = lang::op_code_map.begin(); i != lang::op_code_map.end(); ++i) {
		if (i->second == operation) {
			type_ = i->first;
			break;
		}
	}
	
	for (int j = 0; j < 2; ++j) {
		for (map<lang::operand_type, word>::const_iterator i = lang::operand_code_map.begin(); i != lang::operand_code_map.end(); ++i) {
			if (i->second == operandType[j]) {
				operand_type_[j] = i->first;
				break;
			}
		}
	}
	operand_count_ = operand_count();
}


void emulator::read_operand() {
	data_operand_ = memory_[pc++];
	data_operand_ += memory_[pc++] << 8;
}


void emulator::emulate() {
	initialize();

	data_word * operands[2];

	int i = 0;
	while (pc != end_address_) {
		try {
			if (pc < lang::min_code_address || pc >= lang::max_code_address
				|| sp >= lang::max_code_address || pc >= sp) {
				throw runtime_exception();
			}
			//cout << endl << "Operation " << i++ << ": " << pc << "\t";
			read_operation();

			for (int i = 0; i < operand_count_; ++i) {
				switch (operand_type_[i]) {
				case lang::immed:
					if (register_numbers_[i] == 7) {
						operands[i] = (data_word*)&psw;
					}
					else {
						read_operand();
						operands[i] = &data_operand_;
					}
					
					//cout << "immed = " << *operands[i] << "\t";
					break;
				case lang::reg:
					operands[i] = &registers_[register_numbers_[i]];
					//cout << "reg" << register_numbers_[i] << " = " << *operands[i] << "\t";
					break;
				case lang::immed_memory:
					read_operand();
					operands[i] = (data_word*)&memory_[(word)data_operand_];
					//cout << "mem" << (word)data_operand_ << " = " << *operands[i] << "\t";
					break;
				case lang::reg_memory:
					read_operand();
					operands[i] = (data_word*)&memory_[registers_[register_numbers_[i]] + (word)data_operand_];
					//cout << "memreg" << register_numbers_[i] << ':' << (word)data_operand_ << " = " << *operands[i] << "\t";
				}
			}

			for (map<string, lang::line_type>::const_iterator i = lang::op_map.begin(); i != lang::op_map.end(); ++i) {
				if (i->second == type_) {
					//cout << i->first << "\t";
				}
			}

			if (cond_type_ == lang::al
				|| cond_type_ == lang::eq && get(z_bit) == 1
				|| cond_type_ == lang::ne && get(z_bit) == 0
				|| cond_type_ == lang::gt && get(n_bit) == 0 && get(z_bit) == 0) {
				//cout << "completed";

				data_word oldValue, result;
				switch (type_) {
				case lang::add_op:
					oldValue = *operands[0];
					*operands[0] += *operands[1];

					if (oldValue > *operands[0] && *operands[1] > 0 || oldValue < *operands[0] && *operands[1] < 0) {
						set(o_bit);
					}
					else {
						reset(o_bit);
					}
					if ((word)oldValue > (word)*operands[0]) {
						set(c_bit);
					}
					else {
						reset(c_bit);
					}
					update_zn_bits(*operands[0]);
					break;

				case lang::sub_op:
					oldValue = *operands[0];
					*operands[0] -= *operands[1];

					if (oldValue < *operands[0] && *operands[1] > 0 || oldValue > *operands[0] && *operands[1] < 0) {
						set(o_bit);
					}
					else {
						reset(o_bit);
					}
					if ((word)oldValue < (word)*operands[0]) {
						set(c_bit);
					}
					else {
						reset(c_bit);
					}
					update_zn_bits(*operands[0]);
					break;

				case lang::mul_op:
					*operands[0] *= *operands[1];
					update_zn_bits(*operands[0]);
					break;

				case lang::div_op:
					if (*operands[1] == 0) {
						throw runtime_exception();
					}
					*operands[0] /= *operands[1];
					update_zn_bits(*operands[0]);
					break;

				case lang::cmp_op:
					result = *operands[0] - *operands[1];
					if (*operands[0] < result && *operands[1] > 0 || *operands[0] > result && *operands[1] < 0) {
						set(o_bit);
					}
					else {
						reset(o_bit);
					}
					if ((word)*operands[0] < (word)result) {
						set(c_bit);
					}
					else {
						reset(c_bit);
					}
					update_zn_bits(result);
					break;

				case lang::and_op:
					*operands[0] &= *operands[1];
					update_zn_bits(*operands[0]);
					break;

				case lang::or_op:
					*operands[0] |= *operands[1];
					update_zn_bits(*operands[0]);
					break;

				case lang::not_op:
					*operands[0] = ~*operands[1];
					update_zn_bits(*operands[0]);
					break;

				case lang::test_op:
					result = *operands[0] & *operands[1];
					update_zn_bits(result);
					break;

				case lang::push_op:
					push_word((word&)*operands[0]);
					break;

				case lang::pop_op:
					pop_word((word&)*operands[0]);
					break;

				case lang::call_op:
					push_word(pc);
					pc = *operands[0];
					break;

				case lang::iret_op:
					pop_word(psw);
					pop_word(pc);
					reset(interrupt_bit);
					if (we_are_leaving_) {
						pc = end_address_;
					}
					break;

				case lang::mov_op:
					*operands[0] = *operands[1];
					if (operands[0] == (data_word*)(memory_ + lang::output_periphery)) {
						set(write_bit);
					}
					update_zn_bits(result);
					break;

				case lang::shl_op:
					if (*operands[0] >> *operands[1] != 0) {
						set(c_bit);
					}
					*operands[0] <<= *operands[1];
					update_zn_bits(*operands[0]);
					break;

				case lang::shr_op:
					if ((*operands[0] & ((1 << *operands[1]) - 1)) != 0) {
						set(c_bit);
					}
					*operands[0] >>= *operands[1];
					update_zn_bits(*operands[0]);
					break;
				}
			}
			//cout << endl;
		}
		catch (runtime_exception e) {
			if (errorintr_location_ == 0) {
				throw e;
			}
			push_word(pc);
			push_word(psw);
			pc = errorintr_location_;
			set(interrupt_bit);
			we_are_leaving_ = true;
		}
		
		if (!get(interrupt_bit) && get(timer_bit) && difftime(time(0), time_) >= 1) {
			++time_;
			if (timerintr_location_ == 0) {
				throw timer_not_defined_exception();
			}
			push_word(pc);
			push_word(psw);
			pc = timerintr_location_;
			set(interrupt_bit);
		}
		if (!get(interrupt_bit) && emu_wait_kbhit()) {
			memory_[lang::input_periphery] = getchar();
			if (kbintr_location_ != 0) {
				push_word(pc);
				push_word(psw);
				pc = kbintr_location_;
				set(interrupt_bit);
			}
		}
		
		if (get(write_bit)) {
			reset(write_bit);
			char c = memory_[lang::output_periphery];
			if (c == lang::new_line_code) {
				cout << endl;
			}
			else {
				cout << c;
			}
			cout.flush();
		}
	}
	
	cout << ">>> Emulation complete!" << endl << "    Registers:" << endl;

	for (int i = 0; i < lang::num_of_registers; ++i) {
		cout << "reg" << i << ": " << registers_[i] << endl;
	}
}

bool emulator::emu_init_terminal() {
	struct termios tmp;

	if (tcgetattr(fileno(stdin), &tmp) == -1) {
		printf("Failed to read terminal attributes.\n");
		exit(666);
	}

	//prev_termios = tmp;
	tmp.c_lflag &= ~ECHO;
	tmp.c_lflag &= ~ICANON;
	fflush(stdout);
	if (tcsetattr(fileno(stdin), TCSANOW, &tmp) == -1) {
		printf("Failed to set terminal attributes.\n");
		exit(666);
	}
	return true;
}

int emulator::emu_wait_kbhit() {
	int STDIN_FILENO_ = fileno(stdin);
	// timeout structure passed into select
	struct timeval tv;
	// fd_set passed into select
	fd_set fds;
	// Set up the timeout.
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	// Zero out the fd_set - make sure it's pristine
	FD_ZERO(&fds);
	// Set the FD that we want to read
	FD_SET(STDIN_FILENO_, &fds); //STDIN_FILENO is 0
	// select takes the last file descriptor value + 1 in the fdset to check,
	// the fdset for reads, writes, and errors.  We are only passing in reads.
	// the last parameter is the timeout.  select will return if an FD is ready or
	// the timeout has occurred
	select(STDIN_FILENO_+1, &fds, NULL, NULL, &tv);
	// return 0 if STDIN is not ready to be read.
	return FD_ISSET(STDIN_FILENO_, &fds);
}
