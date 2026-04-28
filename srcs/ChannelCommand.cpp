#include "../inc/Server.hpp"

/**
 * @brief KICK command, used to request the forced removal of a user from a channel. It causes the <user> to PART by force
 * @note usage : KICK <channel> <user> ( <comment> )
 * @note <channel> and <user> can be a list of args
 * @note if <comment> is given, it replaces the default message sent
 * @return ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL, ERR_BADCHANMASK, ERR_CHANOPRIVSNEEDED,
 * ERR_USERNOTINCHANNEL or ERR_NOTONCHANNEL in case of error
 */
void Server::handleKick(int fd, const std::vector<std::string> &args)
{
	// ? ERR_BADCHANMASK
	Client *client = _clients[fd];

	if (!client->isRegistered())
		return ;

	if (args.size() < 3)
	{
		sendError(fd, ERR_NEEDMOREPARAMS, "KICK :Not enough parameters");
		return ;
	}

	const std::string &channelName = args[1];
	const std::string &targetNick = args[2];

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

	if (!chan.isOperator(fd))
	{
		sendError(fd, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return ;
	}

	int targetFd = -1;
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == targetNick)
		{
			targetFd = it->first;
			break;
		}
	}

	if (targetFd == -1)
	{
		sendError(fd, ERR_NOSUCHNICK, targetNick + " :No such nick/channel");
		return ;
	}

	if (!chan.hasMember(targetFd))
	{
		sendError(fd, ERR_USERNOTINCHANNEL, targetNick + " " + channelName + " :They aren't on that channel");
		return ;
	}

	std::string reason = targetNick;
	if (args.size() > 3)
		reason = args[3];

	std::string kickMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";

	const std::set<int> &members = chan.getClientsFd();
	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		send(*it, kickMsg.c_str(), kickMsg.length(), 0);
	}

	chan.removeClient(targetFd);
}

/**
 * @brief INVITE command, used to invite a user to a channel
 * @note usage : INVITE <nickname> <channel>
 * @note No requirement that the channel must exist.
 * @note However, if the channel exists, only members of the channel are allowed to INVITE.
 * @note When invite-only is set in the channel, only channel operators may issue INVITE command.
 * @return ERR_NEEDMOREPARAMS, ERR_NOSUCHNICK, ERR_NOTONCHANNEL, ERR_USERONCHANNEL, ERR_CHANOPRIVSNEEDED in case of error,
 * else RPL_INVITING and RPL_AWAY
 */
void Server::handleInvite(int fd, const std::vector<std::string> &args)
{
	Client *client = _clients[fd];

	if (!client->isRegistered())
		return ;

	if (args.size() < 3)
	{
		sendError(fd, ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters");
		return ;
	}

	const std::string &channelName = args[1];
	const std::string &targetNick = args[2];

	Channel chan = _channels[channelName];

	if (!chan.hasMember(fd))
	{
		sendError(fd, ERR_NOTONCHANNEL, channelName + " :You're not on the channel");
		return ;
	}

	if (!chan.isOperator(fd))
	{
		sendError(fd, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
		return ;
	}

	int targetFd = -1;
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == targetNick)
		{
			targetFd = it->first;
			break;
		}
	}

	if (targetFd == -1)
	{
		sendError(fd, ERR_NOSUCHNICK, targetNick + " :No suchnick/channel");
		return ;
	}

	if (!chan.hasMember(targetFd))
	{
		sendError(fd, ERR_USERNOTINCHANNEL, targetNick + " " + channelName + " :They aren't on that channel");
		return ;
	}

	std::string reason = targetNick;
	if (args.size() > 3)
		reason = args[3];

	std::string inviteMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost INVITE " + channelName + " " + targetNick + " :" + reason + "\r\n";

	const std::set<int> &members = chan.getClientsFd();
	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		send(*it, inviteMsg.c_str(), inviteMsg.length(), 0);
	}

	chan.addClient(targetFd);
}

/**
 * @brief TOPIC command, used to change or view the topic of a channel
 * @note usage : TOPIC <channel> ( <topic> )
 * @note If <topic> is present, the topic for <channel> will be changed. If <topic> is empty, the topic will be removed
 * @return If there is no <topic>, the topic for <channel> is returned.
 * @return ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL or ERR_NOTONCHANNEL in case of error
 */
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

void Server::handleMode(int fd, const std::vector<std::string> &args)
{
	Client *client = _clients[fd];


}