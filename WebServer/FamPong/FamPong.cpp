#include "FamPong.h"

#include "../Server.h"

#include "WinSock2.h"

#include "json.hpp"

FamPong::Data s_FPData;

void FamPong::Init()
{
	WebServer* Server = WebServer::GetInstance();

	// fampong
	Server->LinkRequestToFile("fampong", { "html/fampong/fampong.html", "text/html" });
	Server->LinkRequestToFile("fampong.js", { "html/fampong/fampong.js", "text/javascript"});
	Server->LinkRequestToFile("fampong.css", { "html/fampong/fampong.css", "text/css" });

	printf("[FamPong]: Successfully initialized\n");
}

void FamPong::Destroy()
{
	for (int i = 0; i < 3; i++)
	{
		shutdown(s_FPData.Users[i].Socket, SD_BOTH);
		closesocket(s_FPData.Users[i].Socket);
	}

	printf("[FamPong]: Successfully uninitialized\n");
}

static int dbg = 0;
static int dbgID = 0;

void SendUserData(FamPong::User& user, bool AlreadyConnected = false)
{

	bool thereWasMovement = false;

	if (user.IsHost && AlreadyConnected)
	{

		if (s_FPData.Users[2].Movement != -1)
		{

			printf("sending player2 movement\n");

			thereWasMovement = true;
			int userID   = 2;
			bool isHost  = s_FPData.Users[2].IsHost;
			int movement = s_FPData.Users[2].Movement;

			char jsonData[512];
			sprintf_s(jsonData, sizeof(jsonData), "{\"PlayerID\": %d, \"IsHost\": %s, \"Movement\": %d}", userID, isHost ? "true" : "false", movement);
			//sprintf_s(jsonData, sizeof(jsonData), "{\"PlayerID\": %d, \"IsHost\": %s, \"Movement\": %d}", dbgID, user.IsHost ? "true" : "false", user.Movement);

			// Send JSON data as HTTP response
			std::string httpResponse = /*"HTTP/1.1 200 OK\r\n" +*/
				std::string("HTTP/1.1 200 OK\r\n") +
				std::string("Content-Type: application/json\r\n") +
				"Content-Length: " + std::to_string(strlen(jsonData)) + "\r\n\r\n" +
				jsonData + "\r\n\r\n";

			if (!WebServer::GetInstance()->SendDataToClient(s_FPData.Users[0].Socket, httpResponse.c_str(), httpResponse.length()))
			{
				printf("[FamPong]: Couldn't send HTTP request to: %s:%d\n", s_FPData.Users[0].Address.c_str(), s_FPData.Users[0].Port);
				return;
			}

			s_FPData.Users[2].Movement = -1;
		}

		if (s_FPData.Users[1].Movement != -1)
		{
			printf("sending player1 movement\n");

			thereWasMovement = true;

			int userID   = 1;
			bool isHost  = s_FPData.Users[1].IsHost;
			int movement = s_FPData.Users[1].Movement;

			char jsonData[512];
			sprintf_s(jsonData, sizeof(jsonData), "{\"PlayerID\": %d, \"IsHost\": %s, \"Movement\": %d}", userID, isHost ? "true" : "false", movement);
			//sprintf_s(jsonData, sizeof(jsonData), "{\"PlayerID\": %d, \"IsHost\": %s, \"Movement\": %d}", dbgID, user.IsHost ? "true" : "false", user.Movement);

			// Send JSON data as HTTP response
			std::string httpResponse = /*"HTTP/1.1 200 OK\r\n" +*/
				std::string("HTTP/1.1 200 OK\r\n") +
				std::string("Content-Type: application/json\r\n") +
				"Content-Length: " + std::to_string(strlen(jsonData)) + "\r\n\r\n" +
				jsonData + "\r\n\r\n";

			if (!WebServer::GetInstance()->SendDataToClient(s_FPData.Users[0].Socket, httpResponse.c_str(), httpResponse.length()))
			{
				printf("[FamPong]: Couldn't send HTTP request to: %s:%d\n", s_FPData.Users[0].Address.c_str(), s_FPData.Users[0].Port);
				return;
			}

			s_FPData.Users[1].Movement = -1;
		}
	}

	if (!thereWasMovement)
	{
		char jsonData[512];
		sprintf_s(jsonData, sizeof(jsonData), "{\"PlayerID\": %d, \"IsHost\": %s, \"Movement\": %d}", user.ID, user.IsHost ? "true" : "false", user.Movement);
		//sprintf_s(jsonData, sizeof(jsonData), "{\"PlayerID\": %d, \"IsHost\": %s, \"Movement\": %d}", dbgID, user.IsHost ? "true" : "false", user.Movement);

		// Send JSON data as HTTP response
		std::string httpResponse = /*"HTTP/1.1 200 OK\r\n" +*/
			std::string("HTTP/1.1 200 OK\r\n") +
			std::string("Content-Type: application/json\r\n") +
			"Content-Length: " + std::to_string(strlen(jsonData)) + "\r\n\r\n" +
			jsonData + "\r\n\r\n";


		if (!WebServer::GetInstance()->SendDataToClient(user.Socket, httpResponse.c_str(), httpResponse.length()))
		{
			printf("[FamPong]: Couldn't send HTTP request to: %s:%d\n", user.Address.c_str(), user.Port);
			return;
		}
	}
	
	//printf("[FamPong]: Sent user data to %s:%d\n", user.Address.c_str(), user.Port);

}

void FamPong::OnClientConnect(unsigned long long Socket, std::string Address, int Port, bool& IsHost)
{
	if (Address.length() <= 0)
		return;

	for (int i = 0; i < 3; i++)
	{
		if (Address == s_FPData.Users[i].Address)
		{
			s_FPData.Users[i].Socket  = Socket;
			s_FPData.Users[i].Address = Address;
			s_FPData.Users[i].Port    = Port;

			//printf("[FamPong]: Updated client info for: %s:%d\n", Address.c_str(), Port);
			SendUserData(s_FPData.Users[i], true);

			return;
		}
	}

	s_FPData.Users[s_FPData.ClientCount].IsHost = false;

	if (s_FPData.ClientCount == 0)
	{
		s_FPData.Users[s_FPData.ClientCount].Name = "Host";
		s_FPData.Users[s_FPData.ClientCount].IsHost = true;
	}

	s_FPData.Users[s_FPData.ClientCount].Name = "Player";
	
	s_FPData.Users[s_FPData.ClientCount].Socket  = Socket;
	s_FPData.Users[s_FPData.ClientCount].Address = Address;
	s_FPData.Users[s_FPData.ClientCount].Port    = Port;
	s_FPData.Users[s_FPData.ClientCount].ID      = s_FPData.ClientCount;

	SendUserData(s_FPData.Users[s_FPData.ClientCount]);

	IsHost = s_FPData.Users[s_FPData.ClientCount].IsHost;

	printf("[FamPong]: Client[%d] connected on address: %s:%d\n", s_FPData.ClientCount, Address.c_str(),  Port);

	s_FPData.ClientCount++;
}

void FamPong::ParsePlayerMovement(std::string message, int& PlayerID, int& Movement)
{
	size_t jsonStart = message.find("\r\n\r\n") + 4;
	std::string jsonData = message.substr(jsonStart);

	if (strlen(jsonData.c_str()) <= 0)
		return;

	nlohmann::json JSON = nlohmann::json::parse(jsonData);

	PlayerID = (int)JSON["PlayerID"];
	Movement = (int)JSON["Movement"];
}

void FamPong::OnPlayerMove(int PlayerID, int Movement)
{
	

	for (int i = 0; i < 3; i++)
	{
		if (s_FPData.Users[i].ID == PlayerID)
		{
			s_FPData.Users[i].Movement = Movement;

		}
	}
}

