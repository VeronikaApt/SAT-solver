#include "sat-solver.h"


//Carriage return
void skip(ifstream &input) {
	char ch;
	
	do {
		ch = input.get();
	} while ((ch != '\n') && (ch != EOF));
}


//Call the file format error
void formatError() {
	cerr << "Error 3: Incorrect file format.\nPlease use file in the DIMACS format\n";
	exit(3);
}


//Return true, if the character is number or '-'
bool acceptable(char ch) {
	if (((ch >= '0') && (ch <= '9'))
		|| (ch == '-'))
		return true;
	return false;
}


Solver::Variable::Variable() {
	value = -1;
	init_order = 0;
	define_twice = false;
}


//Keeps the variable assignation order
int Solver::defineOrder() {
	int count = 0;
	for (int i = 0; i < numb_of_var; i++)
		if (var[i].init_order != 0)
			count++;

	return count + 1;
}


Solver::Solver() {
	data = NULL;
	size_of_fact = NULL;
	var = NULL;

	numb_of_fact = 0;
	numb_of_var = 0;
}


Solver::~Solver() {
	for (int i = 0; i < numb_of_fact; i++)
		delete data[i];
	delete[] data;

	delete[] var;

	delete[] size_of_fact;
}


void Solver::read(ifstream &input) {
	char ch;
	bool define_size = false;
	int line = -1;
	char key[] = "p cnf ";
	int size_of_key = 6;
	
	do {
		ch = input.peek();

		//Processing string with comment
		if (ch == 'c') {
			skip(input);	
		}

		//Processing string containing the matrix sizes
		if (ch == key[0]) {
			for (int i = 0; i < size_of_key; i++) {
				ch = input.get();
				if (ch != key[i])
					formatError();
			}

			input >> numb_of_var;
			if (input.fail() || (numb_of_var <= 0))
				formatError();

			input >> numb_of_fact;
			if (input.fail() || (numb_of_fact <= 0))
				formatError();

			data = new int*[numb_of_fact];
			for (int i = 0; i < numb_of_fact; i++)
				data[i] = new int[numb_of_var + 1];  //+1 for null

			var = new Variable[numb_of_var];

			size_of_fact = new int[numb_of_fact];

			define_size = true;

			input.get();

			continue;
		}
			
		//Processing string with the numbers
		if (acceptable(ch)) {
			if (!define_size)
				formatError();

			line += 1;
			if (line >= numb_of_fact)
				formatError();
			
			int column = 0;

			bool end_line = false; 
			int number;
			do { 
				input >> number;
				if ((!input) || (abs(number) > numb_of_var))
					formatError();

				data[line][column++] = number;

				if (data[line][column - 1] == 0) {
					end_line = true;
					skip(input);
				}

			} while (!end_line);

			size_of_fact[line] = column;

		}

		if (ch == '\n')
			input.get();

		if (input.eof() && (line < numb_of_fact - 1))
			formatError();

		//Default
		if ((ch != 'c') && (ch != key[0]) && !acceptable(ch)
			&& !input.eof())
			formatError();

	} while (!input.eof());
}


//Check whether all disjuncts are excluded
void Solver::simplify() {
	for (int i = 0; i < numb_of_fact; i++) {
		int j = 0;
		while (data[i][j] != 0) {
			if (data[i][j] > 0) {
				int ind = data[i][j] - 1;
				if (var[ind].value == 1) {
					int end_line = size_of_fact[i] - 1;

					data[i][end_line] = data[i][0];
					data[i][0] = 0;

					break;
				}
			} else {
				int ind = abs(data[i][j]) - 1;
				if (var[ind].value == 0) {
					int end_line = size_of_fact[i] - 1;

					data[i][end_line] = data[i][0];
					data[i][0] = 0;

					break;
				}
			}

			j++;
		}
	}
}


//For a simplified disjunct recover its status
	//as a non-simplified
void Solver::resimplify() {
	for (int i = 0; i < numb_of_fact; i++)
		if (data[i][0] == 0) {
			int end_line = size_of_fact[i] - 1;

			data[i][0] = data[i][end_line];
			data[i][end_line] = 0;
		}
		
}


//Check whether all disjuncts are excluded
bool Solver::isDone() {
	for (int i = 0; i < numb_of_fact; i++)
		if (data[i][0] != 0)
			return false;

	return true;
}


bool Solver::allVariablesIsDefined() {
	for (int i = 0; i < numb_of_var; i++)
		if (var[i].value == -1)
			return false;

	return true;
}


//Force a disjunct with an only unspecified variable
	//to become true by assigning the proper value for that variable 
bool Solver::propagate() {
	bool flag = false;

	for (int i = 0; i < numb_of_fact; i++) {
		int j = 0;
		int idx, is_not_defined = 0;
		while (data[i][j] != 0) {
			int ind = abs(data[i][j]) - 1;
			if (var[ind].value == -1) {
				is_not_defined += 1;
				idx = j;
			}

			if (is_not_defined > 1)
				break;

			j++;
		}

		if (is_not_defined == 1) {
			int ind = abs(data[i][idx]) - 1;

			if ((data[i][idx] > 0) && (var[ind].value == -1)) {
				var[ind].value = 1;
				var[ind].init_order = defineOrder();
				flag = true;
			} 

			if ((data[i][idx] < 0) && (var[ind].value == -1)) {
				var[ind].value = 0;
				var[ind].init_order = defineOrder();
				flag = true;
			}
		}
	}
	
	return flag;
}


//If a variable is met only under one polarity it is
	//assigned to be true under that polarity
bool Solver::purefied() {
	//Plus-signed variables enterance
	int *plusEnt = new int[numb_of_var] ();

	//Minus-signed variables enterance
	int *minusEnt = new int[numb_of_var] ();

	for (int i = 0; i < numb_of_fact; i++) {
		int j = 0;
		while (data[i][j] != 0) {
			int ind = abs(data[i][j]) - 1;
			if (data[i][j] > 0)
				plusEnt[ind] += 1;
			else
				minusEnt[ind] += 1;
			j++;
		}
	}
	
	bool flag = false;

	//By the way, if a variable is irrelevant for the disjunct,
		//the following piece of code will assign a value for it
	for (int i = 0; i < numb_of_var; i++) {
		if ((plusEnt[i] == 0) && (var[i].value == -1)) {
			var[i].value = 0;
			var[i].init_order = defineOrder();
			flag = true;
		}
		
		if ((minusEnt[i] == 0) && (var[i].value == -1)) {
			var[i].value = 1;
			var[i].init_order = defineOrder();
			flag = true;
		}
	}

	delete[] plusEnt;
	delete[] minusEnt;

	return flag;
}


//Branch variable choise
void Solver::chooseVar() {
	if (!allVariablesIsDefined()) {
		int *count = new int[numb_of_var] ();

		for (int i = 0; i < numb_of_fact; i++) {
			int j = 0;
			while (data[i][j] != 0) {
				int ind = abs(data[i][j]) - 1;

				if ((data[i][j] > 0) && (var[ind].value == -1)) {			
					count[ind] += 1;
				}

				j++;
			}
		}

		int max, idx;

		//First non-defined variable
		for (int i = 0; i < numb_of_var; i++)
			if (var[i].value == -1) {
				max = count[i];
				idx = i;
				break;
			}
		

		for (int i = 1; i < numb_of_var; i++)
			if (count[i] > max) {
				max = count[i];
				idx = i;
			}

		var[idx].value = 1;
		var[idx].init_order = defineOrder();

		delete[] count;
	}
}


//Look for a false disjunct
bool Solver::isDisjunctEmpty() {
	for (int i = 0; i < numb_of_fact; i++) {
		int j = 0;
		while (data[i][j] != 0) {
			int ind = abs(data[i][j]) - 1;

			if (var[ind].value == -1)
				return false;

			j++;
		}

		j = 0;
		while (data[i][j] != 0) {
			int ind = abs(data[i][j]) - 1;

			if ((data[i][j] < 0) && (var[ind].value == 0)
				|| (data[i][j] > 0) && (var[ind].value == 1))
				return false;

			j++;
		}
	}

	return true;
}


//Print the solution in case of successful solution
void Solver::success() {
	cout << "s SATISFIABLE\nv ";
	for (int i = 0; i < numb_of_var; i++) {
		if (var[i].value == 1)
			cout << i + 1 << " ";

		if (var[i].value == 0)
			cout << -(i + 1) << " ";
	}
	cout << "0\n";
}


//Print the failure message
void Solver::failure() {
	cout << "s UNSATISFIABLE\n";
}


//Run DPLL algorithm
void Solver::solve() {
	bool exit = false;
	while (!exit) {
		bool flag;
		do {
			flag = false;

			if (purefied()) {
				flag = true;
				simplify();
			}

			if (propagate()) {
				flag = true;
				simplify();
			}

		} while (flag);

		chooseVar();
		simplify();

		if (isDone()) {
			success();
			exit = true;
		}

		if (isDisjunctEmpty() && !exit) {
			//Is the first-initialized variable already defined twice?
			for (int i = 0; i < numb_of_var; i++)
				if ((var[i].init_order == 1) && var[i].define_twice) {
					failure();
					exit = true;
					break;
				}

			if (!exit) {
				//Let's find the last-initialized variable
				int *order = new int[numb_of_var];

				for (int i = 0; i < numb_of_var; i++)
					order[var[i].init_order - 1] = i;

				int idx = order[numb_of_var - 1];

				//If the last-initialized variable already define twice, 
					//find the last initialized variable which wasn't
				if (var[idx].define_twice) {	
					for (int i = numb_of_var - 1; i >= 0; i--) {
						int ind = order[i];

						if (!var[ind].define_twice) {
							idx = i;
							break;
						}

						if (var[ind].define_twice) {
							var[ind].define_twice = false;
							var[ind].init_order = 0;
							var[ind].value = -1;
						}
					}

					if (var[idx].value == 0)
						var[idx].value = 1;
					else
						var[idx].value = 0;

					var[idx].define_twice = true;

					resimplify();
					simplify();
				}

				//Give a reverse value to the last-initialized variable,
					//which wasn't define twice
				if (!var[idx].define_twice) {
					if (var[idx].value == 0)
						var[idx].value = 1;
					else
						var[idx].value = 0;

					var[idx].define_twice = true;

					resimplify();
					simplify();
				}

				delete[] order;
			}
		}
	}
}