
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <asio.hpp>

#ifdef DEBUG

constexpr bool SERVER_DEBUG = true;

#elif

constexpr bool SERVER_DEBUG = false;

#endif