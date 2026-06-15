#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <chrono>
#include <algorithm>
#include <type_traits>

class PmergeMe {
private:
	std::vector<int>	_vector;
	std::deque<int>		_deque;

	bool parseArguments(int argc, char* argv[]);
	std::vector<size_t> generateJacobsthalSequence(size_t n) const;

	template <typename Container>
	void fordJohnsonSort(Container& c);

	template <typename Container>
	void printContainer(const Container& c) const;

public:
	PmergeMe();
	PmergeMe(const PmergeMe& other);
	PmergeMe& operator=(const PmergeMe& other);
	~PmergeMe();

	void run(int argc, char* argv[]);
};

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

	// Step 1: Handle odd element (straggler) if list has odd number of elements
	bool hasStraggler = (c.size() % 2 != 0);
	int straggler = 0;
	if (hasStraggler)
	{
		straggler = c.back();
		c.pop_back();
	}

	// Step 2: Create pairs by comparing adjacent elements
	using PairType = std::pair<int, int>;
	using PairContainer = std::conditional_t<
		std::is_same_v<Container, std::vector<int>>,
		std::vector<PairType>,
		std::deque<PairType>
	>;

	PairContainer pairs;
	if constexpr (std::is_same_v<Container, std::vector<int>>)
		pairs.reserve(c.size() / 2);

	for (size_t i = 0; i < c.size(); i += 2)
	{
		if (c[i] < c[i + 1])
			pairs.push_back(std::make_pair(c[i + 1], c[i]));
		else
			pairs.push_back(std::make_pair(c[i], c[i + 1]));
	}

	// Step 3: Recursively sort the larger elements from each pair
	Container mainChain;
	if constexpr (std::is_same_v<Container, std::vector<int>>)
		mainChain.reserve(pairs.size());

	for (const auto& [high, low] : pairs)
		mainChain.push_back(high);

	fordJohnsonSort(mainChain);

	// Step 4: Track pairs so we can find smaller elements from sorted larger elements
	struct PairTracker {
		int high;
		int low;
		bool used;
	};

	using TrackerContainer = std::conditional_t<
		std::is_same_v<Container, std::vector<int>>,
		std::vector<PairTracker>,
		std::deque<PairTracker>
	>;

	TrackerContainer tracker;
	if constexpr (std::is_same_v<Container, std::vector<int>>)
		tracker.reserve(pairs.size());

	for (const auto& [high, low] : pairs)
		tracker.push_back({high, low, false});

	Container pendElements; // Smaller elements to insert
	Container pairedHighs; // Track which larger element each smaller element belongs to
	if constexpr (std::is_same_v<Container, std::vector<int>>) {
		pendElements.reserve(pairs.size());
		pairedHighs.reserve(pairs.size());
	}

	// Collect smaller elements in sorted order by following mainChain
	for (int high : mainChain)
	{
		for (auto& t : tracker)
		{
			if (!t.used && t.high == high)
			{
				pendElements.push_back(t.low);
				pairedHighs.push_back(high);
				t.used = true;
				break;
			}
		}
	}

	// Step 5: Insert smaller elements using Jacobsthal sequence for efficiency
	if (!pendElements.empty())
		mainChain.insert(mainChain.begin(), pendElements[0]);

	// Jacobsthal sequence determines optimal insertion order
	std::vector<size_t> jacobsthal = generateJacobsthalSequence(pendElements.size());

	int lastInsertedIdx = 1;
	for (size_t k = 0; k < jacobsthal.size(); ++k)
	{
		int endIdx = static_cast<int>(jacobsthal[k]) - 1;
		if (endIdx >= static_cast<int>(pendElements.size()))
			endIdx = static_cast<int>(pendElements.size()) - 1;

		for (int idx = endIdx; idx >= lastInsertedIdx; --idx)
		{
			int elementToInsert = pendElements[idx];
			int pairedHigh = pairedHighs[idx];

			auto limitIt = std::lower_bound(mainChain.begin(), mainChain.end(), pairedHigh);
			auto insertionPoint = std::upper_bound(mainChain.begin(), limitIt, elementToInsert);
			mainChain.insert(insertionPoint, elementToInsert);

			if (idx == lastInsertedIdx)
				break;
		}

		if (endIdx >= static_cast<int>(pendElements.size()) - 1)
			break;

		lastInsertedIdx = endIdx + 1;
	}

	// Step 6: Insert the straggler (odd element) if it exists
	if (hasStraggler)
	{
		auto insertionPoint = std::upper_bound(mainChain.begin(), mainChain.end(), straggler);
		mainChain.insert(insertionPoint, straggler);
	}

	c.swap(mainChain);
}
