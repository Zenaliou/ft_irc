#ifndef CLIENT_HPP
#define CLIENT_HPP

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
#include <cstdlib>
	#include "Channel.hpp"

	class	Client
	{
		private:
			int	_fd;
			std::string	_nickname; // nom public utilise
			std::string	_username; // user command
			std::string _realname; // utilse pour WHO/WHOIS
			std::set<std::string> _channels;

			bool _passOk;
			bool _registered;
			bool _nickSet;
			bool _userSet;
			// bool _isAuthenticated;

		public:
			Client();
			Client(int fd);
			~Client();
			std::string	_buffer;

			void setNickname(const std::string &nickname);
			void setUsername(const std::string &username);
			void setRealname(const std::string &realname);
			void setPassOk(const bool boolean);
			void setRegistered(const bool boolean);
			void setNickSet(const bool boolean);
			void setUserSet(const bool boolean);

			int getFd() const;
			std::string getNickname() const;
			std::string getUsername() const;
			std::string getRealname() const;
			bool isPassOk() const;
			bool isRegistered() const;
			bool isNickSet() const;
			bool isUserSet() const;

			void closeConnection();
			void send_msg(const std::string &msg);
	};

#endif
