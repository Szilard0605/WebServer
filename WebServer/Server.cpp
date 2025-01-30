#include "Server.h"

#include <stdio.h>

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <sstream>

#include <iostream>

WebServer* WebServer::s_ServerInstance = nullptr;
fd_set s_ReadFDS;


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

    // Set timeout (optional)
    struct timeval timeout;
    timeout.tv_sec = 0;  // 1 second
    timeout.tv_usec = 0;

    // Use select to check if socket is ready for reading
    int socketCount = select(0, &s_ReadFDS, nullptr, nullptr, nullptr);
    if (socketCount == SOCKET_ERROR) 
    {
        // Handle error
        //std::cerr << "Error in select\n";
        return;
    }
    else if (socketCount > 0) 
    {
        // Socket is ready for reading
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

void WebServer::HandleMessage(const char* buffer, int bytesReceived, unsigned long long clientSocket, const char* clientIP, int clientPort)
{
    if (bytesReceived <= 0)
        return;
  
    char* message = new char[bytesReceived + 1];
    memcpy(message, buffer, bytesReceived);
    message[bytesReceived] = '\0';

    std::string url = ParseURLFromMessage(message);
    PageSource pageSource = GetPageSourceFromURL(url);

    printf("%s:%d: %s\n", clientIP, clientPort, message);
   
    if (!pageSource.Path.length())
    {
        if (strlen(message) <= 0)
            return;

        std::ifstream page("html/NotFound.html");
        std::stringstream stream;
        if (page.is_open())
        {
            std::string line;
            while (std::getline(page, line))
            {
                stream << line << "\n";
            }

            std::stringstream httpResponse;
            httpResponse << "HTTP/1.1 200 OK\n"
                << "Content-Type: " << pageSource.Type << "\n"
                << "Content-Length: " << stream.str().size() << "\n\n"
                << stream.str();

            if (SendDataToClient(clientSocket, httpResponse.str().c_str(), httpResponse.str().size()))
            {
                //printf("Sent %s to %s:%d\n", pageSource.Path.c_str(), clientIP, clientPort);
            }
        }
        closesocket(clientSocket);
    }
    else
    {
        std::ifstream page(pageSource.Path);
        std::stringstream stream;
        if (page.is_open())
        {
            std::string line;
            while (std::getline(page, line))
            {
                stream << line << "\n";
            }

            std::stringstream httpResponse;
            httpResponse << "HTTP/1.1 200 OK\n"
                << "Content-Type: " << pageSource.Type  << "\n"
                << "Content-Length: " << stream.str().size() << "\n\n"
                << stream.str();

            if (SendDataToClient(clientSocket, httpResponse.str().c_str(), httpResponse.str().size()))
            {
                //printf("Sent %s to %s:%d\n", pageSource.Path.c_str(), clientIP, clientPort);
            }
        }
        closesocket(clientSocket);
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

bool WebServer::SendDataToClient(unsigned long long socket, const char* data, int size)
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

std::string WebServer::ParseURLFromMessage(std::string message)
{
    std::string parsedURL = message.erase(0, 5);
    return message.substr(0, parsedURL.find(' '));
}

WebServer::PageSource WebServer::GetPageSourceFromURL(std::string url)
{
    if (m_Files.find(url) != m_Files.end())
    {
        return m_Files.at(url);
    }
    return PageSource();
}
