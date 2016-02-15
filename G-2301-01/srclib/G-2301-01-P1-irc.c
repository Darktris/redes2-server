#include <G-2301-01-P1-irc.h>
#include <redes2/irc.h>
#include <syslog.h>
#include <G-2301-01-P1-server.h>
int nick(char* command, void* more) {
    char* prefix, *nick;
    
    syslog(LOG_INFO, "Nick called");
    IRCParse_Nick (command,&prefix,&nick);
    //IRCTADUser_Add (char *user, char *nick, char *realname, char *password, char *host, char *IP)
    syslog(LOG_INFO, "nick: prefix=%s nick=%s", prefix, nick);
}

int no_command(char* command, void* more) {
    syslog(LOG_INFO, "NYI: %s", command);
    return 0;
}
