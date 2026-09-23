#include "HTTPRequest.h"

#include "URLUtils.h"

HTTPRequest::HTTPRequest(const std::string& message, uint64_t clientSocket)
{
	std::string method = message.substr(0, message.find(' '));
	
	std::string url = message.substr(message.find(' ') + 1, message.find(' ', message.find(' ') + 1) - message.find(' ') - 1);
	url = URLUtils::Decode(url);
	
	std::string httpVersion = message.substr(message.find(' ', message.find(' ') + 1) + 1, message.find('\r', message.find(' ', message.find(' ') + 1)) - message.find(' ', message.find(' ') + 1) - 1);
	std::string body = message.substr(message.find("\r\n\r\n") + 4);

	m_Method = HTTPMethod(method);
	m_URL = url;
	m_HTTPVersion = httpVersion;
	m_Body = body;
	m_ClientSocket = clientSocket;
}

bool HTTPRequest::IsValid()
{
	return m_Method != HTTPMethodType::UNKNOWN &&
		   !m_URL.empty() &&
		   m_HTTPVersion == "HTTP/1.0" ||
		   m_HTTPVersion == "HTTP/1.1";
}