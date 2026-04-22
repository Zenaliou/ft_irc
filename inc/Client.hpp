#ifndef CLIENT_HPP
	#define CLIENT_HPP
	#include <iostream>
	#include <string>

	class	Client
	{
		private:
			int	_fd;
			std::string	_nickname;
			std::string	_username;
			std::string	_buffer;

		public:
			Client();
			Client(int fd);
			~Client();

			void setNickname(const std::string &nickname);
			void setUsername(const std::string &username);

			int getFd() const;
			std::string getNickname() const;
			std::string getUsername() const;

	};

#endif
