#include "client.h"
#include <iostream>
#include <core/event.h>
#include <thread>
#include "render/window.h"
#include "render/render.h"
#include <imgui.h>

using namespace mirage::network::client;
using namespace mirage::network;

struct MessageWindow : mirage::ecs::Component<MessageWindow>
{
	std::vector<std::string> messages;
	char buffer[20]{};

	void windowUpdate(mirage::client::MainWindowUpdateEvent&)
	{
		ImGui::Begin("Chat");
		ImGui::InputText("<", buffer, std::size(buffer));
		ImGui::SameLine();
		if(ImGui::Button("Send"))
		{
			mirage::network::MessageSent msg;
			memcpy(msg.message, buffer, std::size(buffer));
			mirage::network::client::client().send(
				mirage::network::AbstractPacket(msg));
		}
		for(auto&& message : messages)
			ImGui::TextUnformatted(message.c_str());
		ImGui::End();
	}

	void initialize(void)
	{
		bindEvent<mirage::client::MainWindowUpdateEvent, &MessageWindow::windowUpdate>();
		bindEvent<PacketReceivedEvent<MessageSent>, &MessageWindow::onMessage>();
	}

	void onMessage(PacketReceivedEvent<MessageSent>& packet)
	{	
		messages.emplace_back(std::string_view(packet.packet.message, packet.packet.messageMax));
	}
};
MIRAGE_CREATE_ON_STARTUP(MessageWindow, msgWindow);
#undef main 
int main(int, char**)
{
	fmtlog::setLogLevel(fmtlog::DBG);
 	fmtlog::startPollingThread();		
	mirage::network::client::client().start();
	mirage::network::client::client().connect(mirage::network::fromString("127.0.0.1", 5000));	
	
	std::thread thr([](void) -> void
	{	
		mirage::client::mainWindow().initialize(600, 300);

		while(true)
		{
			mirage::client::mainWindow().render();
			mirage::client::mainWindow().handleEvents();
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
		}
	});

	thr.detach();

	mirage::ioContext().run();
	while(true) ;
	return 0;
}
