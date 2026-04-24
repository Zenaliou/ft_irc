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
		send(fd, "462 Already registered\r\n", 26, 0);
		return ;
	}

	if (args.size() < 2)
	{
		send(fd, "461 PASS :Not enough parameters\r\n", 37, 0);
		return ;
	}

	std::string password = args[1];
	if (password != _password)
	{
		send(fd, "464 Password incorrect\r\n", 26, 0);
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
		send(fd, "431 No nickname given\r\n", 26, 0);
		return ;
	}

	std::string newNickname = args[1];
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == newNickname)
		{
			send(fd, "433 Nickname is already in use\r\n", 36, 0);
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
		send(fd, "462 Already registered\r\n", 26, 0);
		return ;
	}

	if (args.size() < 2)
	{
		send(fd, "461 USER :Not enough parameters\r\n", 37, 0);
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
