#include "URLUtils.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <stdio.h>

std::unordered_map<char, std::string> URL_ESCAPE_MAP = {
	{' ',  "%20"},
	{'!',  "%21"},
	{'#',  "%23"},
	{'$',  "%24"},
	{'%',  "%25"},
	{'&',  "%26"},
	{'\'', "%27"},
	{'(',  "%28"},
	{')',  "%29"},
	{'*',  "%2A"},
	{'+',  "%2B"},
	{',',  "%2C"},
	{'.',  "%2E"},
	{'/',  "%2F"},
	{':',  "%3A"},
	{';',  "%3B"},
	{'=',  "%3D"},
	{'?',  "%3F"},
	{'@',  "%40"},
	{'{',  "%5B"},
	{'}',  "%5D"},
};

char hex_to_char(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';

	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;

	return -1;
}

std::string URLUtils::Encode(std::string url)
{
	std::string ret;
	for (char c : url)
	{
		auto it = URL_ESCAPE_MAP.find(c);

		if (it != URL_ESCAPE_MAP.end())
		{
			ret += it->second;
		}
		else
		{
			ret += c;
		}
	}
	return ret;
}

std::string URLUtils::Decode(std::string url)
{
	std::string ret;
	for (size_t i = 0; i < url.length(); i++)
	{
		if (url[i] == '%' && i + 2 < url.length())
		{
			char high_bit = hex_to_char(url[i + 1]);
			char low_bit = hex_to_char(url[i + 2]);

			if (high_bit != -1 && low_bit != -1)
			{
				ret += static_cast<char>((high_bit << 4) | low_bit);
				i += 2;
				continue;
			}
		}
		ret += url[i];
	}
	return ret;
}

std::string URLUtils::Normalize(std::string url)
{
	return std::string();
}
