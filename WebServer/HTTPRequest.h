#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#ifdef _WIN32
#pragma push_macro("DELETE")
#undef DELETE
#endif

enum class HTTPMethodType
{
	UNKNOWN,
	GET,
	POST,
	PUT,
	DELETE,
	HEAD,
	OPTIONS,
	PATCH,
	CONNECT,
	TRACE
};

struct HTTPMethod
{
private:
	std::unordered_map<std::string, HTTPMethodType> m_MethodToStrMap = {
		{ "UNKNOWN", HTTPMethodType::UNKNOWN },
		{ "GET",     HTTPMethodType::GET     },
		{ "POST",    HTTPMethodType::POST    },
		{ "PUT",     HTTPMethodType::PUT     },
		{ "PATCH",   HTTPMethodType::PATCH   },
		{ "DELETE",  HTTPMethodType::DELETE  },
		{ "HEAD",    HTTPMethodType::HEAD    },
		{ "OPTIONS", HTTPMethodType::OPTIONS },
		{ "CONNECT", HTTPMethodType::CONNECT },
		{ "TRACE",   HTTPMethodType::TRACE   }
	};

	std::string m_MethodStr;
public:
	HTTPMethod() = default;

	HTTPMethod(const std::string& method) {
		Type = m_MethodToStrMap[method];
		m_MethodStr = method;
	}

	HTTPMethodType Type;

	std::string string() const
	{
		switch (Type)
		{
		case HTTPMethodType::GET:
			return "GET";
		case HTTPMethodType::POST:
			return "POST";
		case HTTPMethodType::PUT:
			return "PUT";
		case HTTPMethodType::DELETE:
			return "DELETE";
		case HTTPMethodType::HEAD:
			return "HEAD";
		case HTTPMethodType::OPTIONS:
			return "OPTIONS";
		case HTTPMethodType::PATCH:
			return "PATCH";
		case HTTPMethodType::CONNECT:
			return "CONNECT";
		case HTTPMethodType::TRACE:
			return "TRACE";
		default:
			return "UNKNOWN";
		}
	}

	operator const char*() const {
		return m_MethodStr.c_str();
	}

	friend bool operator==(const HTTPMethod& method, const HTTPMethodType type)
	{
		return method.Type == type;
	}
};

class HTTPRequest
{
public:
	HTTPRequest() = default;
	HTTPRequest(const std::string& message, uint64_t clientSocket);

	HTTPMethod Method()		  const { return m_Method;		 }
	std::string URL()	      const { return m_URL;			 }
	std::string HTTPVersion() const { return m_HTTPVersion;  }
	std::string Body()		  const { return m_Body;		 }
	uint64_t ClientSocket()   const { return m_ClientSocket; }
private:
	uint64_t m_ClientSocket;
	HTTPMethod m_Method;
	std::string m_URL;
	std::string m_HTTPVersion;
	std::string m_Body;
};

