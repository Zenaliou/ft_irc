#include "../inc/bot.hpp"

// Extrait le nickname d'un préfixe IRC
// Format IRC: nickname!user@host
// Retourne uniquement la partie "nickname"
// Exemple: "Alice!ALice@localhost" -> retourne "Alice"
//          "Bob!bobby@192.168.1.1" -> retourne "Bob"
//          "Charlie" (pas de '!') -> retourne "Charlie"
std::string Bot::getNickFromPrefix(const std::string &prefix)
{
	// Cherche le caractère '!' qui sépare le nickname du reste
	size_t pos = prefix.find('!');
	if (pos != std::string::npos)
		return prefix.substr(0, pos); // Retourne la partie avant le '!'
	return prefix; // Si pas de '!', retourne le préfixe entier
}

// Parse un message IRC complet en ses composants
// Format IRC: [:prefix] COMMAND [params] [:trailing]
// 
// Exemples:
//   Input:  ":Alice!ALice@localhost PRIVMSG #general :Hello world"
//   Output: prefix="Alice!ALice@localhost", command="PRIVMSG", params=["#general", "Hello world"]
//
//   Input:  "PING :server.local"
//   Output: prefix="", command="PING", params=["server.local"]
//
//   Input:  ":Bob!bob@host JOIN #general"
//   Output: prefix="Bob!bob@host", command="JOIN", params=["#general"]
void Bot::parseMessage(const std::string &line, std::string &prefix, std::string &command, std::vector<std::string> &params)
{
	// Réinitialise tous les paramètres de sortie
	params.clear();
	prefix.clear();
	command.clear();

	std::string msg = line;
	size_t pos = 0;

	if (!msg.empty() && msg[0] == ':')
	{
		pos = msg.find(' ');
		if (pos != std::string::npos)
		{
			prefix = msg.substr(1, pos - 1);
			msg = msg.substr(pos + 1);
		}
	}

	pos = msg.find(' ');
	if (pos != std::string::npos)
	{
		command = msg.substr(0, pos);
		msg = msg.substr(pos + 1);
	}
	else
	{
		command = msg;
		return;
	}

	while (!msg.empty())
	{
		if (msg[0] == ':')
		{
			params.push_back(msg.substr(1));
			break;
		}
		
		pos = msg.find(' ');
		if (pos != std::string::npos)
		{
			params.push_back(msg.substr(0, pos));
			msg = msg.substr(pos + 1);
		}
		else
		{
			params.push_back(msg);
			break;
		}
	}
}

// Gère les messages privés (PRIVMSG) reçus
// Détecte et exécute les commandes du bot (commençant par '!')
// 
// Exemples:
//   sender="Alice!ALice@localhost", target="#general", message="!poll Question? Yes No"
//   -> Affiche: "[#general] <Alice> !poll Question? Yes No"
//   -> Appelle: handleCommand("Alice", "#general", "poll", "Question? Yes No")
//
//   sender="Bob!bob@host", target="PollBot", message="!help"
//   -> Affiche: "[PollBot] <Bob> !help"
//   -> Appelle: handleCommand("Bob", "Bob", "help", "") (répond en privé)
//
//   sender="Tom!tom@host", target="#general", message="Hello everyone!"
//   -> Affiche: "[#general] <Tom> Hello everyone!" (pas de commande, juste affichage)
void Bot::handlePrivmsg(const std::string &sender, const std::string &target, const std::string &message)
{
	std::string nick = getNickFromPrefix(sender);
	std::cout << "[" << target << "] <" << nick << "> " << message << std::endl;

	if (!message.empty() && message[0] == '!')
	{
		size_t space = message.find(' ');
		std::string cmd = message.substr(1, space - 1);
		std::string args = (space != std::string::npos) ? message.substr(space + 1) : "";
		
		std::string reply_target = (target == _nickname) ? nick : target;
		handleCommand(nick, reply_target, cmd, args);
	}
}

// Point d'entrée principal pour traiter tous les messages reçus du serveur IRC
// Détermine le type de message et appelle le gestionnaire approprié
// 
// Exemples:
//   Input: "PING :server.local"
//   -> Répond avec: "PONG :server.local\r\n"
//
//   Input: ":Alice!ALice@localhost PRIVMSG #general :!poll test? A B C"
//   -> Appelle: handlePrivmsg("Alice!ALice@localhost", "#general", "!poll test? A B C")
//
//   Input: ":server.local 001 PollBot :Welcome to the IRC Network"
//   -> Affiche: "Successfully registered with the server!"
void Bot::handleMessage(const std::string &line)
{
	std::string prefix, command;
	std::vector<std::string> params;
	
	parseMessage(line, prefix, command, params);

	if (command == "PING")
	{
		if (!params.empty())
			sendRaw("PONG :" + params[0] + "\r\n");
		return;
	}

	if (command == "PRIVMSG" && params.size() >= 2)
	{
		handlePrivmsg(prefix, params[0], params[1]);
		return;
	}

	if (command == "001")
	{
		std::cout << "Successfully registered with the server!" << std::endl;
	}
}

// Envoie un message brut au serveur IRC sans modification
// Utilisé pour envoyer des commandes IRC directement
// 
// Exemples:
//   sendRaw("PONG :server.local\r\n") -> Répond à un PING
//   sendRaw("JOIN #general\r\n") -> Rejoint le channel #general
//   sendRaw("NICK NewBotName\r\n") -> Change le nickname du bot
void Bot::sendRaw(const std::string &msg)
{
	if (_sockfd < 0)
		return;
	send(_sockfd, msg.c_str(), msg.length(), 0);
	std::cout << ">>> " << msg;
}

// Envoie un message privé à un utilisateur ou dans un channel
// Construit et envoie une commande PRIVMSG formatée selon le protocole IRC
// 
// Exemples:
//   sendPrivmsg("#general", "New poll created!") -> Envoie au channel
//   -> Génère: "PRIVMSG #general :New poll created!\r\n"
//
//   sendPrivmsg("Alice", "You already voted!") -> Envoie en privé
//   -> Génère: "PRIVMSG Alice :You already voted!\r\n"
void Bot::sendPrivmsg(const std::string &target, const std::string &msg)
{
	sendRaw("PRIVMSG " + target + " :" + msg + "\r\n");
}
