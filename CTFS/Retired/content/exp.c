#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


int main(void) {
    	char cmd[512] = { 0 };

    	read(STDIN_FILENO, cmd, sizeof(cmd));
	cmd[-1] = 0;

	/*Vemos que el buffer pude ser explotado,, pero como?*/
	printf("%s", cmd);



        return 0;
}


