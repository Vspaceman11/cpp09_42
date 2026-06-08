#include "RPN.hpp"
#include <cctype>

RPN::RPN() {}

RPN::RPN(const RPN& other) {
	*this = other;
}

RPN& RPN::operator=(const RPN& other) {
	if (this != &other) {
		this->_stack = other._stack;
	}
	return *this;
}

RPN::~RPN() {}


bool RPN::isOperator(char c) const
{
	return (c == '+' || c == '-' || c == '*' || c == '/');
}

// Pop two elements, perform the operation, and push the result back
bool RPN::performOperation(char op)
{
	// We need at least two operands to perform any operation
	if (_stack.size() < 2)
		return false;

	// First popped is the right operand (e.g., in "8 9 -", 9 is popped first)
	int second = _stack.back();
	_stack.pop_back();

	// Second popped is the left operand (e.g., 8)
	int first = _stack.back();
	_stack.pop_back();

	int result = 0;
	if (op == '+')
		result = first + second;
	else if (op == '-')
		result = first - second;
	else if (op == '*')
		result = first * second;
	else if (op == '/')
	{
		if (second == 0)
			return false;
		result = first / second;
	}

	_stack.push_back(result);
	return true;
}

// Main function to parse and calculate the RPN expression
void RPN::calculate(const std::string& expression)
{
	for (size_t i = 0; i < expression.length(); ++i)
	{
		char c = expression[i];

		// Skip spaces
		if (std::isspace(c))
			continue;
		// If character is a digit
		if (std::isdigit(c))
		{
			// From subj: numbers must be < 10 (single digits).
			// If the next char is also a digit, it's a multi-digit error.
			if (i + 1 < expression.length() && std::isdigit(expression[i + 1]))
			{
				std::cerr << "Error" << std::endl;
				return;
			}
			// Convert char to int and push to stack
			_stack.push_back(c - '0');
		}
		else if (isOperator(c))
		{
			if (!performOperation(c))
			{
				std::cerr << "Error" << std::endl;
				return;
			}
		}
		else
		{
			std::cerr << "Error" << std::endl;
			return;
		}
	}
	// After processing, exactly one final result must remain in the stack
	if (_stack.size() != 1)
	{
		std::cerr << "Error" << std::endl;
		return;
	}
	// Print result
	std::cout << _stack.back() << std::endl;
}
