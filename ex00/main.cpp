#include "BitcoinExchange.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "Error: could not open file.\n";
		return 1;
	}

	try {
		BitcoinExchange exchange;
		exchange.processInputFile(argv[1]);
	} catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
		return 1;
	}

	return 0;
}
