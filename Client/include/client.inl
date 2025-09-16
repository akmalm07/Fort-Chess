#pragma once
#include "client.h"



namespace client
{
	template<typename T>
	inline void Client::send(const T& data)
	{
		const uint8_t* buffer = reinterpret_cast<const uint8_t*>(&data);
		size_t size = sizeof(T);
		try
		{
			asio::write(_socket, asio::buffer(buffer, size));
		}
		catch (const asio::system_error& e)
		{
			std::cerr << "Send failed: " << e.what() << std::endl;
			stop(); // Stop the client if sending fails
		}
	}

	template<>
	inline void Client::send<std::string>(const std::string& data)
	{
		try
		{
			asio::write(_socket, asio::buffer(data.data(), data.size()));
		}
		catch (const asio::system_error& e)
		{
			std::cerr << "Send failed: " << e.what() << std::endl;
			stop();
		}
	}
}