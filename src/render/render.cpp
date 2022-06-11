#include "render.h"
#include "core/network.h"
#include "core/packet.h"
#include "core/processing.h"
#include <algorithm>
#include <cstring>
#include <ranges>

void mirage::client::IconCache::RequestProcess::update(unsigned delta, void*)
{
	std::lock_guard guard{IconCache::mutex};
	auto& cache = IconCache::getInstance();

	network::ResourceRequest::SerializedT queue{cache.resourceRequestQueue};

	cache.resourceUpdateQueue.insert(cache.resourceUpdateQueue.end(),
		std::make_move_iterator(cache.resourceRequestQueue.begin()),
		std::make_move_iterator(cache.resourceRequestQueue.end()));
	cache.resourceRequestQueue.clear();

	auto serialized = utils::serialize(queue);
	
	assert(serialized.size() < sizeof(network::ResourceRequest::serialized));

	network::ResourceRequest rq;

	strcpy(rq.serialized, serialized.c_str());

	network::client::client().send(network::AbstractPacket{rq});
}

void mirage::client::IconCache::onResourceUpdate(network::client::PacketReceivedEvent<network::ResourceUpdate>& pre)
{
	std::lock_guard guard{mutex};

	auto deserialized = utils::deserialize<network::ResourceUpdate::SerializedT>(
		std::string(utils::stringView(pre.packet.serialized, sizeof(pre.packet.serialized))));
	
	
	for(auto&& resource : deserialized)
	{
		std::ranges::remove_copy(resourceUpdateQueue, resourceUpdateQueue.begin(), resource.id);
		if(resources.contains(resource.id))
		{
			auto&& oldResource = resources[resource.id];
			SDL_FreeSurface(oldResource.surface);
			if(textures.contains(resource.id))
				SDL_DestroyTexture(textures[resource.id]);
		}
		textures[resource.id] = SDL_CreateTextureFromSurface(mainWindow().renderer, resource.surface);
		resources.insert_or_assign(resource.id, std::move(resource));
	}
}

void mirage::client::IconCache::initialize(void)
{
	startProcess<RequestProcess>(ecs::processing::PeriodMS<200>::getInstance());
}

void mirage::client::IconCache::lateInitialize(void)
{
	bindEvent<network::client::PacketReceivedEvent<network::ResourceUpdate>, &IconCache::onResourceUpdate>();
}

void mirage::client::GameRenderer::render(MainWindowUpdateEvent&)
{
	std::lock_guard guard{IconCache::mutex};

	SDL_SetRenderDrawColor(mainWindow().renderer, 255, 0, 0, 255);

	const auto w = mainWindow().width;
	const auto h = mainWindow().height;

	for(auto&& verticeGroup : frame)
	{
		/*
		 * TODO: rewrite for unseq iterating
		 */
		for(auto&& vertice : verticeGroup.vertices)
		{
			SDL_Rect rect
				{
					static_cast<int>((static_cast<float>(vertice.x) / UINT16_MAX) * w), 
					static_cast<int>((static_cast<float>(vertice.y) / UINT16_MAX) * h), 
					64, 64
				};

			SDL_RenderCopy(mainWindow().renderer, IconCache::getInstance().textures[vertice.icon], nullptr, &rect);
		}
	}
}

void mirage::client::GameRenderer::onFrame(mirage::network::client::PacketReceivedEvent<network::GraphicFrame>& packet)
{
	auto nFrame = utils::deserialize<decltype(frame)>
		(std::string{utils::stringView(packet.packet.serialized, std::size(packet.packet.serialized))});
	{
		std::lock_guard guard{IconCache::mutex};
		for(auto&& verticeGroup : nFrame)
			for(auto&& vertice : verticeGroup.vertices)
				if(!IconCache::getInstance().resources.contains(vertice.icon))
					IconCache::getInstance().resourceRequestQueue.push_back(vertice.icon);
	}
	
	if(IconCache::getInstance().resourceRequestQueue.empty() &&
	   IconCache::getInstance().resourceUpdateQueue.empty())
		frame = std::move(nFrame);
}

void mirage::client::GameRenderer::initialize(void) {}
void mirage::client::GameRenderer::lateInitialize(void)
{
	bindEvent<MainWindowUpdateEvent, &GameRenderer::render>();
	bindEvent<network::client::PacketReceivedEvent<network::GraphicFrame>, &GameRenderer::onFrame>();
}
