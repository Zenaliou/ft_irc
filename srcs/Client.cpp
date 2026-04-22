#include "../inc/Client.hpp"

Client::Client() {}
Client::Client(int fd) : _fd(fd) {}
Client::~Client() { closeConnection(); }

void Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string &username)
{
	_username = username;
}

int Client::getFd() const { return (_fd); }

std::string Client::getNickname() const { return (_nickname); }
std::string Client::getUsername() const { return (_username); }

void Client::closeConnection()
{
	if (_fd >= 0)
	{
		close(_fd);
		_fd = -1;
	}
}

void Client::send_msg(const std::string &msg)
{
	std::string out = msg + "\r\n";
	send(_fd, out.c_str(), out.size(), 0);
}