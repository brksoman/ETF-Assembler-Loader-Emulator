#ifndef __LANGUAGE_SPECIFICATIONS__
#define __LANGUAGE_SPECIFICATIONS__

#include <string>
#include <map>
#include <vector>
#include <initializer_list>
using namespace std;

typedef vector<string> strings;
typedef unsigned short word;
typedef short data_word;
typedef unsigned char byte;
typedef char data_byte;
/*using strings = vector<string>;
using word = unsigned short;
using data_word = short;
using byte = unsigned char;
using data_byte = char;*/

namespace lang {
	const int addressing_bits = 16;
	const long memory_size = 1 << addressing_bits;
	
	const int ivt_memory_size = 2 * 8;
	const long min_code_address = ivt_memory_size;

	const int periphery_memory_size = 128;
	const long max_code_address = memory_size - periphery_memory_size;
	
	const long input_periphery = 0xfffc;
	const long output_periphery = 0xfffe;
	const long new_line_code = 0x10;

	const int byte_roof = 256;

	const int cond_start_bit = 16 - 2;
	const int op_start_bit = cond_start_bit - 4;
	const int operand1_start_bit = op_start_bit - 5;
	const int operand2_offset = 5;
	const int addressing_type_offset = 3;
	const int num_of_registers = 8;

	enum line_type {
		label, text_section, data_section, rodata_section, bss_section,
		extern_dir, global_dir, char_dir, word_dir, long_dir, align_dir,
		skip_dir, end_dir,

		add_op, sub_op, mul_op, div_op, cmp_op, and_op, or_op, not_op,
		test_op, push_op, pop_op, call_op, iret_op, mov_op, shl_op,
		shr_op, ret_op, jmp_op
	};
	enum cond_type {
		eq, ne, gt, al
	};
	enum operand_type {
		immed, reg, immed_memory, reg_memory, psw
	};
	const map<string, line_type> dir_map = {
		{ ".text", text_section },
		{ ".data", data_section },
		{ ".rodata", rodata_section },
		{ ".bss", bss_section },
		{ ".extern", extern_dir },
		{ ".global", global_dir },
		{ ".end", end_dir },
		{ ".char", char_dir },
		{ ".word", word_dir },
		{ ".long", long_dir },
		{ ".align", align_dir },
		{ ".skip", skip_dir }
	};
	const map<string, line_type> op_map = {
		{ "add", add_op },
		{ "sub", sub_op },
		{ "mul", mul_op },
		{ "div", div_op },
		{ "cmp", cmp_op },
		{ "and", and_op },
		{ "or", or_op },
		{ "not", not_op },
		{ "test", test_op },
		{ "push", push_op },
		{ "pop", pop_op },
		{ "call", call_op },
		{ "iret", iret_op },
		{ "mov", mov_op },
		{ "shl", shl_op },
		{ "shr", shr_op },
		{ "ret", ret_op },
		{ "jmp", jmp_op }
	};
	const map<string, cond_type> cond_map = {
		{ "eq", eq },
		{ "ne", ne },
		{ "gt", gt },
		{ "al", al }
	};

	const vector<line_type> sections = {
		text_section,
		data_section,
		rodata_section,
		bss_section
	};
	const vector<line_type> vars = {
		char_dir,
		word_dir,
		long_dir
	};
	const vector<line_type> operations = {
		add_op,
		sub_op,
		mul_op,
		div_op,
		cmp_op,
		and_op,
		or_op,
		not_op,
		test_op,
		push_op,
		pop_op,
		call_op,
		iret_op,
		mov_op,
		shl_op,
		shr_op,
		ret_op,
		jmp_op
	};

	const vector<line_type> operations_with_0operand = {
		iret_op,
		ret_op
	};
	const vector<line_type> operations_with_1operand = {
		push_op,
		pop_op,
		call_op,
		jmp_op
	};
	const vector<line_type> operations_with_2operand = {
		add_op,
		sub_op,
		mul_op,
		div_op,
		cmp_op,
		and_op,
		or_op,
		not_op,
		test_op,
		mov_op,
		shl_op,
		shr_op
	};
	const vector<operand_type> data_operands = {
		immed,
		immed_memory,
		reg_memory
	};
	const vector<operand_type> memory_operands = {
		immed_memory,
		reg_memory
	};
	const vector<operand_type> legal_destination_operands = {
		immed_memory,
		reg,
		reg_memory
	};

	const vector<line_type> pcrelative_operations = {
	};
	const vector<line_type> operations_with_destination_operand = {
		add_op,
		sub_op,
		mul_op,
		div_op,
		and_op,
		or_op,
		not_op,
		pop_op,
		mov_op,
		shl_op,
		shr_op
	};
	const vector<line_type> operations_with_memory = {
		mov_op,
		jmp_op
	};

	const map<line_type, int> var_size_map = {
		{ char_dir, 1 },
		{ word_dir, 2 },
		{ long_dir, 4 }
	};

	const map<line_type, word> op_code_map = {
		{ add_op, 0 << op_start_bit },
		{ sub_op, 1 << op_start_bit },
		{ mul_op, 2 << op_start_bit },
		{ div_op, 3 << op_start_bit },
		{ cmp_op, 4 << op_start_bit },
		{ and_op, 5 << op_start_bit },
		{ or_op, 6 << op_start_bit },
		{ not_op, 7 << op_start_bit },
		{ test_op, 8 << op_start_bit },
		{ push_op, 9 << op_start_bit },
		{ pop_op, 10 << op_start_bit },
		{ call_op, 11 << op_start_bit },
		{ iret_op, 12 << op_start_bit },
		{ mov_op, 13 << op_start_bit },
		{ shl_op, 14 << op_start_bit },
		{ shr_op, 15 << op_start_bit },

		{ ret_op, 10 << op_start_bit },
		{ jmp_op, 13 << op_start_bit }
	};
	const map<cond_type, word> cond_code_map = {
		{ eq, 0 << cond_start_bit },
		{ ne, 1 << cond_start_bit },
		{ gt, 2 << cond_start_bit },
		{ al, 3 << cond_start_bit }
	};
	const map<operand_type, word> operand_code_map = {
		{ immed, 0 << (operand1_start_bit + addressing_type_offset) },
		{ reg, 1 << (operand1_start_bit + addressing_type_offset) },
		{ immed_memory, 2 << (operand1_start_bit + addressing_type_offset) },
		{ reg_memory, 3 << (operand1_start_bit + addressing_type_offset) },
		{ psw, 0 << (operand1_start_bit + addressing_type_offset) }
	};

	const map<word, string> ivt = {
		{ 0, "_start" },
		{ 1, "_timer" },
		{ 2, "_error" },
		{ 3, "_taster" },
		{ 4, "_userintr1" },
		{ 5, "_userintr2" },
		{ 6, "_userintr3" },
		{ 7, "_userintr4" }
	};
}

#endif
