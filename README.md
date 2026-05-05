*This project has been created as part of the 42 curriculum by makoon, eieong, niclee.*

# ft_IRC Internet Relay Chat

## Description

This project consists in the implementation of a simplified IRC (Internet Relay Chat) server written in C++98.

The goal is to recreate the core functionality of an IRC server, allowing multiple clients to connect, communicate in channels, and exchange message in real time using the IRC protocol.

The server hamdles multiple simultaneous clients using non-blocking I/0 and the poll() system call.
It supports basic IRC commands such as:
- PASS
- NICK
- USER
- JOIN
- PART
- PRIVMSG
- QUIT
- PING
- KICK
- INVITE
- TOPIC
- MODE

The project also includes optional bonus features such as a simple IRC and potential file transfer support.


## Instructions

### Compilation

The project must be compiled using :
```bash
make        # Compile the server
make bonus  # Compile the bot
```

### Execution

The executable will be run as follows:
```bash
./ircserv <port> <password>                     # for server
./ircbot <host> <port> <password> <nickname>    #for bot
```
port : The port number on which IRC server will be listening for incoming IRC connections
password: The connection password. It will be needed by any IRC client that tries to connect the server.

### Connecting with IRC client

** With irssi :**
```bash
irssi
/connect localhost <port> <password> <nickname>
```

** With netcat :***
```bash
nc -C localhost <password>
PASS <password>
NICK <nickname>
USER <username> <hostname> <servername> <realname>
```

Commands after client connection:
JOIN <channel>{,<channel>} [<key>{,<key>}]
PART <channel>{,<channel>}
- PRIVMSG
- QUIT
- PING
- KICK
- INVITE
- TOPIC
- MODE


## Resources

### Documentation

RFC 1459 - IRC Protocol : https://datatracker.ietf.org/doc/html/rfc1459


### AI Usage
