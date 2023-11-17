/* run a command and returns its output that you can assing to a variable
 * you have to free the memory
 * example:
 *  char *cmd = "pactl list sinks | grep 'Volume:' | head -n 1 | awk '{print $5}' | sed 's/%//'";
	char *outputVol = output_to_char(cmd);
	int currentVolume = atoi(outputVol);
	free(outputVol);
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

char *output_to_char(char *command) {
	char *buffer = (char *)malloc(777);

	// allocate memory on the heap
	if (buffer == NULL) {
		perror("malloc");
		return NULL;
	}

	memset(buffer, 0, 777);

	int pipe_fd[2];
	if (pipe(pipe_fd) == -1) {
		perror("pipe");
		free(buffer); // free allocated memory before returning
		return false;
	}

	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		free(buffer); // free allocated memory before returning
		return false;
	}

	if (pid == 0) {
		// Child process
		close(pipe_fd[0]); // Close the read end of the pipe

		// Redirect the standard output to the pipe
		dup2(pipe_fd[1], STDOUT_FILENO);
		close(pipe_fd[1]);

		// run command
		execlp("/bin/sh", "/bin/sh", "-c", command, NULL);

		perror("execlp");
		free(buffer); // free allocated memory before returning
		exit(1);
	}
	else {
		// Parent process
		close(pipe_fd[1]);  // Close the write end of the pipe

		ssize_t n;

		n = read(pipe_fd[0], buffer, 776);
		close(pipe_fd[0]);

		if (n <= 0) {
			perror("read");
			free(buffer); // free allocated memory before returning
			return false;
		}
	}
	buffer[strlen(buffer) - 1] = '\0'; // removes new line

	return buffer;
}
