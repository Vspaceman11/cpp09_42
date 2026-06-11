#include "PmergeMe.hpp"
#include <sstream>
#include <iomanip>

// default constructor: initialize empty containers
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

// Parsing and strict validation of incoming arguments
bool PmergeMe::parseArguments(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg.empty())
			return false;

		// Only allow characters 0-9: no signs, no spaces, no letters
		for (char c : arg)
		{
			if (!std::isdigit(static_cast<unsigned char>(c)))
				return false;
		}

		try
		{
			// convert to integer and validate 32-bit signed positive range
			long long val = std::stoll(arg);
			if (val > 2147483647 || val < 0) // Protection against int overflow
				return false;

			// push same value to both containers so timings compare identical data
			_vector.push_back(static_cast<int>(val));
			_deque.push_back(static_cast<int>(val));
		}
		catch (...) // stoll can throw on overflow or invalid input
		{
			return false;
		}
	}
	// ensure at least one valid number was provided
	return _vector.empty() ? false : true;
}

// Generation of Jacobsthal sequence (J_k = J_{k-1} + 2 * J_{k-2})
std::vector<size_t> PmergeMe::generateJacobsthalSequence(size_t n) const
{
	// generate Jacobsthal numbers until we reach or exceed n
	// Jacobsthal: J0=0, J1=1, Jk = J_{k-1} + 2*J_{k-2}
	std::vector<size_t> seq;
	if (n <= 1)
		return seq; // no useful insertion order for 0 or 1 pend elements

	std::vector<size_t> jacob;
	jacob.push_back(0);
	jacob.push_back(1);

	// build sequence until we cover 'n'
	while (jacob.back() < n)
	{
		size_t next = jacob[jacob.size() - 1] + 2 * jacob[jacob.size() - 2];
		jacob.push_back(next);
	}

	// skip the first few trivial Jacobsthal numbers and return the rest
	// starting from index 3 matches the insertion grouping used in the algorithm
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

	std::cout << "After:  ";
	printContainer(_vector);

	std::cout << "Time to process a range of " << _vector.size()
			  << " elements with std::vector : " << durationVec.count() << " us" << std::endl;

	std::cout << "Time to process a range of " << _deque.size()
			  << " elements with std::deque  : " << durationDeq.count() << " us" << std::endl;
}
