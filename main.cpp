#include "sat-solver.h"

int main(int argc, char *argv[]) {

	if (argc < 2) {
		cerr << "Error 1: You should put the name of the file\n";
		exit(1);
	}
	
	ifstream input;
	input.open(argv[1], ios::in);
	if (input.fail()) {
		cerr << "Error 2: Cannot open file\n";
		exit(2);
	}

	Solver task;

	task.read(input);

	task.solve();
	
	return 0;
}