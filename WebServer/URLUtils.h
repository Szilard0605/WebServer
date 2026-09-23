#pragma once
#include <string>

class URLUtils
{
public:
	static std::string Encode(std::string url);
	static std::string Decode(std::string url);
	static std::string Normalize(std::string url);
};

