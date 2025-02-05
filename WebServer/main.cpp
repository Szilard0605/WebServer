
#include "Server.h"
#include <windows.h>

#include <time.h>

#define SERVER_TICK_RATE 3

int main(int argc, char* argv[])
{
	WebServer Server;


	// teszt pages
	Server.SetHomePageSource("html/teszt/index.html");

	Server.LinkRequestToFile("teszt", { "html/teszt/teszt.html", "text/html" });
	Server.LinkRequestToFile("homepage", { "html/index.html", "text/html" });
	Server.LinkRequestToFile("tesztstyle.css", { "html/teszt/tesztstyle.css", "text/css"});
	Server.LinkRequestToFile("script.js", {"html/teszt/script.js", "text/javascript"});
	Server.LinkRequestToFile("p5teszt", { "html/teszt/p5teszt.html", "text/html" });
	Server.LinkRequestToFile("script.js", { "html/teszt/script.js", "text/javascript" });

	if (Server.Start("192.168.0.21", 3000))
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