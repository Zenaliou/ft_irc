#include "../inc/Server.hpp"

/**
 * PASS
 * Verifie que le client connait le mdp
 * - mauvais mdp -> rejet
 * - deja enregistre -> erreur
 * - pas assez d'arg -> erreur
 */
void Server::handlePass(int fd, const std::vector<std::string> &args)
{
	Client	*client = _clients[fd];

	if (client->isRegistered())
	{
		sendError(fd, "462", "Already registered");
		return ;
	}

	if (args.size() < 2)
	{
		sendError(fd, "461", "PASS :Not enough parameters");
		return ;
	}

	std::string password = args[1];
	if (password != _password)
	{
		sendError(fd, "464", "Password incorrect");
		return ;
	}

	client->setPassOk(true);
	std::cout << "Client " << fd << " passed authentication" << std::endl;
}

void Server::handleNick(int fd, const std::vector<std::string> &args)
{
	Client	*client = _clients[fd];

	if (args.size() < 2)
	{
		sendError(fd, "431", "No nickname given");
		return ;
	}

	std::string newNickname = args[1];
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == newNickname)
		{
			sendError(fd, "433", "Nickname is already in use");
			return ;
		}
	}

	std::string oldNickname = client->getNickname();

	client->setNickname(newNickname);
	client->setNickSet(true);

	if (client->isRegistered())
	{
		std::string msg = ":" + oldNickname + " NICK :" + newNickname + "\r\n";
		for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
			send(it->first, msg.c_str(), msg.size(), 0);
	}

	tryRegister(client, fd);
}

void Server::handleUser(int fd, const std::vector<std::string> &args)
{
	Client	*client = _clients[fd];

	if (client->isRegistered())
	{
		sendError(fd, "462", "Already registered");
		return ;
	}

	if (args.size() < 5)
	{
		sendError(fd, "461", "USER :Not enough parameters");
		return ;
	}

	// args[2] et args[3] inutiles pour ft_irc (hostname et servername = :ircserver)
	std::string username = args[1];
	std::string realname = args[4];

	client->setUsername(username);
	client->setRealname(realname);
	client->setUserSet(true);

	tryRegister(client, fd);
}

void Server::handlePrivmsg(int fd, const std::vector<std::string> &args)
{
	Client	*client = _clients[fd];

	if (!client->isRegistered())
		return ;

	if (args.size() < 3)
	{
		sendError(fd, "461", "PRIVMSG :Not enough parameters");
		return ;
	}

	const std::string &target = args[1];
	const std::string &msg = args[2];
	std::string prefix = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost ";
	std::string fullMsg = prefix + "PRIVMSG " + target + " :" + msg + "\r\n";

	if (target[0] == '#')
	{
		if (_channels.find(target) == _channels.end())
		{
			sendError(fd, "403", target + " :No such channel");
			return ;
		}

		Channel &chan = _channels[target];
		for (std::set<int>::const_iterator it = chan.getClientsFd().begin(); it != chan.getClientsFd().end(); ++it)
		{
			if (*it != fd)
				send(*it, fullMsg.c_str(), fullMsg.size(), 0);
		}
	}
	else
	{
		for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			if (it->second->getNickname() == target)
			{
				send(it->first, fullMsg.c_str(), fullMsg.size(), 0);
				return ;
			}
		}
		sendError(fd, "401", target + " :No such nick");
	}
}

void Server::handleJoin(int fd, const std::vector<std::string> &args)
{
	Client	*client = _clients[fd];

	if (!client->isRegistered())
		return ;

	if (args.size() < 2)
	{
		sendError(fd, "461", "JOIN :Not enough parameters");
		return ;
	}

	const std::string &channelName = args[1];
	if (channelName[0] != '#' && channelName[0] != '&')
	{
		sendError(fd, "476", channelName + " :Bad Channel Mask");
	}

	if (_channels.find(channelName) == _channels.end())
		_channels.insert(std::make_pair(channelName, Channel(channelName)));

	Channel &chan = _channels[channelName];
	chan.addClient(fd);

	std::string joinMsg = ":" + client->getNickname() + " JOIN" + channelName + "\r\n";
	for (std::set<int>::const_iterator it = chan.getClientsFd().begin(); it != chan.getClientsFd().end(); ++it)
		send(*it, joinMsg.c_str(), joinMsg.size(), 0);
}

void Server::handleQuit(int fd, const std::vector<std::string> &args)
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return ;
	Client *client = it->second;

	std::string reason = "Client Quit";
	if (args.size() > 1)
		reason = args[1];
	std::string quitMsg = ":" + client->getNickname() + " QUIT :" + reason + "\r\n";

	for(std::map<std::string, Channel>::iterator chan_map = _channels.begin(); chan_map != _channels.end(); ++chan_map)
	{
		Channel &chan = chan_map->second;
		if (chan.getClientsFd().empty())
		{
			for(std::set<int>::const_iterator it = chan.getClientsFd().begin(); it != chan.getClientsFd().end(); ++it)
			{
				if (*it != fd)
					send(*it, quitMsg.c_str(), quitMsg.size(), 0);
			}
			chan.removeClient(fd);
		}
	}
	removeClient(fd);
}