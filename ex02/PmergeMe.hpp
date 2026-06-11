#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <map>
#include <string>
#include <chrono>
#include <algorithm>
#include <type_traits>

class PmergeMe {
private:
	// numbers stored for timing with std::vector
	std::vector<int>	_vector;
	// numbers stored for timing with std::deque
	std::deque<int>		_deque;

	// Helper methods for validation and parsing
	// parseArguments: read argv and validate numeric inputs
	bool parseArguments(int argc, char* argv[]);

	// Generation of Jacobsthal numbers to optimize insertion order
	std::vector<size_t> generateJacobsthalSequence(size_t n) const;

	// Template method for the Ford-Johnson algorithm itself
	template <typename Container>
	void fordJohnsonSort(Container& c);

	// Template to print container elements
	template <typename Container>
	void printContainer(const Container& c) const;

public:
	PmergeMe();
	PmergeMe(const PmergeMe& other);
	PmergeMe& operator=(const PmergeMe& other);
	~PmergeMe();

	// Main method to run the program
	void run(int argc, char* argv[]);
};

// Implementation of template methods
template <typename Container>
void PmergeMe::printContainer(const Container& c) const
{
	for (const auto& element : c)
	{
		std::cout << element << " ";
	}
	std::cout << std::endl;
}

template <typename Container>
void PmergeMe::fordJohnsonSort(Container& c)
{
	// fordJohnsonSort: implement Ford-Johnson (merge-insertion) sorting
	// keeps main sorted chain and inserts pend elements efficiently
	// If container has fewer than 2 elements it's already sorted
	if (c.size() < 2)
		return;

	// Step 1: Check for a "straggler" element (if size is odd)
	bool hasStraggler = (c.size() % 2 != 0);
	int straggler = 0;
	if (hasStraggler)
	{
		straggler = c.back();
		c.pop_back();
	}

	// Step 2: Divide into pairs and perform pairwise comparison
	// For each consecutive pair we record (high, low).
	// Storing high first builds the "main chain" of larger elements.
	// This simplifies later binary searches because mainChain contains
	// the representatives (the larger member of each pair).
	std::vector<std::pair<int, int>> pairs;
	pairs.reserve(c.size() / 2);
	for (size_t i = 0; i < c.size(); i += 2)
	{
		// compare two elements and store them as (high, low)
		if (c[i] < c[i + 1])
			pairs.push_back(std::make_pair(c[i + 1], c[i]));
		else
			pairs.push_back(std::make_pair(c[i], c[i + 1]));
	}

	// After this loop 'pairs' contains pairs where first is the larger
	// element and second is its partner (the one to be inserted later).

	// Step 3: Recursive sorting of larger elements
	// mainChain holds the larger element from each pair and is
	// usually roughly half the original size. Sorting this chain
	// first reduces the number of costly insertions later.
	Container mainChain;
	if constexpr (std::is_same_v<Container, std::vector<int>>)
		mainChain.reserve(pairs.size());
	for (const auto& [high, low] : pairs)
		mainChain.push_back(high);

	// Recursively sort the chain of highs — this forms the backbone
	// into which the smaller pair members (pend elements) will be inserted.
	fordJohnsonSort(mainChain);

	// Align pend elements with sorted main chain. We built a map from
	// each high -> deque of its paired lows to handle duplicate highs.
	// Using deque preserves the original pairing order (FIFO) so that
	// when we iterate the sorted mainChain we pop the corresponding low
	// that belonged to that high in the original pairing.
	std::map<int, std::deque<int>> lowsByHigh;
	for (const auto& [high, low] : pairs)
		lowsByHigh[high].push_back(low);

	// pendElements will be inserted back into the sorted main chain.
	Container pendElements;
	std::vector<int> pairedHighs;
	pairedHighs.reserve(pairs.size());
	if constexpr (std::is_same_v<Container, std::vector<int>>)
		pendElements.reserve(pairs.size());
	for (int high : mainChain)
	{
		// take the low that was originally paired with this high
		pendElements.push_back(lowsByHigh[high].front());
		lowsByHigh[high].pop_front();
		pairedHighs.push_back(high);
	}

	// Now 'pendElements[i]' should be inserted relative to 'pairedHighs[i]'
	// so the original pair relationships are preserved after full insertion.

	// Step 4: Insert pend elements back into mainChain using Jacobsthal numbers
	// The first pend element is inserted at the front to seed the chain.
	if (!pendElements.empty())
		mainChain.insert(mainChain.begin(), pendElements[0]);

	// Jacobsthal sequence determines groups of pend elements to insert.
	// This order reduces the total number of comparisons needed overall.
	std::vector<size_t> jacobsthal = generateJacobsthalSequence(pendElements.size());

	size_t lastInsertedIdx = 1;
	for (size_t k = 0; k < jacobsthal.size(); ++k)
	{
		size_t endIdx = jacobsthal[k] - 1;
		if (endIdx >= pendElements.size())
			endIdx = pendElements.size() - 1;

		// insert pend elements in reverse within the group so that earlier
		// insertions do not invalidate binary search regions for later ones.
		for (size_t idx = endIdx; idx >= lastInsertedIdx; --idx)
		{
			int elementToInsert = pendElements[idx];
			int pairedHigh = pairedHighs[idx];

			// find the range in mainChain up to the pairedHigh
			auto limitIt = std::lower_bound(mainChain.begin(), mainChain.end(), pairedHigh);
			// insert elementToInsert before pairedHigh but after any equals
			auto insertionPoint = std::upper_bound(mainChain.begin(), limitIt, elementToInsert);
			mainChain.insert(insertionPoint, elementToInsert);

			// The loop decrements idx; break when reaching lastInsertedIdx to
			// avoid underflow of size_t (this prevents idx wrapping to huge value).
			if (idx == lastInsertedIdx)
				break;
		}

		// if we reached the last pend element, stop early
		if (endIdx >= pendElements.size() - 1)
			break;

		lastInsertedIdx = endIdx + 1;
	}

	// Notes on bounds:
	// - lower_bound(mainChain, pairedHigh) finds first element >= pairedHigh.
	// - upper_bound(..., elementToInsert) ensures stability: when elementToInsert
	//   equals existing values, it is placed after them but still before pairedHigh.

	// Step 5: If there was a straggler (odd) element, insert it at the very end via binary search
	if (hasStraggler)
	{
		auto insertionPoint = std::upper_bound(mainChain.begin(), mainChain.end(), straggler);
		mainChain.insert(insertionPoint, straggler);
	}

	c.swap(mainChain);
}
