#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char charbuffer[256];

char *return_nth_line(char *filename, int linenumber) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		perror("Error opening file");
		return NULL;
	}

	int currentLine = 1;

	while (fgets(charbuffer, sizeof(charbuffer), file) != NULL) {
		if (currentLine == linenumber) {
			charbuffer[strlen(charbuffer) - 1] = '\0';
			return charbuffer;
			break;  
		}

		currentLine++;
	}
	fclose(file);
	return NULL;
}
