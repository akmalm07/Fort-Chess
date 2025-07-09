#pragma once

#include "server.h"

namespace server
{
	template<typename T>
	inline void Server::set_on_message_with_trigger(std::function<void()> callback, const T& comparison)
	{
		const uint8_t* data = reinterpret_cast<const uint8_t*>(&comparison);

		_onMessageWithTrigger.emplace_back(callback, data, sizeof(T));
	}
	template<typename T>
	inline void Server::set_on_message_starting_with(std::function<void()> callback, const T& comparison)
	{
		const uint8_t* data = reinterpret_cast<const uint8_t*>(&comparison);
		_onMessageStartingWith.emplace_back(callback, data, sizeof(T));
	}
}