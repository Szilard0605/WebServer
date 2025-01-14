
#include "Server.h"
#include "FamPong/FamPong.h"

#include <windows.h>

#include <time.h>

#define SERVER_TICK_RATE 3

int main(int argc, char* argv[])
{
	WebServer Server;


	// teszt pages
	Server.SetHomePageSource("html/teszt/index.html");
	Server.LinkRequestToFile("homepage", { "html/teszt/index.html", "text/html" });
	Server.LinkRequestToFile("teszt", { "html/teszt/teszt.html", "text/html" });
	Server.LinkRequestToFile("tesztstyle.css", { "html/teszt/tesztstyle.css", "text/css"});
	Server.LinkRequestToFile("script.js", {"html/teszt/script.js", "text/javascript"});

	FamPong::Init();

	if (Server.Start("192.168.0.19", 7000))
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