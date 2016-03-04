#ifndef _IRC_SVR_H
#define _IRC_SVR_H

#define NCOMMANDS 300
#define MAX_USERS 100
#define MAX_USERNAME 20
#define MAX_NICK 20
#define MAX_USERS_INCHAN 20
#define IRCSVROK 0
#define IRCSVRERR_MALLOC -1
#define IRCSVRERR_MAXLEN -2
#define IRCSVRERR_ARGS -3


#define IRCSVR_MOTD "motd"
#define IRCSVR_NAME "eps.irc.jp"
#define IRCSVR_INFO "El mejor servidor IRC, ¿no?"

int process_command(char* command, void* data);
int set_user(int socketd, char* user);
int set_nick(int socketd, char* nick);
int get_socketd(char* user);
char* get_user(int socketd); 
char* get_nick(int socketd); 
#endif
