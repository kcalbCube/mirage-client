#include "input.h"
#include "SDL_events.h"
#include "core/network.h"
#include "core/packet.h"
#include "core/utility.h"
#include <SDL.h>

void mirage::client::InputProcessor::InputProcess::update(float)
{
	network::Input input;
	auto serialized = utils::serialize(parent->pressed);
	memcpy(input.serialized, serialized.data(), serialized.size());
	network::client::client().send(network::AbstractPacket{input});
}

void mirage::client::InputProcessor::onEventUpdate(EventUpdateEvent& evev)
{
	auto& event = *evev.event;

	switch(event.type)
	{
	case SDL_KEYDOWN:
		pressed.insert(event.key.keysym.scancode);
		break;
	case SDL_KEYUP:
		pressed.erase(event.key.keysym.scancode);
		break;
	case SDL_QUIT:
		SDL_Quit();
		abort(); // TODO: add properly exit
	}
}

void mirage::client::InputProcessor::initialize(void)
{
	startProcess<InputProcess>(ecs::processing::TickProcessor<std::ratio<1,1>>::getInstance());
}

void mirage::client::InputProcessor::lateInitialize(void)
{
	bindEvent<EventUpdateEvent, &InputProcessor::onEventUpdate>();
}