#ifndef CHANNEL_HPP
	#define CHANNEL_HPP
	#include <iostream>
	#include <set>

	class	Channel
	{
		private:
			std::string _name; // nom du channel
			std::string _topic;
			std::set<int> _clients_fd; // liste des membres
			std::set<int> _operators;
			bool _inviteOnly; // MODE i
			std::string _password; // MODE k
			int	_user_limit; //MODE l
		public:
			Channel();
			Channel(const std::string &name);
			~Channel();

			const std::set<int> &getClientsFd() const;

			void addClient(int fd);
			void removeClient(int fd);
	};

#endif
