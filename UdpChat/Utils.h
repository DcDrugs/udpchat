#pragma once
#include <string>
#include <vector>
#include <iostream>

std::string strip(const std::string& inpt);

std::string Convert(std::vector<int> str);
std::vector<int> Convert(std::string str);
std::vector<int> Convert(int* start, int* end);