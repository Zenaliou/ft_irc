*This project has been created as part of the 42 curriculum by makoon, eieong, niclee.*

# ft_IRC Internet Relay Chat

## Description

This project consists in the implementation of a simplified IRC (Internet Relay Chat) server written in C++98.

The goal is to recreate the core functionality of an IRC server, allowing multiple clients to connect, communicate in channels, and exchange message in real time using the IRC protocol.

The server handles multiple simultaneous clients using non-blocking I/0 and the poll() system call.
It supports basic IRC commands such as:
- PASS : command used to set a connection password
- NICK : command used to give a nickname or change the previous one
- USER : command used at the beginning of connection to specify the username, hostname, servername and realname of new user
- JOIN : command used to join a channel
- PART : command used to leave a channel
- PRIVMSG : command used to send a private message to a user or a channel
- QUIT : command used to disconnect from the server
- PING : command used to check if the connection is still alive
- KICK : command used to remove a user from a channel
- INVITE : command used to invite a user  from a channel
- TOPIC : command used to set or view the topic of a channel
- MODE : command used to set or modify user or channel modes (permissions and settings).

The different flags that can be used to MODE command :
- i : only invited users can join the channel
- t : only channel operators can change the topic
- k : a password is required to join the channel
- o : grants or removes operator rights to a user
- l : set the maximum number of users allowed in the channel

The project also includes optional bonus features such as a bot and potential file transfer support.


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
./ircbot <host> <port> <password> <nickname>    # for bot
```
port : The port number on which IRC server will be listening for incoming IRC connections
password: The connection password. It will be needed by any IRC client that tries to connect the server.

### Connecting with IRC client

** With irssi :**
```bash
irssi
/connect localhost <port> <password> [<nickname>]
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
PRIVMSG <receiver>{,<receiver>} <text to be sent>
QUIT [<Quit message>]
PING <server>
KICK <channel> <user> [<comment>]
INVITE <nickname> <channel>
TOPIC <channel> [<topic>]
MODE <channel> {[+|-]|o|i|t} [<limit>] [<user>]

### Testing bonus

```bash
echo "Hello !" > /tmp/test.txt  # Create a file
```

On client1 connection :
```bash
/dcc send <client2> /tmp/test.txt # Send test.txt to client2
```

On client2 connection :
```bash
/dcc get <client1>
```

## Resources

### Documentation

RFC 1459 - IRC Protocol : https://datatracker.ietf.org/doc/html/rfc1459


### AI Usage

ChatGPT was used as a learning and debugging assistant.