#ifndef __EMULATOR__
#define __EMULATOR__

#include <termios.h>
#include <sys/select.h>

#include <ctime>
#include <algorithm>
using namespace std;

#include "Language_specification.h"
#include "Symbol_table.h"
#include "Exceptions.h"

class emulator {
public:
	emulator() :
		pc((word&)registers_[lang::num_of_registers - 1]), sp((word&)registers_[lang::num_of_registers - 2]), memory_(nullptr) {
		if (constructed) {
			throw multiple_emulator_exception();
		}
		constructed = true;
	}
	emulator(byte * memory, word end_address);
	~emulator() { constructed = false; if (memory_) delete[] memory_; }
	void set(byte * memory, word end_address);

	void emulate();
	byte* memory() { return memory_; }

private:
	static bool constructed;
													// main methods
	void read_operation();
	void read_operand();
													// private fields
	byte * memory_;
	word start_address_;
	word end_address_;
													// registers
	data_word registers_[lang::num_of_registers];
	word & pc, & sp;
	word psw;
													// current operation info
	lang::line_type type_;
	lang::cond_type cond_type_;

	lang::operand_type operand_type_[2];
	int register_numbers_[2];
	data_word data_operand_;
	int operand_count_;
	bool we_are_leaving_;
													// clock
	time_t time_;
	word timerintr_location_;
	word kbintr_location_;
	word errorintr_location_;
													// methods
	void initialize();
	int operand_count() const;

	void push_word(word&);
	void pop_word(word&);
													// keyboard reading
	static bool is_init_;

	static bool emu_init_terminal();
	static struct termios prev_termios;
	static int emu_wait_kbhit();
	


												// PSW bits
	static const int z_bit = 0,
		o_bit = 1,
		c_bit = 2,
		n_bit = 3,
		timer_bit = 13,
		write_bit = 14,
		interrupt_bit = 15;

	void update_zn_bits(data_word operand);
	void set(int bit);
	void reset(int bit);
	int get(int bit) const;
};

inline void emulator::set(int bit) {
	psw |= 1 << bit;
}
inline void emulator::reset(int bit) {
	psw &= ~(1 << bit);
}
inline int emulator::get(int bit) const {
	return (psw >> bit) & 1;
}

inline void emulator::push_word(word& w) {
	memory_[--sp] = (byte)(w >> (sizeof(byte) * 8));
	memory_[--sp] = (byte)w;
}
inline void emulator::pop_word(word& w) {
	w = memory_[sp++];
	w += ((word)memory_[sp++]) << (sizeof(byte) * 8);
}

inline int emulator::operand_count() const {
	if (find(lang::operations_with_0operand.begin(), lang::operations_with_0operand.end(), type_) != lang::operations_with_0operand.end())
		return 0;
	if (find(lang::operations_with_1operand.begin(), lang::operations_with_1operand.end(), type_) != lang::operations_with_1operand.end())
		return 1;
	if (find(lang::operations_with_2operand.begin(), lang::operations_with_2operand.end(), type_) != lang::operations_with_2operand.end())
		return 2;
	return -1;
}

inline void emulator::update_zn_bits(data_word operand) {
	(operand == 0) ? set(z_bit) : reset(z_bit);
	(operand < 0) ? set(n_bit) : reset(n_bit);
}

#endif
