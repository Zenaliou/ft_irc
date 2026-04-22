#ifndef CLIENT_HPP
	#define CLIENT_HPP
	#include <iostream>
	#include <string>
	#include "Channel.hpp"

	class	Client
	{
		private:
			int	_fd;
			std::string	_nickname; // nom public utilise
			std::string	_username; // user command

			std::set<std::string> _channels;
			/**
			 * 	bool _isAuthenticated;
			 * 	bool _passOk;
			 * 	bool _nickSet;
			 * 	bool _userSet;
			 */

		public:
			Client();
			Client(int fd);
			~Client();
			std::string	_buffer;
			void setNickname(const std::string &nickname);
			void setUsername(const std::string &username);

			int getFd() const;
			std::string getNickname() const;
			std::string getUsername() const;

	};

#endif
