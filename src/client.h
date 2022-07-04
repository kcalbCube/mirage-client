#pragma once
#include <core/network.h>
namespace mirage::network::client
{

	template<size_t Size>
	union Buffer
	{
		PacketVoid packet;
		uint8_t data[Size];
	};

	class Client
	{
		std::string username;
		boost::asio::ip::udp::endpoint connected, endpoint;	
		boost::asio::ip::udp::socket socket;

		boost::asio::ip::tcp::endpoint connectedTcp;
		boost::asio::ip::tcp::socket socketTcp;	

		Buffer<maxPacketSize> buffer;
		Buffer<maxTcpPacketSize> tcpBuffer;

		void handleReceiveFrom(
				const boost::system::error_code&,
				size_t);
		void handleTcpReceiveFrom(
				const boost::system::error_code&,
				size_t);
		void handlePacketRaw(const AbstractPacket&);
	public:	

		Client(std::string);
		Client(Client&&);
		Client(const Client&) = delete;
		~Client(void);

		const boost::asio::ip::udp::endpoint& getConnected(void) const;

		void connect(boost::asio::ip::udp::endpoint);
		void connectTcp(boost::asio::ip::tcp::endpoint);
	
		void disconnect(void);

		void send(const boost::asio::const_buffer&);
		void sendTcp(const boost::asio::const_buffer&);

		void startReceive(void);
		void startReceiveTcp(void);

		void start(void);

		void sendInfo(void);
	};
	
	template<typename T>
	struct PacketReceivedEvent	
	{
		T packet;
	};


	inline Client& client(void)
	{
		static Client client("kcalbCube");
		return client;
	}

	struct ClientConnected {};
}
