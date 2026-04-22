#ifndef SERVER_HPP
	#define SERVER_HPP
	#include <iostream>

	class	Server
	{
		private:

		public:
			Server();
			Server(Server const & src);
			~Server();

			Server &	operator=(Server const & rhs);
	};

#endif
