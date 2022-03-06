#include "Utils.h"


std::string strip(const std::string& inpt)
{
	auto start_it = inpt.begin();
	auto end_it = inpt.rbegin();
	while (std::isspace(*start_it))
		++start_it;
	while (std::isspace(*end_it))
		++end_it;
	return std::string(start_it, end_it.base());
}

std::string Convert(std::vector<int> str) {
	std::string res;
	res.reserve(str.size());
	for (auto i : str)
		res.push_back(i);

	return res;
}

std::vector<int> Convert(std::string str) {
	std::vector<int> res;
	res.reserve(str.size());
	for (auto i : str)
		res.push_back(i);

	return res;
}

std::vector<int> Convert(int* start, int* end) {
	std::vector<int> res;
	size_t size = end - start;
	res.resize(size);

	for (int i = 0; i < res.size(); ++i)
	{
		res[i] = start[i];
	}

	return res;
}