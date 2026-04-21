#include "../inc/Client.hpp"

Client::Client() {}
Client::Client(int fd) : _fd(fd) {}
Client::~Client() {}

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
