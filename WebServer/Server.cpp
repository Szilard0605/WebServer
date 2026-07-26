#include "Server.h"

#include <stdio.h>

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <sstream>

#include <iostream>

#include "json.hpp"

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
            printf("Connectionis closed for %s:%d\n", clientIP, clientPort);
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

    std::string url = ParseURLFromMessage(message);
    printf("url: %s\n", url.c_str());
    if (url == "/create_post")
    {
        std::string postMessage(message);
        std::string postBody;
        int lineBytes = 0;
        for (size_t i = 0; i < bytesReceived; ++i)
        {
            if (message[i] == '\r')
                continue;

            if (message[i] == '\n')
            {
                if (lineBytes == 0)
                {
                    postBody = postMessage.substr(i + 1, (bytesReceived + 2 - i));
                    break;
                }

                lineBytes = 0;
            }
            else lineBytes++;
        }

        nlohmann::json jsonBody = nlohmann::json::parse(postBody);
        std::cout << "--- POST FROM USER ---\n";
        std::cout << "User ID: " << jsonBody["userId"] << std::endl;
        std::cout << "Title: " << jsonBody["title"] << std::endl;
        std::cout << "----------------------\n";
        return;
    }

    printf("%s:%d: %s\n", clientIP, clientPort, message);
   
    if (!url.length())
    {
        printf("No url, redirecting to index\n");
        SendFileToClient("index.html", clientSocket);
        return;
    }
    else
    {
        printf("Need to send: %s\n", url.c_str());
        SendFileToClient(url, clientSocket);
        return;
    }
}


void WebServer::Shutdown()
{
    closesocket(m_SocketHandler);
    WSACleanup();
    shutdown(m_SocketHandler, SD_BOTH);
    m_ServerShouldRun = false;
    printf("[WebServer]: Successfully shutdown\n");
}

void WebServer::LinkRequestToFile(std::string request, PageSource source)
{
    m_Files[request] = source;
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

void WebServer::SendFileToClient(std::string fileName, uint32_t clientSocket)
{
    std::ifstream pageSrc(std::string("html\\") + fileName, std::ios::binary);
    std::stringstream stream;
    if (pageSrc.is_open())
    {
        std::string line;
        while (std::getline(pageSrc, line))
        {
            stream << line << "\n";
        }

        std::stringstream httpResponse;
        std::string type = GetSourceTypeByFilename(fileName);
        httpResponse << "HTTP/1.1 200 OK\n"
            << "Content-Type: " << type << "\n"
            << "Content-Length: " << stream.str().size() << "\n\n"
            << stream.str();

        if (SendDataToClient(clientSocket, httpResponse.str().c_str(), httpResponse.str().size()))
        {
            printf("Sent %s\n", fileName.c_str());
        }
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
    std::string type = fileName.substr(pos, fileName.length());
    
    if (extMap.find(type) != extMap.end())
        return extMap[type];

    return "Unknown";
}
