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

void Server::handleMode(int fd, const std::vector<std::string> &args)
{
	Client *client = _clients[fd];


}