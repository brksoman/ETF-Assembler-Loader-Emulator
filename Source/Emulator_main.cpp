#include "Emulator.h"
#include "Loader.h"

int main(int argc, char* argv[]) {
	strings outputNames;
	
	for (int i = 1; i < argc; ++i) {
		outputNames.insert(outputNames.end(), string(argv[i]));
	}
	
	try {
		loader l(outputNames);
		cout << ">>> Loading program..." << endl;
		l.load();
		
		cout << ">>> Loading complete." << endl << "    Emulating program..." << endl;
		emulator e(l.memory(), l.end_address());
		e.emulate();
		cout << ">>> Which memory do you want to print? (provide first address and length): ";
		int startAddress, length;
		byte* memory = e.memory();
		cin >> startAddress >> length;
		if (startAddress >= 0 && startAddress + length < lang::memory_size) {
			length += startAddress;
			for (int i = startAddress; i < length; ++i) {
				cout << (int)memory[i] << endl;
			}
		}
		
		
	}
	catch (exception& e) {
		cout << e.what() << endl;
		return -1;
	}
	return 0;
}
