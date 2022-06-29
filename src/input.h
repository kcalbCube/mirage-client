#pragma once
#include <core/processing.h>
#include <core/input.h>
#include "core/static.h"
#include "render/window.h"
#include "client.h"

namespace mirage::client
{
	struct InputProcessor
		: ecs::Component<InputProcessor>,
		  ecs::Processing<InputProcessor>
	{
		struct InputProcess : Process<InputProcess>
		{
			void update(float);
		};

		std::set<SDL_Scancode> pressed;

		void onEventUpdate(EventUpdateEvent&);
		void initialize(void);
		void lateInitialize(void);
	};

	MIRAGE_CREATE_ON_EVENT(network::client::ClientConnected, InputProcessor);
}