/* vim: set ts=4 sw=4 et: */
/**
  @file G-2301-01-P1-daemon.c
  @brief Libreria de creacion de un daemon
  @author Sergio Fuentes  <sergio.fuentesd@estudiante.uam.es>
  @author Daniel Perdices <daniel.perdices@estudiante.uam.es>
  @date 2016/02/10
  */
#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "G-2301-01-P1-daemon.h"

/**
  @brief Crea un servicio daemon identificado
  @param srvcid: Identificacion del daemon
  @return DMNOK
*/
int daemonize(char* srvcid) {
    pid_t pid;

    if(!srvcid) return DMNERR_ID;

    pid = fork();
    if(pid<0) return DMNERR_FORK;
    if(pid>0) return DMNOK;

    umask(0);
    setlogmask(LOG_UPTO(LOG_INFO));
    openlog(srvcid ":", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL3);
    syslog(LOG_ERR, "Started " srvcid);

    if(setsid()<0) {
        syslog(LOG_ERR, "Error creating session");
        return DMNERR_SID;
    }
}
