#ifndef CHANNEL_HPP
	#define CHANNEL_HPP
	#include <iostream>
	#include <set>

	class	Channel
	{
		private:
			std::string _name;
			std::set<int> _clients_fd;
		public:
			Channel();
			Channel(const std::string &name);
			~Channel();

			const std::set<int> &getClientsFd() const;

			void addClient(int fd);
			void removeClient(int fd);
	};

#endif
