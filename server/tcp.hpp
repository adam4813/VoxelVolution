#include <ctime>
#include <iostream>
#include <string>
#include <asio.hpp>
#include <functional>

using asio::ip::tcp;

std::string make_daytime_string() {
	using namespace std; // For time_t, time and ctime;
	time_t now = time(0);
	return ctime(&now);
}

class tcp_connection
	: public std::enable_shared_from_this < tcp_connection > {
public:
	typedef std::shared_ptr<tcp_connection> pointer;

	static pointer create(asio::io_service& io_service) {
		return pointer(new tcp_connection(io_service));
	}

	tcp::socket& socket() {
		return socket_;
	}

	void start() {
		message_ = make_daytime_string();

		asio::async_write(socket_, asio::buffer(message_),
			std::bind(&tcp_connection::handle_write, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2));
	}

private:
	tcp_connection(asio::io_service& io_service)
		: socket_(io_service) { }

	void handle_write(const asio::error_code& /*error*/,
		size_t /*bytes_transferred*/) { }

	tcp::socket socket_;
	std::string message_;
};

class tcp_server {
public:
	tcp_server(asio::io_service& io_service)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), 13)) {
		start_accept();
	}

private:
	void start_accept() {
		tcp_connection::pointer new_connection = tcp_connection::create(acceptor_.get_io_service());

		acceptor_.async_accept(new_connection->socket(),
			std::bind(&tcp_server::handle_accept, this, new_connection,
			std::placeholders::_1));
	}

	void handle_accept(tcp_connection::pointer new_connection,
		const asio::error_code& error) {
		if (!error) {
			std::cout << "Connected" << std::endl;
			new_connection->start();
		}

		start_accept();
	}

	tcp::acceptor acceptor_;
};
