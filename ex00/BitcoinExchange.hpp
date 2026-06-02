#pragma once
#include <string>
#include <string_view>
#include <map>
#include <iostream>

class BitcoinExchange
{
	private:
		std::map<std::string, double> _db;

		bool parseDatabase();
		bool isValidDate(std::string_view date) const;
		bool isValidValue(std::string_view valueStr, double& value) const;
		void trim(std::string_view& str) const;

	public:
		BitcoinExchange();
		BitcoinExchange(const BitcoinExchange& other);
		BitcoinExchange& operator=(const BitcoinExchange& other);
		~BitcoinExchange();

		void processInputFile(const std::string& inputFilename);
};
