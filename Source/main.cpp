#include "Assembler.h"
#include "Loader.h"
#include "Emulator.h"

int main() {
	assembler a;

	while (true) {
		strings outputNames;
		cout << "Unesite podatke fajlova za asembliranje:" << endl;
		string line;
		string sourceName, outputName;
		long startingAddress;

		try {
			for (getline(cin, line); line != ""; getline(cin, line)) {
				if (line == "exit") {
					break;
				}
				stringstream ss(line);

				ss >> sourceName >> outputName >> startingAddress;
				a.set(sourceName, outputName, startingAddress);
				a.assemble();

				outputNames.insert(outputNames.end(), outputName);
			}

			loader l(outputNames);
			l.load();

			emulator e(l.memory(), l.end_address());
			e.emulate();
		}
		catch (exception& e) {
			cout << endl << e.what() << endl;
		}
	}

	system("pause");
}