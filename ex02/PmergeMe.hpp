#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <chrono>
#include <algorithm>

class PmergeMe {
private:
	std::vector<int>	_vector;
	std::deque<int>		_deque;

	bool parseArguments(int argc, char* argv[]);
	std::vector<size_t> generateJacobsthalSequence(size_t n) const;

	// Separate implementations for vector and deque
	void fordJohnsonSortVector(std::vector<int>& vec);
	void fordJohnsonSortDeque(std::deque<int>& deq);

	void printVectorContainer(const std::vector<int>& c) const;
	void printDequeContainer(const std::deque<int>& c) const;

public:
	PmergeMe();
	PmergeMe(const PmergeMe& other);
	PmergeMe& operator=(const PmergeMe& other);
	~PmergeMe();

	void run(int argc, char* argv[]);
};
