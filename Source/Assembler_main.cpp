#include "Assembler.h"
#include <exception>
using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cout << ">>> Invalid numbers of arguments!" << endl;
		cout << "    Arguments should be: <source name> <output name> <starting location>" << endl;
		return -2;
	}
	string sourceName, outputName;
	int startingLocation;
	
	try {
		sourceName = string(argv[1]);
		outputName = string(argv[2]);
		startingLocation = atoi(argv[3]);
	}
	catch (exception& e) {
		cout << ">>> Invalid arguments!" << endl;
		cout << "    Arguments should be: <source name> <output name> <starting location>" << endl;
		return -3;	
	}
	
	try {
		assembler a;
		a.set(sourceName, outputName, startingLocation);
		a.assemble();
		cout << endl;
	}
	catch (exception& e) {
		cout << endl << e.what() << endl;
		return -1;
	}
	return 0;
}
