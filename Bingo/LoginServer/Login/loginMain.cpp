#include <iostream>
#include <winsock2.h>

#include "Server.h"
#include "LoginProcessor.h"
#include "luaHelper.h"
#include <thread>

#pragma comment(lib, "ws2_32.lib")

int main()
{
	LuaHelper luahelper;
	luahelper.InitLua();

	luahelper.ReadScript(SETTING_FILE_PATH);

	std::cout << "Login Server Start\n";

	std::shared_ptr<Server> server = std::make_shared<Server>(luahelper.Get<short>("myPort"));
	std::shared_ptr<IProcessor> processor = std::make_shared<LoginProcessor>(&luahelper, server);
	server->SetProcessor(processor);
	processor->Init();

	std::thread networkThread([&server]() 
		{
			server->Run();
		});

	processor->ConnectServer();
	while (true)
	{
		processor->Process();
	}

	return 0;
}