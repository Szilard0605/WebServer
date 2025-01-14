#pragma once

#include <string>

class FamPong
{
public:
	struct User
	{
		unsigned long long Socket;
		std::string Name;
		bool IsHost = false;
		std::string Address;
		int Port;
		int ID = -1;

		int Movement = -1;
	};

	struct Data
	{
		int ClientCount = 0;
		User Users[3];
		bool GameStarted = false;
	};

	static void Init();
	static void Destroy();

	static void OnClientConnect(unsigned long long Socket, std::string Address, int Port, bool& IsHost);
	static void ParsePlayerMovement(std::string message, int& PlayerID, int& Movement);
	static void OnPlayerMove(int PlayerID, int Movement);

};

