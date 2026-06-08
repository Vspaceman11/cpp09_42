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

bool RPN::performOperation(char op)
{
	if (_stack.size() < 2)
		return false;

	int second = _stack.back();
	_stack.pop_back();

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

void RPN::calculate(const std::string& expression)
{
	for (size_t i = 0; i < expression.length(); ++i)
	{
		char c = expression[i];

		if (std::isspace(c))
			continue;

		if (std::isdigit(c))
		{
			if (i + 1 < expression.length() && std::isdigit(expression[i + 1]))
			{
				std::cerr << "Error" << std::endl;
				return;
			}
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
	if (_stack.size() != 1)
	{
		std::cerr << "Error" << std::endl;
		return;
	}
	std::cout << _stack.back() << std::endl;
}
