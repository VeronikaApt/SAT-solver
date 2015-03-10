#include <fstream>
#include <iostream>

using namespace std;

class Solver {
private:
	int numb_of_var;
	int numb_of_fact;
	int **data;
	int *size_of_fact;

	struct Variable {
		int value;
		int init_order;
		bool define_twice;

		Variable();
	};

	Variable *var;

public:
	Solver();
	~Solver();

	void read(ifstream &input);

	//Run DPLL algorithm
	void solve();

	//Exclude disjuncts with true value
	void simplify();

	//Check whether all disjuncts are excluded
	bool isDone();

	//Force a disjunct with an only unspecified variable
	//to become true by assigning the proper value for that variable 
	bool propagate();

	//If a variable is met only under one polarity it is
	//assigned to be true under that polarity
	bool purefied();

	//Branch variable choise
	void chooseVar();

	//Look for a false disjunct
	bool isDisjunctEmpty();

	//Keeps the variable assignation order
	int defineOrder();

	//For a simplified disjunct recover its status
	//as a non-simplified
	void resimplify();

	bool allVariablesIsDefined();

	//Print the solution in case of successful solution
	void success();

	//Print the failure message
	void failure();
};

//Call the file format error
void formatError();

//Carriage return
void skip(ifstream &input);

//Return true, if the character is number or '-'
bool acceptable(char ch);