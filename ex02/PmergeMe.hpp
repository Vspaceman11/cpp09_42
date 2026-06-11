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
	std::vector<int>	_vector;
	std::deque<int>		_deque;

	// Helper methods for validation and parsing
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
	std::vector<std::pair<int, int>> pairs;
	pairs.reserve(c.size() / 2);
	for (size_t i = 0; i < c.size(); i += 2)
	{
		if (c[i] < c[i + 1])
			pairs.push_back(std::make_pair(c[i + 1], c[i]));
		else
			pairs.push_back(std::make_pair(c[i], c[i + 1]));
	}

	// Step 3: Recursive sorting of larger elements
	Container mainChain;
	if constexpr (std::is_same_v<Container, std::vector<int>>)
		mainChain.reserve(pairs.size());
	for (const auto& [high, low] : pairs)
		mainChain.push_back(high);

	// Recursively call the same algorithm for the main chain
	fordJohnsonSort(mainChain);

	// Align pend elements with sorted main chain (FIFO per high, safe for duplicates)
	std::map<int, std::deque<int>> lowsByHigh;
	for (const auto& [high, low] : pairs)
		lowsByHigh[high].push_back(low);

	Container pendElements;
	std::vector<int> pairedHighs;
	pairedHighs.reserve(pairs.size());
	if constexpr (std::is_same_v<Container, std::vector<int>>)
		pendElements.reserve(pairs.size());
	for (int high : mainChain)
	{
		pendElements.push_back(lowsByHigh[high].front());
		lowsByHigh[high].pop_front();
		pairedHighs.push_back(high);
	}

	// Step 4: Insert pend elements back into mainChain using Jacobsthal numbers
	if (!pendElements.empty())
		mainChain.insert(mainChain.begin(), pendElements[0]);

	std::vector<size_t> jacobsthal = generateJacobsthalSequence(pendElements.size());

	size_t lastInsertedIdx = 1;
	for (size_t k = 0; k < jacobsthal.size(); ++k)
	{
		size_t endIdx = jacobsthal[k] - 1;
		if (endIdx >= pendElements.size())
			endIdx = pendElements.size() - 1;

		for (size_t idx = endIdx; idx >= lastInsertedIdx; --idx)
		{
			int elementToInsert = pendElements[idx];
			int pairedHigh = pairedHighs[idx];
			auto limitIt = std::lower_bound(mainChain.begin(), mainChain.end(), pairedHigh);
			auto insertionPoint = std::upper_bound(mainChain.begin(), limitIt, elementToInsert);
			mainChain.insert(insertionPoint, elementToInsert);

			if (idx == lastInsertedIdx)
				break;
		}

		if (endIdx >= pendElements.size() - 1)
			break;

		lastInsertedIdx = endIdx + 1;
	}

	// Step 5: If there was a straggler (odd) element, insert it at the very end via binary search
	if (hasStraggler)
	{
		auto insertionPoint = std::upper_bound(mainChain.begin(), mainChain.end(), straggler);
		mainChain.insert(insertionPoint, straggler);
	}

	c.swap(mainChain);
}
