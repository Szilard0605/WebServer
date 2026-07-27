#include "Server.h"

#include <stdio.h>

//#include <WinSock2.h>
//#include <ws2tcpip.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <fstream>
#include <sstream>

#include <iostream>

#include "json.hpp"

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
}

WebServer::~WebServer()
{
    close(m_SocketHandler);
    //shutdown(m_SocketHandler, SD_BOTH);
}

bool WebServer::Start(const int Port)
{
    m_Port = Port;
    m_SocketHandler = socket(AF_INET, SOCK_STREAM, 0);

    if (m_SocketHandler == -1)
    {
        printf("Couldn't open socket handler\n");
        return false;
    }

    int flags = fcntl(m_SocketHandler, F_GETFL, 0); 
    fcntl(m_SocketHandler, F_SETFL, flags | O_NONBLOCK);


    sockaddr_in sAddr;
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(m_Port);
    sAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_SocketHandler, (struct sockaddr*)&sAddr, sizeof(sAddr)) == -1)
    {
        printf("Couldn't bind socket: %s\n", strerror(errno));
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
    int clientSocket = accept(m_SocketHandler, (sockaddr*)&clientAddr, &addrlen);

    if (clientSocket == -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return; // no client waiting

        std::cout << "Error in select: " << strerror(errno) << std::endl; 
        return;
    }
    struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&clientAddr;
    struct in_addr ipAddr = pV4Addr->sin_addr;

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddr, clientIP, INET_ADDRSTRLEN);

    u_short clientPort = ntohs(pV4Addr->sin_port);



    FD_ZERO(&s_ReadFDS);
    FD_SET(clientSocket, &s_ReadFDS);
 
    int socketCount = select(clientSocket + 1, &s_ReadFDS, nullptr, nullptr, nullptr);

    if (socketCount == -1) 
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
            std::cerr << "Error in recv.\n";
    
        }
    }
}

void WebServer::HandleMessage(const char* buffer, int bytesReceived, uint32_t clientSocket, const char* clientIP, int clientPort)
{
    printf("%d bytes received\n", bytesReceived);

    if (bytesReceived <= 0)
        return;
  
    char* message = new char[bytesReceived + 1];
    memcpy(message, buffer, bytesReceived);
    message[bytesReceived] = '\0';

    std::string url = ParseURLFromMessage(message);
    printf("url: %s\n", url.c_str());

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
    close(m_SocketHandler);
    //WSACleanup();
    //shutdown(m_SocketHandler, SD_BOTH);
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
            if (errno == EWOULDBLOCK) 
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
    std::ifstream pageSrc(std::string("html/") + fileName, std::ios::binary);
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
    else
    {
        std::cout << "Couldn't open file: " << fileName;
    }
    close(clientSocket);
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
