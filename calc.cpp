#include "calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>

using std::map;
using std::vector;
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::stringstream;
using std::stoi;
using std::to_string;

struct Calc {
private:
  // fields
  map<string, int> variableCollection;
  bool valid = true;
  
public:

  //lock for concurrence
  pthread_mutex_t lock;
  
  // public member functions
  Calc();
  ~Calc();
  int evalExpr(const std::string &expr, int &result); 

private:
  // private member functions
  vector<string> tokenize(const string &expr);
  int parseOperand(string operand);
  int assignVariable(string variable, string value);
  int calculate(string operand1, string op, string operand2);                  
  bool checkVar(string input);
  bool checkInt(string input);              
  bool checkOperand(string input);       
  bool checkOp(string input);
  bool checkVarExistence(string varName);
  string singleCharToString(char character);
};

/*
 * Evalutes expression given by user input
 *
 * Parameters:
 *   expr - string with user input 
 *   result - pointer to an integer to store expression result
 *
 * Returns:
 *   1 if valid expression, 0 if not
 */
int Calc::evalExpr(const std::string &expr, int &result) {
  
  vector<string> input = tokenize(expr); //splits user input into tokoens
  int inputSize = input.size();
  valid = true; //initializes valid to true before any error checking
  
  if (inputSize > 5 || inputSize < 1 || inputSize == 2 || inputSize == 4) { //first check for valid input
    valid = false;
  }
  
  if (inputSize == 1 && checkOperand(input.at(0))) { //evaluate expression: operand
    result = parseOperand(input.at(0));
    return valid;
  }
      
  if (inputSize == 3) {
    if (singleCharToString('=').compare(input.at(1)) == 0 && checkVar(input.at(0)) && checkOperand(input.at(2))) { //evaluate expression: var = operand
      result = assignVariable(input.at(0), input.at(2));
      return valid;
    }
    if (checkOperand(input.at(0)) && checkOperand(input.at(2)) && checkOp(input.at(1))) { //evaluate expression: operand op operand
      result = calculate(input.at(0), input.at(1), input.at(2));
      return valid;
    }
  }
      
  if (inputSize == 5) { //evaluate expression: var = operand op operand
    if (singleCharToString('=').compare(input.at(1)) == 0 && checkVar(input.at(0)) && checkOperand(input.at(2)) && checkOp(input.at(3)) && checkOperand(input.at(4))) {
      result = assignVariable(input.at(0), to_string(calculate(input.at(2), input.at(3), input.at(4))));
      return valid;
    }
  }

  return false; //return false if none of the if statements execute
}

Calc::Calc(){
  //constructor must initialize mutex when data structure is initialized
  pthread_mutex_init(&lock, NULL);
}

Calc::~Calc() {
  //constructor must destroy mutex when data structure is deallocated
  pthread_mutex_destroy(&lock);
}

/*
 * Splits user input into tokens
 *
 * Parameters:
 *   expr - string with user input 
 *
 * Returns:
 *   vector with tokens
 */
vector<string> Calc::tokenize(const string &expr) {
  vector<string> vec;
  stringstream s(expr);

  string tok;
  while (s >> tok) {
    vec.push_back(tok);
  }

  return vec;
}

/*
 * Returns operand value
 *
 * Parameters:
 *   operand - string with inputted operand 
 *
 * Returns:
 *   integer with operand value
 */
int Calc::parseOperand(string operand) {
  if (checkVar(operand)) { //finds value if operand is a value
    if (checkVarExistence(operand)) {
      return variableCollection.find(operand)->second;
    }
    else { //flags expression as invalid
      valid = false;
      return 0;
    }
  }
  else { //returns operand as integer
    return stoi(operand); 
  }
}

/*
 * Adds variable to map
 *
 * Parameters:
 *   variable - string with variable name 
 *   value - string with variable value 
 *
 * Returns:
 *   integer with value
 */
int Calc::assignVariable(string variable, string value) {
  int intValue = parseOperand(value);
  variableCollection.erase(variable); //removes previous key if exists
  variableCollection.insert(std::pair<string,int>(variable,intValue));
  return intValue;
}

/*
 * Calculates arithmetic for expression
 *
 * Parameters:
 *   operand1 - string with first operand
 *   op - string with operator 
 *   operand2 - string with second operand 
 *
 * Returns:
 *   integer with calculated value
 */
int Calc::calculate(string operand1, string op, string operand2) {
  int value1 = parseOperand(operand1);
  int value2 = parseOperand(operand2);

  if (op == "+") {
    return value1 + value2;
  }
  if (op == "-") {
    return value1 - value2;
  }
  if (op == "*") {
    return value1 * value2;
  }
  if (op == "/") {
    if (value2 == 0) { //ensures no divide by 0
      valid = false;
      return 0;
    }
    return value1 / value2;
  }

  valid = false; //bad expression if no if statement entered, should never reach
  return 0;
}

/*
 * Checks if string is a valid variable
 *
 * Parameters:
 *   input - string to check
 *
 * Returns:
 *   true if valid, false if not
 */
bool Calc::checkVar(string input) {
  int i = 0;
  bool validVar = true;
  
  //checks if each character is an alpha
  while (input[i]) {
    if (!isalpha(input[i])) {
      validVar = false;
    }
    i++;
  }
  
  return validVar;
}

/*
 * Checks if string is a valid integer
 *
 * Parameters:
 *   input - string to check
 *
 * Returns:
 *   true if valid, false if not
 */
bool Calc::checkInt(string input) {
  bool validInt;
  //checks if first character is either a negative or a digit
  validInt = (isdigit(input[0]) || '-' == input[0]) ? true : false;
  int i = 1;
  
  //checks if each character is a digit
  while (input[i]) {
    if (!isdigit(input[i])) {
      validInt = false;
    }
    i++;
  }
  
  return validInt;
}

/*
 * Checks if string is a valid operand
 *
 * Parameters:
 *   input - string to check
 *
 * Returns:
 *   true if valid, false if not
 */
bool Calc::checkOperand(string input) {
  return checkVar(input) || checkInt(input);
}

/*
 * Checks if string is a valid operator
 *
 * Parameters:
 *   input - string to check
 *
 * Returns:
 *   true if valid, false if not
 */
bool Calc::checkOp(string input) {
  return singleCharToString('+').compare(input) == 0|| singleCharToString('-').compare(input) == 0|| singleCharToString('*').compare(input) == 0|| singleCharToString('/').compare(input) ==0;
}

/*
 * Checks if variable exists in dictionary
 *
 * Parameters:
 *   varName - variable name
 *
 * Returns:
 *   true if exists, false if not
 */
bool Calc::checkVarExistence(string varName) {
  if (variableCollection.count(varName) == 1) {
    return true;
  }
  
  return false;
}

/*
 * Converts single character to string
 *
 * Parameters:
 *   character - character to convert
 *
 * Returns:
 *   character as string
 */
string Calc::singleCharToString(char character) {
  string s(1, character);
  return s;
}

/*
 * Creates an instance of the struct Calc data type
 *
 * Returns:
 *   pointer to new Calculator struct
 */
extern "C" struct Calc *calc_create(void) {
    return new Calc();
}

/*
 * Deletes an instance of the struct Calc data type
 *
 * Parameters:
 *   calc - pointer to Calculator struct
 */
extern "C" void calc_destroy(struct Calc *calc) {
    delete calc;
}

/*
 * Evalutes expression given by user input
 *
 * Parameters:
 *   calc - pointer to Calculator struct
 *   expr - string with user input 
 *   result - pointer to an integer to store expression result
 * 
 * Returns:
 *   return value from evalExpr()
 */
extern "C" int calc_eval(struct Calc *calc, const char *expr, int *result) {

  //critical section, mutex used to lock data before starting to accept user input and evaluate
  pthread_mutex_lock(&calc->lock);
  int i = calc->evalExpr(expr, *result);
  //once evaluation is done, data is no longer critical, data unlocked
  pthread_mutex_unlock(&calc->lock);

  return i;
}
