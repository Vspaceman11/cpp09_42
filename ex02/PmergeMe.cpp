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

// Generation of Jacobsthal sequence for Ford-Johnson algorithm
// Returns the insertion boundaries for the classical merge-insertion sort
std::vector<size_t> PmergeMe::generateJacobsthalSequence(size_t n) const
{
	std::vector<size_t> seq;
	if (n == 0)
		return seq;

	// Generate Jacobsthal sequence: J(0)=0, J(1)=1, J(k) = J(k-1) + 2*J(k-2)
	std::vector<size_t> jacob;
	jacob.push_back(0);
	jacob.push_back(1);

	while (jacob.back() < n)
	{
		size_t next = jacob[jacob.size() - 1] + 2 * jacob[jacob.size() - 2];
		jacob.push_back(next);
	}

	// Build sequence of insertion boundaries according to classical Ford-Johnson
	// We skip J(0)=0 and use pairs: (J(2k), J(2k-1)) in that order
	for (size_t i = 2; i < jacob.size(); i += 2)
	{
		// For each pair of consecutive Jacobsthal numbers, add both if they fit
		if (jacob[i] <= n)
			seq.push_back(jacob[i]);
		if (i + 1 < jacob.size() && jacob[i + 1] <= n)
			seq.push_back(jacob[i + 1]);
	}

	// Ensure we cover all elements
	if (seq.empty() || seq.back() < n)
		seq.push_back(n);

	return seq;
}

void PmergeMe::printVectorContainer(const std::vector<int>& c) const
{
	for (const auto& element : c)
	{
		std::cout << element << " ";
	}
	std::cout << std::endl;
}

void PmergeMe::printDequeContainer(const std::deque<int>& c) const
{
	for (const auto& element : c)
	{
		std::cout << element << " ";
	}
	std::cout << std::endl;
}

// Vector-specific implementation of Ford-Johnson sort
void PmergeMe::fordJohnsonSortVector(std::vector<int>& vec)
{
	if (vec.size() < 2)
		return;

	// Step 1: Handle odd element (straggler) if list has odd number of elements
	bool hasStraggler = (vec.size() % 2 != 0);
	int straggler = 0;
	if (hasStraggler)
	{
		straggler = vec.back();
		vec.pop_back();
	}

	// Step 2: Create pairs by comparing adjacent elements
	// Store as (high, low) - in original order, NOT sorted
	std::vector<std::pair<int, int>> pairs;
	pairs.reserve(vec.size() / 2);

	for (size_t i = 0; i < vec.size(); i += 2)
	{
		if (vec[i] < vec[i + 1])
			pairs.push_back({vec[i + 1], vec[i]});  // high, low
		else
			pairs.push_back({vec[i], vec[i + 1]});  // high, low
	}

	// Step 3: Extract mainChain from pairs in ORIGINAL unsorted order
	std::vector<int> mainChain;
	mainChain.reserve(pairs.size());
	for (const auto& p : pairs)
		mainChain.push_back(p.first);  // Extract high elements only

	// Step 4: Recursively sort mainChain (this is where actual sorting happens)
	fordJohnsonSortVector(mainChain);

	// Step 5: Build pendElements matching the sorted order of mainChain
	// For each high element in sorted mainChain, find its corresponding low element
	// Also store the corresponding high element to optimize insertion range in Step 6
	std::vector<int> pendElements;
	std::vector<int> pairedHighs;  // Track high element for each pendElement
	pendElements.reserve(pairs.size());
	pairedHighs.reserve(pairs.size());

	std::vector<bool> used(pairs.size(), false);  // Track which pairs are used

	for (int high : mainChain)
	{
		for (size_t i = 0; i < pairs.size(); ++i)
		{
			// Find matching pair and extract its low element
			if (!used[i] && pairs[i].first == high)
			{
				pendElements.push_back(pairs[i].second);
				pairedHighs.push_back(high);  // Store corresponding high
				used[i] = true;  // Mark as used
				break;
			}
		}
	}

	// Step 6: Insert pendElements using binary search with Jacobsthal sequence
	// Limit search range to the position of each element's paired high
	if (!pendElements.empty())
		mainChain.insert(mainChain.begin(), pendElements[0]);

	// Generate Jacobsthal sequence for insertion batches
	std::vector<size_t> jacobsthal = generateJacobsthalSequence(pendElements.size());

	size_t prevBound = 1;  // Already inserted pendElements[0]

	for (size_t bound : jacobsthal)
	{
		if (bound > pendElements.size())
			bound = pendElements.size();

		// Insert elements from prevBound to bound (right to left, in reverse order)
		for (int i = static_cast<int>(bound) - 1; i >= static_cast<int>(prevBound); --i)
		{
			int elementToInsert = pendElements[i];
			int pairedHigh = pairedHighs[i];  // Get the corresponding high element

			// Find position of the paired high element in mainChain
			auto limitIt = std::lower_bound(mainChain.begin(), mainChain.end(), pairedHigh);

			// Binary search for insertion point, limited to position of paired high
			// Since low <= high, search ends at limitIt
			auto insertionPoint = std::upper_bound(mainChain.begin(), limitIt, elementToInsert);
			mainChain.insert(insertionPoint, elementToInsert);
		}

		prevBound = bound;

		if (bound == pendElements.size())
			break;
	}

	// Step 7: Insert the straggler (odd element) if it exists
	if (hasStraggler)
	{
		auto insertionPoint = std::upper_bound(mainChain.begin(), mainChain.end(), straggler);
		mainChain.insert(insertionPoint, straggler);
	}

	vec.swap(mainChain);
}

// Deque-specific implementation of Ford-Johnson sort
void PmergeMe::fordJohnsonSortDeque(std::deque<int>& deq)
{
	if (deq.size() < 2)
		return;

	// Step 1: Handle odd element (straggler) if list has odd number of elements
	bool hasStraggler = (deq.size() % 2 != 0);
	int straggler = 0;
	if (hasStraggler)
	{
		straggler = deq.back();
		deq.pop_back();
	}

	// Step 2: Create pairs by comparing adjacent elements
	// Store as (high, low) - in original order, NOT sorted
	std::deque<std::pair<int, int>> pairs;

	for (size_t i = 0; i < deq.size(); i += 2)
	{
		if (deq[i] < deq[i + 1])
			pairs.push_back({deq[i + 1], deq[i]});  // high, low
		else
			pairs.push_back({deq[i], deq[i + 1]});  // high, low
	}

	// Step 3: Extract mainChain from pairs in ORIGINAL unsorted order
	std::deque<int> mainChain;
	for (const auto& p : pairs)
		mainChain.push_back(p.first);  // Extract high elements only

	// Step 4: Recursively sort mainChain (this is where actual sorting happens)
	fordJohnsonSortDeque(mainChain);

	// Step 5: Build pendElements matching the sorted order of mainChain
	// For each high element in sorted mainChain, find its corresponding low element
	// Also store the corresponding high element to optimize insertion range in Step 6
	std::deque<int> pendElements;
	std::deque<int> pairedHighs;  // Track high element for each pendElement

	std::vector<bool> used(pairs.size(), false);  // Track which pairs are used

	for (int high : mainChain)
	{
		for (size_t i = 0; i < pairs.size(); ++i)
		{
			// Find matching pair and extract its low element
			if (!used[i] && pairs[i].first == high)
			{
				pendElements.push_back(pairs[i].second);
				pairedHighs.push_back(high);  // Store corresponding high
				used[i] = true;  // Mark as used
				break;
			}
		}
	}

	// Step 6: Insert pendElements using binary search with Jacobsthal sequence
	// Limit search range to the position of each element's paired high
	if (!pendElements.empty())
		mainChain.insert(mainChain.begin(), pendElements[0]);

	// Generate Jacobsthal sequence for insertion batches
	std::vector<size_t> jacobsthal = generateJacobsthalSequence(pendElements.size());

	size_t prevBound = 1;  // Already inserted pendElements[0]

	for (size_t bound : jacobsthal)
	{
		if (bound > pendElements.size())
			bound = pendElements.size();

		// Insert elements from prevBound to bound (right to left, in reverse order)
		for (int i = static_cast<int>(bound) - 1; i >= static_cast<int>(prevBound); --i)
		{
			int elementToInsert = pendElements[i];
			int pairedHigh = pairedHighs[i];  // Get the corresponding high element

			// Find position of the paired high element in mainChain
			auto limitIt = std::lower_bound(mainChain.begin(), mainChain.end(), pairedHigh);

			// Binary search for insertion point, limited to position of paired high
			// Since low <= high, search ends at limitIt
			auto insertionPoint = std::upper_bound(mainChain.begin(), limitIt, elementToInsert);
			mainChain.insert(insertionPoint, elementToInsert);
		}

		prevBound = bound;

		if (bound == pendElements.size())
			break;
	}

	// Step 7: Insert the straggler (odd element) if it exists
	if (hasStraggler)
	{
		auto insertionPoint = std::upper_bound(mainChain.begin(), mainChain.end(), straggler);
		mainChain.insert(insertionPoint, straggler);
	}

	deq.swap(mainChain);
}

void PmergeMe::run(int argc, char* argv[])
{
	if (!parseArguments(argc, argv))
	{
		std::cerr << "Error" << std::endl;
		return;
	}

	std::cout << "Before: ";
	printVectorContainer(_vector);

	// --- Time measurement for std::vector ---
	auto startTimeVec = std::chrono::high_resolution_clock::now();
	fordJohnsonSortVector(_vector);
	auto endTimeVec = std::chrono::high_resolution_clock::now();
	auto durationVec = std::chrono::duration_cast<std::chrono::microseconds>(endTimeVec - startTimeVec);

	// --- Time measurement for std::deque ---
	auto startTimeDeq = std::chrono::high_resolution_clock::now();
	fordJohnsonSortDeque(_deque);
	auto endTimeDeq = std::chrono::high_resolution_clock::now();
	auto durationDeq = std::chrono::duration_cast<std::chrono::microseconds>(endTimeDeq - startTimeDeq);

	std::cout << "After:  ";
	printVectorContainer(_vector);

	std::cout << "Time to process a range of " << _vector.size()
			  << " elements with std::vector : " << durationVec.count() << " us" << std::endl;

	std::cout << "Time to process a range of " << _deque.size()
			  << " elements with std::deque  : " << durationDeq.count() << " us" << std::endl;
}
