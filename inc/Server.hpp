#ifndef SERVER_HPP
#define SERVER_HPP

#include "../inc/Client.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <csignal>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <stdexcept>

class Client;
class Channel;

extern volatile sig_atomic_t g_running;

class Server
{
	public:
		Server(int port, const std::string &password);
		~Server();
		void run();

	private:
		int									_serverFd;
		std::vector<struct pollfd>			_pollfds;
		std::map<int, Client*>				_clients;
		int									_port;
		std::string							_password;
		std::map<std::string, Channel*>		_channels;

		void		acceptClient();
		void		removeClient(int fd);
		static void	setNonBlocking(int fd);
		void	handleClient(int fd);
		void	parseMessage(Client *client, const std::string &line);

		Server(const Server &);
		Server &operator=(const Server &);
};

#endif