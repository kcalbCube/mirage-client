#pragma once
#include <SDL.h>
#include <core/graphics.h>
#include <core/network.h>
#include "window.h"
#include <core/event.h>
#include <core/ecs.h>
#include <vector>
#include "../client.h"
#include <deque>
#include <set>
#include <core/processing.h>

namespace mirage::client
{
	struct IconCache
		: public ecs::Component<IconCache>,
		  public ecs::Singleton<IconCache>,
		  public ecs::Singleton<IconCache>::Lockable,
		  public ecs::Processing<IconCache>
	{
		struct RequestProcess : Process<RequestProcess>
		{
			void update(float delta);
		};

		static constexpr unsigned requestPeriod = 100;
		std::set<graphics::Icon> 	    
			resourceRequestQueue,
			resourceUpdateQueue;
		std::map<graphics::Icon, graphics::IconResource> resources;
		std::map<graphics::Icon, SDL_Texture*> textures;

		void onResourceUpdate(network::client::PacketReceivedEvent<network::ResourceUpdate>&);
	
		void initialize(void);
		void lateInitialize(void);
	};

	class GameRenderer
		: public ecs::Component<GameRenderer>
	{
	public:
		std::vector<graphics::VerticeGroup> frame;
		void render(MainWindowUpdateEvent&);

		void onFrame(
			network::client::PacketReceivedEvent<network::GraphicFrame>&);	

		void initialize(void);
		void lateInitialize(void);
	};
	MIRAGE_CREATE_ON_STARTUP(GameRenderer, gameRenderer);
}
