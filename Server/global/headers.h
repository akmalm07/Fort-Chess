
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <asio.hpp>

#ifdef DEBUG

constexpr bool SERVER_DEBUG = true;

#else

constexpr bool SERVER_DEBUG = false;

#endif