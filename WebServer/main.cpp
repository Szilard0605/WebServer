
#include "Server.h"
#include <time.h>

#define SERVER_TICK_RATE 3

int main()
{
	WebServer Server;

	if (Server.Start(80))
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
	}

}
