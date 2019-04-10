#include "cli.h"
#include <stdio.h>

int	main(int argc, char *argv[])
{
	int	status;

	if (argc < 2 || argc > 4) {
		cli_help(argc, argv);
		return (0);
	}
	status = cli_handle_command(argc, argv);
	if (status < 0) {
		printf("Error\n");
	}
	else {
		printf("\nCommand succesfully executed\n");
	}

	return (0);
}