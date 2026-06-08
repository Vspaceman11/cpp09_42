#pragma once
#include <iostream>
#include <string>
#include <list>

class RPN
{
private:
	std::list<int> _stack;

	bool isOperator(char c) const;
	bool performOperation(char op);
public:
	RPN();
	RPN(const RPN& other);
	RPN& operator=(const RPN& other);
	~RPN();

	void calculate(const std::string& expression);
};

