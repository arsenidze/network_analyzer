#include "daemon.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <sys/stat.h>

void	daemon_start(void)
{
	pid_t	pid;
	pid_t	sid;
	
	pid = fork();
	if (pid < 0) {
		syslog(LOG_ERR, "%s", strerror(errno));
    	exit(EXIT_FAILURE);
    }

	if (pid > 0) {
		syslog(LOG_DEBUG, "%s", "Parent fork success");
		exit(EXIT_SUCCESS);
	}

	umask(0);

	sid = setsid();
	if (sid < 0) {
		syslog(LOG_ERR, "%s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if ((chdir("/")) < 0) {
		syslog(LOG_ERR, "%s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}