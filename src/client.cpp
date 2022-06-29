#include "client.h"
#include "libs/asio/include/boost/asio/read.hpp"
#include <core/network.h>
#include <core/event.h>
#include <boost/bind.hpp>

namespace mirage::network::client
{
	void Client::handlePacketRaw(const AbstractPacket& packet)
	{
		logi("received packet, c {}, id {}", packet.packet->constant, packet.packet->id);
		if(packet.packet->constant != packetConstant)
			return;
		switch(packet.packet->id)
		{
			case PacketId::connect:
			{
				const auto cr = packetCast<ConnectionResponce>(packet);
				if(cr.responce == ConnectionResponce::success)
					event::enqueueEvent<ClientConnected>();
			}
				break;
			case PacketId::message:
				event::enqueueEvent<PacketReceivedEvent<MessageSent>>
					(packetCast<MessageSent>(packet));
				break;
			case PacketId::graphicFrame:
				event::enqueueEvent<PacketReceivedEvent<GraphicFrame>>
					(packetCast<GraphicFrame>(packet));
				break;
			case PacketId::resource:
				event::enqueueEvent<PacketReceivedEvent<ResourceUpdate>>
					(packetCast<ResourceUpdate>(packet));
				break;
			default:
			{
				break;
			}
		}
	}
	void Client::handleReceiveFrom(
			const boost::system::error_code& ec,
			size_t size)
	{	
		handlePacketRaw(AbstractPacket(&buffer.packet, size));
		startReceive();
	}

	Client::Client(std::string sv)
		: username {std::move(sv)}, 
		  socket(ioContext(),
			boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)),
		  socketTcp{ioContext()},
		  buffer{},
		  tcpBuffer{}
	{
	}
	
	Client::~Client(void) {}

	void Client::connect(boost::asio::ip::udp::endpoint con)
	{
		connected = std::move(con);

		InitializeConnection incon;
		strcpy(incon.username, username.c_str());
		
		send(AbstractPacket(incon));
	}

	void Client::connectTcp(boost::asio::ip::tcp::endpoint con)
	{
		socketTcp.connect(con);
		connectedTcp = std::move(con);		
		
	}

	void Client::send(const boost::asio::const_buffer& buffer)
	{
		socket.send_to(buffer, connected);
	}

	void Client::sendTcp(const boost::asio::const_buffer& buffer)
	{
		socketTcp.send(buffer);
	}

	void Client::startReceive(void)
	{
		socket.async_receive_from(
			boost::asio::buffer(buffer.data), endpoint,
			boost::bind(&Client::handleReceiveFrom,
				this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void Client::handleTcpReceiveFrom(	
			const boost::system::error_code& ec, 
			size_t size)
	{
		if ((ec == boost::asio::error::eof) || (ec == boost::asio::error::connection_reset))
			return; // FIXME: add disconnect handling
		handlePacketRaw(AbstractPacket(&tcpBuffer.packet, size));
		startReceiveTcp();
	}

	void Client::startReceiveTcp(void)
	{
		socketTcp.async_receive(
			boost::asio::buffer(tcpBuffer.data),
			boost::bind(&Client::handleTcpReceiveFrom,
				this,	
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));

	}

	void Client::start(void)
	{
		startReceive();
		startReceiveTcp();
	}
}