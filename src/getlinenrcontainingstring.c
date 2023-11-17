//******* returns the line number containing the given string in the given text file
#include <stdio.h>
#include <string.h>

int get_line_number_containing_string(char *textfile, char *strToSearchFor) {
	FILE *file = fopen(textfile, "r");
	if (file == NULL) {
		perror("Error opening file");
		return 1;
	}

	int lineCount = 0;
	char buffer[1024];

	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		if (strstr(buffer, strToSearchFor)) {
			break;
		}
		lineCount++;
	}

	fclose(file);

	return lineCount;
}
