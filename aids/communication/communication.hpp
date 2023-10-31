#pragma once
#include <ws2tcpip.h>
#include <string>
#include <stack>
#include <shared_mutex>

// Communication is drived through a websocket

static const char* port = "42069";
static const std::size_t buffer_size = 0x4000;

class communication_t
{
private:
	SOCKET server_socket = INVALID_SOCKET;
	SOCKET client_socket = INVALID_SOCKET;
public:
	bool setup();
	void receive_data();
	void finish();

	std::mutex pending_scripts_mutex{};
	std::stack<std::string> pending_scripts{};
} static communication;