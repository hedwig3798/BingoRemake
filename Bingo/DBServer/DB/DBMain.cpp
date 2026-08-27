#include <iostream>
#include <winsock2.h>

#include "Server.h"
#include "DBProcessor.h"
#include "luaHelper.h"
#include <thread>

#pragma comment(lib, "ws2_32.lib")

int main()
{
	LuaHelper luahelper;
	luahelper.InitLua();

	luahelper.ReadScript(SETTING_FILE_PATH);

	std::cout << "DB Server Start\n";

	Server server(luahelper.Get<short>("myPort"));

	std::shared_ptr<IProcessor> processor = std::make_shared<DBProcessor>(&luahelper);
	processor->Init();
	server.SetProcessor(processor);

	std::thread networkThread([&server]() 
		{
			server.Run();
		});



	while (true)
	{
		processor->Process();
	}

	return 0;
}