#include "render.h"
#include "core/network.h"
#include "core/packet.h"
#include "core/processing.h"
#include <algorithm>
#include <cstring>
#include <ranges>
#include <SDL.h>

void mirage::client::IconCache::RequestProcess::update(float)
{
	std::lock_guard guard{IconCache::mutex};
	auto& cache = parent.get();

	if(cache.resourceRequestQueue.empty())
		return;

	network::ResourceRequest::SerializedT queue{std::begin(cache.resourceRequestQueue), std::end(cache.resourceRequestQueue)};

	cache.resourceUpdateQueue = cache.resourceRequestQueue;
	cache.resourceRequestQueue.clear();

	auto serialized = utils::serialize(queue);
	
	assert(serialized.size() < sizeof(network::ResourceRequest::serialized));

	network::ResourceRequest rq;

	memcpy(rq.serialized, serialized.data(), serialized.size());

	network::client::client().send(network::AbstractPacket{rq});
}

void mirage::client::IconCache::onResourceUpdate(network::client::PacketReceivedEvent<network::ResourceUpdate>& pre)
{
	std::lock_guard guard{mutex};

	auto deserialized = utils::deserialize<network::ResourceUpdate::SerializedT>(
		std::string(std::string_view(pre.packet.serialized, sizeof(pre.packet.serialized))));
		
	for(auto&& resource : deserialized)
	{	
		resourceUpdateQueue.erase(resource.id);
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

	auto w = mainWindow().width;
	auto h = mainWindow().height;

	for(auto&& verticeGroup : frame)
	{
		graphics::Scale scale{1.f, 1.f};

		for(auto&& filter : verticeGroup.filters)
		{	
			if(graphics::Scale* scale_ = boost::get<graphics::Scale>(&filter))
			{
				scale.first *= scale_->first;
				scale.second*= scale_->second;
			}
		}

		for(auto&& vertice : verticeGroup.vertices)
		{
			auto texture = IconCache::getInstance().textures[vertice.icon];

			auto rawX = static_cast<int>(vertice.x); 
			auto rawY = static_cast<int>(vertice.y);

			int tW, tH;
			SDL_QueryTexture(texture, nullptr, nullptr, &tW, &tH); // TODO: move this to texture loader.

			SDL_Rect rect
				{
					(rawX + tW / 2) - static_cast<int>(tW * scale.first),
					(rawY + tH / 2) - static_cast<int>(tH * scale.second),
					static_cast<int>(tW * scale.first),
					static_cast<int>(tH * scale.second)
				};

			SDL_RenderCopy(mainWindow().renderer, texture, nullptr, &rect);
		}
	}
}

void mirage::client::GameRenderer::onFrame(mirage::network::client::PacketReceivedEvent<network::GraphicFrame>& packet)
{
	auto nFrame = utils::deserialize<decltype(frame)>
		(std::string{std::string_view(packet.packet.serialized, std::size(packet.packet.serialized))});
	{
		std::lock_guard guard{IconCache::mutex};
		for(auto&& verticeGroup : nFrame)
			for(auto&& vertice : verticeGroup.vertices)
				if(!IconCache::getInstance().resources.contains(vertice.icon))
					IconCache::getInstance().resourceRequestQueue.insert(vertice.icon);
	}

	// TODO: rewrite layers
	for(auto&& vg : nFrame)
		std::sort(std::begin(vg.vertices), std::end(vg.vertices), 
			[](auto& a, auto& b) -> bool
			{
				return a.layer < b.layer;
			});
	
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
