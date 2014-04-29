#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <string>

#define BUFFERSIZE 2048

/*bind socket return fd if success*/
int socketBind(int port);

/*send content to the router on port, return 0 if success*/
int socketSend(int fd, int port, std::string content);

/*receive content, return port number if success, else return -1*/
int socketReceive(int fd, std::string &content);

#endif
