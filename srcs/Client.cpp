#include "../inc/Client.hpp"

Client::Client() {}
Client::Client(int fd) : _fd(fd) {}
Client::~Client() { closeConnection(); }

void Client::setNickname(const std::string &nickname) { _nickname = nickname; }
void Client::setUsername(const std::string &username) { _username = username; }
void Client::setRealname(const std::string &realname) { _realname = realname; }
void Client::setPassOk(const bool boolean) { _passOk = boolean; }
void Client::setRegistered(const bool boolean) { _registered = boolean; }
void Client::setNickSet(const bool boolean) { _nickSet = boolean; }
void Client::setUserSet(const bool boolean) { _userSet = boolean; }

int Client::getFd() const { return (_fd); }

std::string Client::getNickname() const { return (_nickname); }
std::string Client::getUsername() const { return (_username); }
std::string Client::getRealname() const { return (_realname); }
bool Client::isPassOk() const { return (_passOk); }
bool Client::isRegistered() const { return (_registered); }
bool Client::isNickSet() const { return (_nickSet); }
bool Client::isUserSet() const { return (_userSet); }

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