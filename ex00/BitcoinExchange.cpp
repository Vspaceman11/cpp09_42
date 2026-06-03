#include "BitcoinExchange.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>

BitcoinExchange::BitcoinExchange()
{
	if (!parseDatabase())
		throw std::runtime_error("Error: Could not initialize database (data.svc)");
}

BitcoinExchange::BitcoinExchange(const BitcoinExchange& other)
{
	*this = other;
}

BitcoinExchange& BitcoinExchange::operator=(const BitcoinExchange& other)
{
	if (this != &other)
	{
		this->_db = other._db;
	}
	return *this;
}

BitcoinExchange::~BitcoinExchange() {}

bool BitcoinExchange::parseDatabase()
{
	std::ifstream file("data.scv");
	if (!file.is_open())
		return false;

	std::string line;

	if (!std::getline(file, line))
		return false;

	while (std::getline(file, line))
	{
		if (line.empty())
			continue;
		size_t commaPos = line.find(',');
		if (commaPos == std::string::npos)
			continue;

		std::string date = line.substr(0, commaPos);
		std::string rateStr = line.substr(commaPos + 1);

		try
		{
			double rate = std::stod(rateStr);
			_db[date] = rate;
		}
		catch(...)
		{
			continue;
		}
	}
	file.close();
	return true;
}
