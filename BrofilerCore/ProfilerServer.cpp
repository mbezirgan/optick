#include "Common.h"
#include "ProfilerServer.h"

#include "Socket.h"
#include "Message.h"

#if defined(BRO_MSVC)
#pragma comment( lib, "ws2_32.lib" )
#endif

namespace Brofiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const short DEFAULT_PORT = 31313;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Server::Server(short port) : socket(Memory::New<Socket>()), isInitialized(false)
{
	socket->Bind(port, 4);
	socket->Listen();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Server::Update()
{
	std::lock_guard<std::recursive_mutex> lock(socketLock);

	if (!InitConnection())
		return;

	int length = -1;
	while ( (length = socket->Receive( buffer, BIFFER_SIZE ) ) > 0 )
	{
		networkStream.Append(buffer, length);
	}

	while (IMessage *message = IMessage::Create(networkStream))
	{
		message->Apply();
		Memory::Delete(message);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Server::Send(DataResponse::Type type, OutputDataStream& stream)
{
	std::lock_guard<std::recursive_mutex> lock(socketLock);

	std::string data = stream.GetData();

	DataResponse response(type, (uint32)data.size());
	socket->Send((char*)&response, sizeof(response));
	socket->Send(data.c_str(), data.size());
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Server::InitConnection()
{
	return socket->Accept();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Server::~Server()
{
	if (socket)
	{
		Memory::Delete(socket);
		socket = nullptr;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Server & Server::Get()
{
	static Server instance(DEFAULT_PORT);
	return instance;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
