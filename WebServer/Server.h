#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <filesystem>

#include "HTTPRequest.h"

class WebServer
{
public:
	struct PageSource
	{
		std::string Path = "";
		std::string Type = "";
	};

	WebServer();
	~WebServer();
	bool Start(const char* Address, const int Port);
	void Update();
	void Shutdown();

	inline bool ShouldRun() { return m_ServerShouldRun; }

	static WebServer* GetInstance() { return s_ServerInstance; }

	bool SendDataToClient(uint32_t socket, const char* data, int size);
	bool HTMLSourceFileExists(const std::string& url);
	bool DirectoryExists(const std::string& dirName);
	void SendFileToClient(const std::string& fileName, uint32_t clientSocket);

	void HandleMessage(const char* buffer, int bytesReceived, uint32_t clientSocket, const char* clientIP, int clientPort);
	void HandleRequest(HTTPRequest request);
private:
	bool m_ServerShouldRun = false;

	static WebServer* s_ServerInstance;

	std::string ParseURLFromMessage(std::string message);
	std::string GetSourceTypeByFilename(std::string fileName);

	unsigned long long m_SocketHandler = 0;
	std::string m_Address;
	int m_Port = 0;
	int m_ClientCount = 0;
};

