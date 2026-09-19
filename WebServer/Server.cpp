#include "Server.h"

#include <stdio.h>

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

WebServer* WebServer::s_ServerInstance = nullptr;
fd_set s_ReadFDS;

struct Post
{
    int UserID;
    std::string title;
};

std::vector<Post> g_Posts;

WebServer::WebServer()
{
    if (s_ServerInstance)
    {
        printf("Error: Server instance already exists\n");
        return;
    }

    s_ServerInstance = this;

    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);

    int wsOk = WSAStartup(ver, &wsData);
    if (wsOk != 0)
    {
        printf("Couldn't startup WSA\n");
        return;
    }
}

WebServer::~WebServer()
{
    closesocket(m_SocketHandler);
    WSACleanup();
    shutdown(m_SocketHandler, SD_BOTH);
}

bool WebServer::Start(const char* Address, const int Port)
{
    m_Port = Port;

    m_SocketHandler = socket(AF_INET, SOCK_STREAM, 0);

    if (m_SocketHandler == SOCKET_ERROR)
    {
        printf("Couldn't open socket handler\n");
        return false;
    }

    u_long iMode = 1;
    if (ioctlsocket(m_SocketHandler, FIONBIO, &iMode) == SOCKET_ERROR)
    {
        printf("ioctlsocket failed\n");
        return false;
    }

    sockaddr_in sAddr;
    sAddr.sin_family = AF_INET;
    
    sAddr.sin_port = htons(m_Port);
    int res = inet_pton(AF_INET, Address, &sAddr.sin_addr.S_un.S_addr);
    if (res <= 0)
    {
        printf("Failed to bind IPv4 Address: %s\n", Address);
        return false;
    }

    if (bind(m_SocketHandler, (sockaddr*)&sAddr, sizeof(sAddr)) != 0)
    {
        printf("Couldn't bind socket\n");
        return false;
    }

    listen(m_SocketHandler, SOMAXCONN);

    m_ServerShouldRun = true;
    return true;
}

void WebServer::Update()
{

    sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    SOCKET clientSocket = accept(m_SocketHandler, (sockaddr*)&clientAddr, &addrlen);

    struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&clientAddr;
    struct in_addr ipAddr = pV4Addr->sin_addr;

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddr, clientIP, INET_ADDRSTRLEN);

    u_short clientPort = ntohs(pV4Addr->sin_port);


    if (clientSocket <= 0)
    {
        printf("Accepting connection request failed from address: %s:%d\n", clientIP, clientPort);
        return;
    }
    FD_ZERO(&s_ReadFDS);
    FD_SET(clientSocket, &s_ReadFDS);

    struct timeval timeout;
    timeout.tv_sec = 0;  
    timeout.tv_usec = 0;

    int socketCount = select(0, &s_ReadFDS, nullptr, nullptr, nullptr);
    if (socketCount == SOCKET_ERROR) 
        return;

    else if (socketCount > 0) 
    {
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) 
        {
            HandleMessage(buffer, bytesReceived, clientSocket, clientIP, clientPort);
        }
        else if (bytesReceived == 0) 
        {
            printf("Connection is closed for %s:%d\n", clientIP, clientPort);
            return;
        }
        else
        {
            int errorCode = WSAGetLastError(); 
            if (errorCode != WSAEWOULDBLOCK)
            {
                std::cerr << "Error in recv: " << errorCode << "\n";
                return;
            }
        }
    }
}

void WebServer::HandleMessage(const char* buffer, int bytesReceived, uint32_t clientSocket, const char* clientIP, int clientPort)
{
    if (bytesReceived <= 0)
        return;
  
    char* message = new char[bytesReceived + 1];
    memcpy(message, buffer, bytesReceived);
    message[bytesReceived] = '\0';

    //printf(message);

	HTTPRequest request(message, clientSocket);

	printf("%s:%d: %s: %s\n", clientIP, clientPort, request.Method().string().c_str(), request.URL().c_str());

    if (!request.URL().length())
    {
        SendFileToClient("index.html", clientSocket);
        return;
    }
    
    HandleRequest(request);
}

void WebServer::HandleRequest(HTTPRequest request)
{
    if (request.Method() == HTTPMethodType::GET)
    {
        std::string url = request.URL();

        if (url == "/")
        {
            SendFileToClient("index.html", request.ClientSocket());
            return;
        }

        if (DirectoryExists(url))
        {
            SendFileToClient(url + "/index.html", request.ClientSocket());
            return;
        }

        if (!HTMLSourceFileExists(url))
        {
            SendFileToClient("404.html", request.ClientSocket());
            return;
        }
        else
        {
            SendFileToClient(url, request.ClientSocket());
            return;
        }
    }
}

bool WebServer::HTMLSourceFileExists(const std::string& url)
{
    std::string path = std::string("html") + url;
    std::ifstream file(path, std::ios::binary);
    if (file.is_open())
    {
        file.close();
        return true;
    }
    return false;
}

bool WebServer::DirectoryExists(const std::string& dirName)
{
    return std::filesystem::is_directory(std::filesystem::path("html" + dirName));
}

void WebServer::Shutdown()
{
    closesocket(m_SocketHandler);
    WSACleanup();
    shutdown(m_SocketHandler, SD_BOTH);
    m_ServerShouldRun = false;
    printf("[WebServer]: Successfully shutdown\n");
}

bool WebServer::SendDataToClient(uint32_t socket, const char* data, int size)
{
    int totalSent = 0;
    while (totalSent < size) 
    {
        int sent = send(socket, data + totalSent, size - totalSent, 0);
        if (sent > 0) 
        {
            totalSent += sent;
        }
        else if (sent == 0) 
        {
            printf("[WebServer]: Send: Connection is closed\n");
            return false;
        }
        else 
        {
            int errorCode = WSAGetLastError(); 
            if (errorCode == WSAEWOULDBLOCK) 
            {
                printf("[WebServer]: Send: Socket buffer is full, retry later\n");
                return false;
            }
            else 
            {
                printf("[WebServer]: Failed to send %d bytes\n", sent);
                return false;
            }
        }
    }
    return true;
}

void WebServer::SendFileToClient(const std::string& fileName, uint32_t clientSocket)
{
    std::string path = std::string("html/") + fileName;
    std::ifstream pageSrc(path, std::ios::binary);
    std::stringstream stream;
    if (pageSrc.is_open())
    {
        std::ostringstream stream;
        stream << pageSrc.rdbuf();
        std::string content = stream.str();

        std::stringstream httpResponse;
        std::string Type = GetSourceTypeByFilename(fileName);
        httpResponse << "HTTP/1.1 200 OK\r\n"
                     << "Content-Type: " << Type << "\r\n"
                     << "Content-Length: " << content.size() << "\r\n"
                     << "Connection: close\r\n" 
                     << "\r\n"
                     << content;

        SendDataToClient(clientSocket, httpResponse.str().c_str(), httpResponse.str().size());
    }
    closesocket(clientSocket);
}

std::string WebServer::ParseURLFromMessage(std::string message)
{
    std::string parsedURL = message.erase(0, 5);
    return message.substr(0, parsedURL.find(' '));
}

std::string WebServer::GetSourceTypeByFilename(std::string fileName)
{
    std::unordered_map<std::string, std::string> extMap = {
        {".css", "text/css"},
        {".js", "text/javascript"},
        {".png", "image/png"}
    };

    size_t pos = fileName.find(".");
    std::string Type = fileName.substr(pos, fileName.length());
    
    if (extMap.find(Type) != extMap.end())
        return extMap[Type];

    return "Unknown";
}
