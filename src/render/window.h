#pragma once
#include <core/utility.h>
#include <core/ecs.h>
#include <SDL.h>

namespace mirage::client
{
	class MainWindow
	{
	public:
		unsigned 
			width = 0,
			height = 0;
		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;

		void render(void);
	
		void initialize(unsigned w, unsigned h);	
		void deinitialize(void);

		void handleEvents(void);

		~MainWindow(void) { deinitialize(); }
	};

	MIRAGE_COFU(MainWindow, mainWindow);

	struct MainWindowUpdateEvent {};
	struct MainWindowRenderEvent {};
	struct EventUpdateEvent
	{
		SDL_Event* event;
	};
}
