#include "../inc/Server.hpp"

/**
 * @brief PASS command, used to set a 'connection password'
 * @note usage: PASS <password>
 * @return send ERR_NEEDMOREPARAMS, ERR_ALREADYREGISTERED or ERR_PASSWDMISMATCH in case of error
 */
void Server::handlePass(int fd, const std::vector<std::string> &args)
{
	Client	*client = _clients[fd];

	if (client->isRegistered())
	{
		sendError(fd, ERR_ALREADYREGISTERED, "Already registered");
		return ;
	}

	if (args.size() < 2)
	{
		sendError(fd, ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
		return ;
	}

	std::string password = args[1];
	if (password != _password)
	{
		sendError(fd, ERR_PASSWDMISMATCH, "Password incorrect");
		return ;
	}

	client->setPassOk(true);
	std::cout << "Client " << fd << " passed authentication" << std::endl;
}

/**
 * @brief NICK command, used to give user a nickname or to change it
 * @note usage: NICK <nickname>
 * @return send ERR_NONICKNAMEGIVEN, ERR_ERRONEUSNICKNAME or ERR_NICKNAMEINUSE in case of error
 */
// TODO ERR_ERRONEUSNICKNAME
void Server::handleNick(int fd, const std::vector<std::string> &args)
{
	Client	*client = _clients[fd];

	if (args.size() < 2)
	{
		sendError(fd, ERR_NONICKNAMEGIVEN, "No nickname given");
		return ;
	}

	std::string newNickname = args[1];
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == newNickname)
		{
			sendError(fd, ERR_NICKNAMEINUSE, "Nickname is already in use");
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

/**
 * @brief USER command, used to specify username, hostname and realname of a new user.
 * @note usage: USER <user> <mode> <unused> <realname>
 * @return send ERR_NEEDMOREPARAMS, ERR_ALREADYREGISTERED in case of error
 */
void Server::handleUser(int fd, const std::vector<std::string> &args)
{
	Client	*client = _clients[fd];

	if (client->isRegistered())
	{
		sendError(fd, ERR_ALREADYREGISTERED, "Already registered");
		return ;
	}

	if (args.size() < 5)
	{
		sendError(fd, ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
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
		sendError(fd, ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
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
			sendError(fd, ERR_NOSUCHCHANNEL, target + " :No such channel");
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
		sendError(fd, ERR_NOSUCHNICK, target + " :No such nick");
	}
}

void Server::handleJoin(int fd, const std::vector<std::string> &args)
{
	Client	*client = _clients[fd];

	if (!client->isRegistered())
		return ;

	if (args.size() < 2)
	{
		sendError(fd, ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
		return ;
	}

	const std::string &channelName = args[1];
	if (channelName[0] != '#' && channelName[0] != '&')
	{
		sendError(fd, ERR_BADCHANMASK, channelName + " :Bad Channel Mask");
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

void Server::handlePart(int fd, const std::vector<std::string> &args)
{
	Client *client = _clients[fd];

	if (!client->isRegistered())
		return ;

	if (args.size() < 2)
	{
		sendError(fd, ERR_NEEDMOREPARAMS, "PART :Not enough parameters");
		return ;
	}

	const std::string &channelName = args[1];

	if (_channels.find(channelName) == _channels.end())
	{
		sendError(fd, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return ;
	}

	Channel &chan = _channels[channelName];

	if (!chan.hasMember(fd))
	{
		sendError(fd, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		return ;
	}

	std::string reason = channelName;
	if (args.size() > 2)
		reason = args[2];

	std::string partMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost PART " + channelName + " :" + reason + "\r\n";

	/*C  ->  part #irctest :I'm joined to too many channels already
	  S <-   :alice!a@localhost PART #irctest :I'm joined to too many channels already*/

	for (std::set<int>::const_iterator it = chan.getClientsFd().begin(); it != chan.getClientsFd().end(); ++it)
		send(*it, partMsg.c_str(), partMsg.size(), 0);

	chan.removeClient(fd);

	if (chan.getMemberCount() == 0)
		_channels.erase(channelName);
}

void Server::handleTopic(int fd, const std::vector<std::string> &args)
{
	Client *client = _clients[fd];

	if (!client->isRegistered())
		return ;

	if (args.size() < 2)
	{
		sendError(fd, ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");
		return ;
	}

	const std::string &channelName = args[1];

	if (_channels.find(channelName) == _channels.end())
	{
		sendError(fd, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
		return ;
	}

	Channel &chan = _channels[channelName];

	if (!chan.hasMember(fd))
	{
		sendError(fd, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
		return ;
	}

	/*If only has 2 args (ex: TOPIC #general) show the topic of the channel, it can has no topic*/
	if (args.size() == 2)
	{
		if (chan.getTopic().empty())
		{
			std::string msg = ":ircserv 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n";
			send(fd, msg.c_str(), msg.size(), 0);
		}
		else
		{
			std::string msg = ":ircserv 332 " + client->getNickname() + " " + channelName + " :" + chan.getTopic() + "\r\n";
			send(fd, msg.c_str(), msg.size(), 0);
		}
		return ;
	}

	/* pour unset topic faudrait modifier le parsing 🤔*/
	
	chan.setTopic(args[2]);

	std::string topicMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost TOPIC " + channelName + " :" + args[2] + "\r\n";
	for (std::set<int>::const_iterator it = chan.getClientsFd().begin(); it != chan.getClientsFd().end(); ++it)
		send(*it, topicMsg.c_str(), topicMsg.size(), 0);

	/*C  ->  TOPIC #v3 :This is a cool channel!
	  S <-   :dan!d@Clk-830D7DDC TOPIC #v3 :This is a cool channel!*/
}
