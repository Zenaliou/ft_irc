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

static bool isSpecial(char c)
{
	return (c == '[' || c == ']' || c == '\\' || c == '`' || c == '_' ||
			c == '^' || c == '{' || c == '}' || c == '|');
}

static bool isLetter(char c)
{
	return (std::isalpha(static_cast<unsigned char>(c)));
}

static bool isDigit(char c)
{
	return (std::isdigit(static_cast<unsigned char>(c)));
}

static bool isValidNickname(const std::string &nick)
{
	if (nick.empty() || nick.size() > 9)
		return (false);

	if (!(isLetter(nick[0])) && !isSpecial(nick[0]))
		return (false);

	for (size_t i = 1; i < nick.length(); ++i)
	{
		char c = nick[i];
		if (!(isLetter(c) || isDigit(c) || isSpecial(c) || c == '-'))
			return (false);
	}
	return (true);
}

/**
 * @brief NICK command, used to give user a nickname or to change it
 * @note usage: NICK <nickname>
 * @return send ERR_NONICKNAMEGIVEN, ERR_ERRONEUSNICKNAME or ERR_NICKNAMEINUSE in case of error
 */
void Server::handleNick(int fd, const std::vector<std::string> &args)
{
	Client	*client = _clients[fd];

	if (args.size() < 2)
	{
		sendError(fd, ERR_NONICKNAMEGIVEN, "No nickname given");
		return ;
	}

	std::string newNickname = args[1];
	if (!isValidNickname(newNickname))
	{
		sendError(fd, ERR_ERRONEUSNICKNAME, newNickname + " :Erroneous nickname");
		return ;
	}

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
	// TODO args[2] pour set le mode
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

/**
 * @brief PRIVMSG command, used to send messages to user/channel.
 * @note usage: PRIVMSG <msgtarget> <text to be sent>
 * @note <msgtarget> is usually the nickname or channel name
 * @return send ERR_NORECIPIENT, ERR_NOTEXTTOSEND, ERR_CANNOTSENDTOCHAN,
 * 		ERR_TOOMANYTARGETS, ERR_NOSUCHNICK in case of error, else RPL_AWAY
 */
void Server::handlePrivmsg(int fd, const std::vector<std::string> &args)
{
	Client	*client = _clients[fd];

	if (!client->isRegistered())
		return ;
	if (args.size() < 2)
	{
		sendError(fd, ERR_NORECIPIENT, ":No recipient given (PRIVMSG)");
		return ;
	}
	else if (args.size() < 3)
	{
		const std::string &param = args[1];
		if (param[0] == ':')
		{
			sendError(fd, ERR_NORECIPIENT, ":No recipient given (PRIVMSG)");
			return ;
		}
		else
		{
			sendError(fd, ERR_NOTEXTTOSEND, ":No text to send");
			return ;
		}
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

/**
 * @brief JOIN command, used to start listening to specific channel.
 * @note usage: JOIN <channel> <key> OR JOIN 0
 * @note <channel> and <key> can be a list of args (multiple targets and keys)
 * @note 0 is used to PART from all channel
 * @return send ERR_NEEDMOREPARAMS, ERR_BANNEDFROMCHAN, ERR_INVITEONLYCHAN, ERR_BADCHANNELKEY,
 * 		ERR_CHANNELISFULL, ERR_BADCHANMASK, ERR_NOSUCHCHANNEL, ERR_TOOMANYCHANNELS,
 * 		ERR_TOOMANYTARGETS or ERR_UNAVAILRESOURCE in case of error, else RPL_TOPIC + RPL_NAMREPLY
 */
void Server::handleJoin(int fd, const std::vector<std::string> &args)
{
	// TODO : ERR_BANNEDFROMCHAN -> check liste bans
	// TODO : ERR_INVITEONLYCHAN -> check isInviteOnly()
	// TODO : ERR_BADCHANNELKEY -> check _password
	// TODO : ERR_CHANNELISFULL -> check _user_limit
	// TODO : ERR_NOSUCHCHANNEL -> check la syntaxe.... et non l'existence du channel (pcq si channel existe pas -> le creer)
	// TODO : ERR_TOOMANYCHANNELS -> Pas de limite explicite, depend du server... set a 10 channel ?
	// TODO : ERR_TOOMANYTARGETS -> Pareil, pas de limite, depend du server... check sur les listes d'args
	// TODO : ERR_UNAVAILRESOURCE -> Pas vraiment a gerer pour ft_irc
	// TODO : RPL_TOPIC
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

/**
 * @brief QUIT command, used to terminate a client session with a message
 * @note usage: QUIT <Quit message>
 */
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

/**
 * @brief PART command, used to be removed from the list of active members for all given channels listed in parameter, sending an optionnal message
 * @note usage : PART <channel> <part message>
 * @note <channel> can be a list of args
 * @return ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL or ERR_NOTONCHANNEL in case of error
 */
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
