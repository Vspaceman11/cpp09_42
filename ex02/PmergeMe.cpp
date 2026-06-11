#include "PmergeMe.hpp"
#include <sstream>
#include <iomanip>

PmergeMe::PmergeMe() {}

PmergeMe::PmergeMe(const PmergeMe& other)
{
	*this = other;
}

PmergeMe& PmergeMe::operator=(const PmergeMe& other)
{
	if (this != &other)
	{
		this->_vector = other._vector;
		this->_deque = other._deque;
	}
	return *this;
}

PmergeMe::~PmergeMe() {}

// Parsing and strict validation of incoming numbers
bool PmergeMe::parseArguments(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg.empty())
			return false;

		// Check that all characters of the argument are digits (positive numbers only)
		for (size_t j = 0; j < arg.length(); ++j)
		{
			if (!std::isdigit(arg[j]))
				return false;
		}

		try
		{
			long long val = std::stoll(arg);
			if (val > 2147483647 || val < 0) // Protection against int overflow
				return false;

			_vector.push_back(static_cast<int>(val));
			_deque.push_back(static_cast<int>(val));
		}
		catch (...)
		{
			return false;
		}
	}
	return _vector.empty() ? false : true;
}

// Index generation based on the Jacobsthal sequence (J_k = J_{k-1} + 2 * J_{k-2})
std::vector<size_t> PmergeMe::generateJacobsthalSequence(size_t n) const
{
	std::vector<size_t> seq;
	if (n <= 1)
		return seq;

	std::vector<size_t> jacob;
	jacob.push_back(0);
	jacob.push_back(1);

	// Generate base Jacobsthal numbers until they exceed the size of the pend elements array
	while (jacob.back() < n)
	{
		size_t next = jacob[jacob.size() - 1] + 2 * jacob[jacob.size() - 2];
		jacob.push_back(next);
	}

	// Form the sequence to manage insertion groups (skip 0 and 1)
	for (size_t i = 3; i < jacob.size(); ++i)
	{
		seq.push_back(jacob[i]);
	}
	return seq;
}

void PmergeMe::run(int argc, char* argv[])
{
	if (!parseArguments(argc, argv))
	{
		std::cerr << "Error" << std::endl;
		return;
	}

	// Print the "Before" state
	std::cout << "Before: ";
	printContainer(_vector);

	// --- Time measurement for std::vector ---
	auto startTimeVec = std::chrono::high_resolution_clock::now();
	fordJohnsonSort(_vector);
	auto endTimeVec = std::chrono::high_resolution_clock::now();
	auto durationVec = std::chrono::duration_cast<std::chrono::microseconds>(endTimeVec - startTimeVec);

	// --- Time measurement for std::deque ---
	auto startTimeDeq = std::chrono::high_resolution_clock::now();
	fordJohnsonSort(_deque);
	auto endTimeDeq = std::chrono::high_resolution_clock::now();
	auto durationDeq = std::chrono::duration_cast<std::chrono::microseconds>(endTimeDeq - startTimeDeq);

	// Print the "After" state (after sorting)
	std::cout << "After:  ";
	printContainer(_vector);

	// Print execution time statistics
	std::cout << "Time to process a range of " << _vector.size()
			  << " elements with std::vector : " << durationVec.count() << " us" << std::endl;

	std::cout << "Time to process a range of " << _deque.size()
			  << " elements with std::deque  : " << durationDeq.count() << " us" << std::endl;
}
