#include <asio.hpp>
#include "tcp.hpp"
#include "server.hpp"

int main(int argc, void* argv) {
	asio::io_service io_service;
	tcp_server server(io_service);

	std::list<chat_server> servers;

	tcp::endpoint endpoint(tcp::v4(), 12345);
	servers.emplace_back(io_service, endpoint);

	io_service.run();
	return 0;
}
