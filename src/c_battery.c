// call this funtion with state 1 to increase and 0 to decrease brightness

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "externvars.h"
#include "replacenthline.h"
#include "getvaluefromconf.h"

int brightness(int state) {
	currentBrightness = get_double_value_from_conf(pathToConfig, "currentbrightness");

	printf("Current brightness level: %.1f\n", currentBrightness);

	char command[128];

	if (currentBrightness > 1.0) currentBrightness = currentBrightness - 0.1;
	if (currentBrightness < 0.3) currentBrightness = 0.3;

	// if maximum brightness
	if (currentBrightness >= 1.0 && state == 0) {
		// if max brightness reached
		printf("WARNING! Maximum brightness reached!\n");
	}
	else if (currentBrightness <= 1.0 && currentBrightness >= 0.3 && state == 0)	{
		// brightness up
		printf("brightness up\n");

		currentBrightness = currentBrightness + 0.1;

		const char *newContent[777];
		snprintf((char *)newContent, sizeof(newContent), "currentbrightness=%.1f", currentBrightness);
		replacenthline(pathToConfig, 9, (const char *)newContent);

		snprintf(command, sizeof(command),
		"xrandr --output `xrandr | awk '/ primary / {print $1}'` --brightness %.1f",
																	currentBrightness);

		int pid = fork();

		if (pid == -1) {
			printf("Failed to fork\n");
			return 1;
		}
		if (pid == 0) {
			execlp("/bin/sh", "/bin/sh", "-c", command, (char *)NULL);
		}
		else {
			wait(NULL);
		}
	}
	else {
		// brightness down
		printf("brightness down\n");

		currentBrightness = currentBrightness - 0.1;

		const char *newContent[777];
		snprintf((char *)newContent, sizeof(newContent), "currentbrightness=%.1f", currentBrightness);
		replacenthline(pathToConfig, 9, (const char *)newContent);

		snprintf(command, sizeof(command),
		"xrandr --output `xrandr | awk '/ primary / {print $1}'` --brightness %.1f",
																	currentBrightness);

		int pid = fork();

		if (pid == -1) {
			printf("Failed to fork\n");
			return 1;
		}

		if (pid == 0) {
			execlp("/bin/sh", "/bin/sh", "-c", command, (char *)NULL);
		}
		else {
			wait(NULL);
		}
	}
	return 0;
}
