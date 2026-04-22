#include "../inc/Channel.hpp"

Channel::Channel() {}
Channel::Channel(const std::string &name) : _name(name) {}
Channel::~Channel() {}

const std::set<int> &Channel::getClientsFd() const { return (_clients_fd); }

void Channel::addClient(int fd)
{
	_clients_fd.insert(fd);
}

void Channel::removeClient(int fd)
{
	_clients_fd.erase(fd);
}