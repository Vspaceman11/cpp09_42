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

	// Helper methods for validation and parsing
	bool parseArguments(int argc, char* argv[]);

	// Generation of Jacobsthal numbers to optimize insertion order
	std::vector<size_t> generateJacobsthalSequence(size_t n) const;

	// Template method for the Ford-Johnson algorithm itself
	template <typename Container>
	void fordJohnsonSort(Container& c);

	// Template to print container elements to the console
	template <typename Container>
	void printContainer(const Container& c) const;

public:
	// --- Orthodox Canonical Form ---
	PmergeMe();
	PmergeMe(const PmergeMe& other);
	PmergeMe& operator=(const PmergeMe& other);
	~PmergeMe();

	// Main method to run the program
	void run(int argc, char* argv[]);
};

// Implementation of template methods must be in the header or included in it,
// since the compiler needs to see the template body during instantiation.

template <typename Container>
void PmergeMe::printContainer(const Container& c) const
{
	typename Container::const_iterator it;
	for (it = c.begin(); it != c.end(); ++it)
	{
		std::cout << *it << " ";
	}
	std::cout << std::endl;
}

template <typename Container>
void PmergeMe::fordJohnsonSort(Container& c)
{
	if (c.size() < 2)
		return;

	// Step 1: Check for a straggler element (if the size is odd)
	bool hasStraggler = (c.size() % 2 != 0);
	int straggler = 0;
	if (hasStraggler)
	{
		straggler = c.back();
		c.pop_back();
	}

	// Step 2: Divide into pairs and pairwise comparison
	// Create a container of pairs. Each pair: (larger, smaller)
	std::vector<std::pair<int, int>> pairs;
	for (size_t i = 0; i < c.size(); i += 2)
	{
		if (c[i] < c[i + 1])
			pairs.push_back(std::make_pair(c[i + 1], c[i]));
		else
			pairs.push_back(std::make_pair(c[i], c[i + 1]));
	}

	// Step 3: Recursive sorting of larger elements
	// Collect all larger elements (first in pairs)
	Container mainChain;
	for (size_t i = 0; i < pairs.size(); ++i)
	{
		mainChain.push_back(pairs[i].first);
	}

	// Recursively call the same algorithm for the main chain
	fordJohnsonSort(mainChain);

	// Prepare pend elements with duplicate protection (Bug fix)
	Container pendElements;
	std::vector<bool> used(pairs.size(), false); // Array of flags for "processed" pairs

	for (size_t i = 0; i < mainChain.size(); ++i)
	{
		for (size_t j = 0; j < pairs.size(); ++j)
		{
			// A pair matches only if its value is equal AND it has not been used yet
			if (!used[j] && mainChain[i] == pairs[j].first)
			{
				pendElements.push_back(pairs[j].second);
				used[j] = true; // Mark the pair as used
				break;
			}
		}
	}

	// Step 4: Insert pend elements back into mainChain using Jacobsthal numbers
	// The very first pend element is guaranteed to be less than or equal to the first mainChain element,
	// it can be inserted at the very beginning without checks (0 comparisons)
	if (!pendElements.empty())
		mainChain.insert(mainChain.begin(), pendElements[0]);

	// Generate Jacobsthal sequence for the remaining pend elements
	std::vector<size_t> jacobsthal = generateJacobsthalSequence(pendElements.size());

	size_t lastInsertedIdx = 1;
	for (size_t k = 0; k < jacobsthal.size(); ++k)
	{
		size_t groupEnd = jacobsthal[k];
		if (groupEnd >= pendElements.size())
			groupEnd = pendElements.size() - 1;

		// Insert group elements from right to left (moving backwards to lastInsertedIdx)
		for (size_t idx = groupEnd; idx >= lastInsertedIdx; --idx)
		{
			int elementToInsert = pendElements[idx];
			// Use binary search (std::upper_bound) to minimize comparisons
			typename Container::iterator insertionPoint = std::upper_bound(mainChain.begin(), mainChain.end(), elementToInsert);
			mainChain.insert(insertionPoint, elementToInsert);

			if (idx == lastInsertedIdx) // Protection against unsigned size_t underflow during decrement
				break;
		}
		lastInsertedIdx = groupEnd + 1;
	}

	// Step 5: If there was a straggler (odd) element, insert it at the very end via binary search
	if (hasStraggler)
	{
		typename Container::iterator insertionPoint = std::upper_bound(mainChain.begin(), mainChain.end(), straggler);
		mainChain.insert(insertionPoint, straggler);
	}

	// Write the result back to the original container
	c = mainChain;
}
