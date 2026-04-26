#include "../inc/Server.hpp"

volatile sig_atomic_t g_running = 1;

static void signalHandler(int sig)
{
	(void)sig;
	g_running = 0;
	std::cout << "\nShutting down..." << std::endl;
}

void Server::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("fcntl F_GETFL failed");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("fctl F_SETFL failed");
}

Server::Server(int port, const std::string &password) : _port(port), _password(password)
{
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0)
		throw std::runtime_error("Failed to create socket");

	int opt = 1;
	setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(_serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Failed to bind socket");
	if (listen(_serverFd, SOMAXCONN) < 0)
		throw std::runtime_error("Failed to listen socket");

	setNonBlocking(_serverFd);

	struct pollfd p;
	p.fd      = _serverFd;
	p.events  = POLLIN;
	p.revents = 0;
	_pollfds.push_back(p);

	std::cout << "Server listening on port " << port << std::endl;
}

Server::~Server()
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		delete it->second;
	_clients.clear();
	close(_serverFd);
}

void Server::run()
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	std::cout << "Server starting main event loop..." << std::endl;

	while (g_running)
	{
		int ret = poll(&_pollfds[0], _pollfds.size(), -1);
		if (ret < 0)
		{
			if (!g_running)
				break;
			std::cerr << "poll() error" << std::endl;
			break;
		}

		for (size_t i = 0; i < _pollfds.size(); i++)
		{
			if (_pollfds[i].revents == 0)
				continue;

			int fd = _pollfds[i].fd;

			if (fd == _serverFd)
				acceptClient();

			else
				handleClient(fd);
		}
	}
}

void Server::acceptClient()
{
	int client_fd = accept(_serverFd, NULL, NULL);
	if (client_fd < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "accept() failed" << std::endl;
		return;
	}

	setNonBlocking(client_fd);

	Client *client = new Client(client_fd);
	_clients[client_fd] = client;

	struct pollfd p;
	p.fd = client_fd;
	p.events = POLLIN;
	p.revents = 0;
	_pollfds.push_back(p);

	std::cout << "New client connected: fd " << client_fd << std::endl;
}

void Server::removeClient(int fd)
{
	for (size_t i = 0; i < _pollfds.size(); i++)
	{
		if (_pollfds[i].fd == fd)
		{
			_pollfds.erase(_pollfds.begin() + i);
			break;
		}
	}

	for (std::map<std::string, Channel>::iterator ch = _channels.begin(); ch != _channels.end(); ++ch)
		ch->second.removeClient(fd);

	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		delete it->second;
		_clients.erase(it);
	}

	std::cout << "Client disconnected: fd " << fd << std::endl;
}

void Server::handleClient(int fd)
{
	char buf[512];
	int n = recv(fd, buf, sizeof(buf) - 1, 0);

	if (n <= 0 )
	{
		removeClient(fd);
		return;
	}

	Client *client = _clients[fd];
	client->_buffer.append(buf, n);

	size_t pos;
	while ((pos = client->_buffer.find("\r\n")) != std::string::npos)
	{
		std::string line = client->_buffer.substr(0, pos);
		client->_buffer.erase(0, pos + 2);
		if (!line.empty())
			processMessage(fd, line);
	}
}

void parseMessage(std::vector<std::string> &tok, std::string &line)
{
	std::string trailing; // Si ":" -> un seul argument

	size_t pos = line.find(" :");
	if (pos != std::string::npos)
	{
		trailing = line.substr(pos + 2);
		line = line.substr(0, pos);
	}

	std::istringstream iss(line);
	std::string word;
	while (iss >> word)
		tok.push_back(word);
	if (!trailing.empty())
		tok.push_back(trailing);
}

void execCommand(const std::string cmd, int fd, std::vector<std::string> &tokens)
{
	(void)cmd;
	(void)fd;
	(void)tokens;
	// if (cmd == "PASS")
	// 	handlePass(fd, tokens);
	// else if (cmd == "NICK")
	// 	handleNick(fd, tokens);
	// else if (cmd == "USER")
	// 	handleUser(fd, tokens);
	// else if (cmd == "PRIVMSG")
	// 	handlePrivmsg(fd, tokens);
	// else if (cmd == "JOIN")
	// 	handleJoin(fd, tokens);
	// else if (cmd == "QUIT")
	// 	handleQuit(fd, tokens);
	// else
	// 	send(fd, "421 Unknown command\r\n", 21, 0);
}

void Server::processMessage(int fd, const std::string &msg)
{
	std::cout << "Received from " << fd << ": " << msg << std::endl << std::flush;

	std::string line = msg;
	if (!line.empty() && line[line.size() - 1] == '\n')
		line.erase(line.size() - 1);
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	std::vector<std::string> tokens;
	parseMessage(tokens, line);
	if (tokens.empty())
		return ;
	std::string command = tokens[0];
	execCommand(command, fd, tokens);
}