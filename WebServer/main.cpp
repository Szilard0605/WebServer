
#include "Server.h"
#include <windows.h>

#include <time.h>

#define SERVER_TICK_RATE 3

int main(int argc, char* argv[])
{
	WebServer Server;

	if (Server.Start("192.168.0.177", 80))
	{
		printf("Server started\n");
	}

	Server.Update();
	clock_t lastUpdateTime = clock();


	while (Server.ShouldRun())
	{
		clock_t now = clock();
		if (now - lastUpdateTime >= SERVER_TICK_RATE)
		{
			Server.Update();
			lastUpdateTime = now;
		}


		if (GetForegroundWindow() == GetConsoleWindow())
		{
			if (GetAsyncKeyState(VK_F1))
			{
				Server.Shutdown();
			}
		}
	}

}