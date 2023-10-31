#include "communication.hpp"

bool cleanup(bool status, addrinfo* info, SOCKET server_socket = NULL)
{
	if (!status)
	{
		int code = WSAGetLastError();
		if (server_socket)
			closesocket(server_socket);
		WSACleanup();
		std::printf("Failed to establish communication!\n");
		std::printf("Error code: %d\n", code);
	}
	else
		std::printf("Communication established!\n");

	freeaddrinfo(info);
	return status;
}

bool communication_t::setup()
{
	WSADATA wsa_data{ 0 };

	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
	{
		std::printf("Failed to establish communication!\n");
		return false;
	}

	addrinfo* result = nullptr, hints{ 0 };
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, port, &hints, &result))
		return cleanup(false, result);

	if (server_socket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol); server_socket == INVALID_SOCKET)
		return cleanup(false, result);

	if (bind(server_socket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR)
		return cleanup(false, result, server_socket);

	if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
		return cleanup(false, result, server_socket);

	return cleanup(true, result);
}

void communication_t::receive_data()
{
	if (server_socket == INVALID_SOCKET)
	{
		std::printf("Invalid server socket!\n");
		return;
	}

	while (true)
	{
		if (client_socket = accept(server_socket, NULL, NULL); client_socket == INVALID_SOCKET)
		{
			std::printf("Invalid client!\n");
			return;
		}

		std::string script{};
		{
			std::string buffer{};
			buffer.resize(buffer_size, '\0');

			while (std::size_t received = recv(client_socket, &buffer[0], buffer.size(), 0))
			{
				// Socket disconnected
				if (!received)
					break;

				script += std::string{ buffer.c_str(), received };
				buffer.resize(buffer_size, '\0');

				// Finished sending data since it couldn't fill the buffer
				if (received < buffer_size)
					break;
			}

			shutdown(client_socket, SD_BOTH);
			closesocket(client_socket);
		}

		std::lock_guard my_guard{ pending_scripts_mutex };
		pending_scripts.push(script);
	}

	return;
}

void communication_t::finish()
{
	closesocket(server_socket);
	WSACleanup();
}