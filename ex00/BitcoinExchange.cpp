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

bool BitcoinExchange::isValidDate(std::string_view date) const
{
	if (date.length() != 10 || date [4] != '-' || date[7] != '-')
		return false;

	for (size_t i = 0; i < date.length(); ++i)
	{
		if (i != 4 && i != 7 && !std::isdigit (date[i]))
			return false;
	}


	int year;
	int month;
	int day;
	try
	{
		year = std::stoi(std::string(date.substr(0, 4)));
		month = std::stoi(std::string(date.substr(5, 2)));
		day = std::stoi(std::string(date.substr(8, 2)));
	}
	catch(...)
	{
		return false;
	}


	if (year < 2009 || month < 1 | month > 12 || day < 1)
		return false;

	const int daysInMonths[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	bool isLeap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));

	int maxDays = daysInMonths[month - 1];
	if (month == 2 && isLeap)
		maxDays = 29;
	return day <= maxDays;
}


bool BitcoinExchange::isValidValue(std::string_view valueStr, double& value) const
{
	if (valueStr.empty())
	{
		std::cerr << "Error: empty value" << std::endl;
		return false;
	}

	try
	{
		size_t processedChars = 0;
		std::string s(valueStr);
		value = std::stod(s, &processedChars);

		if (processedChars != s.length())
		{
			std::cerr << "Error: ad input: " << valueStr << std::endl;
			return false;
		}
	}
	catch(...)
	{
		std::cerr << "Error: bad input: " << valueStr << std::endl;
		return false;
	}

	if (value < 0)
	{
		std::cerr << "Error: not a positive number" << std::endl;
		return false;
	}
	if (value > 1000)
	{
		std::cerr << "Error: too large number" << std::endl;
		return false;
	}
	return true;
}


