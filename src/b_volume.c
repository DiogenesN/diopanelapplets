// call this funtion with state 1 to increase and 0 to decrease volume

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "externvars.h"
#include "outputtochar.h"
#include "subprocessrun.h"
#include "replacenthline.h"
#include "getvaluefromconf.h"

int volume(int state) {
	const char *cmd = "pactl list sinks | grep 'Volume:' | head -n 1 | awk '{print $5}' | sed 's/%//'";
	const gchar *outputCurrVol = output_to_char(cmd);
	int currentVolume = atoi(outputCurrVol);
	printf("current volume %d\n", currentVolume);

	if (currentVolume <= 0) currentVolume = currentVolume - 0;
	if (currentVolume >= 150) currentVolume = 150;

	if (currentVolume >= 150 && state == 0) {
		// if max volume reached
		printf("WARNING! Maximum volume reached!\n");
		return 1;
	}
	else if (currentVolume <= 0 && state == 1) {
		// if min volume reached
		printf("WARNING! Minumum volume reached!\n");
		return 1;
	}
	if (state == 0)	{
		// volume up
		printf("volume up\n");

		int pid = fork();

		if (pid == -1) {
			printf("Failed to fork\n");
			return 1;
		}
		if (pid == 0) {
			execlp("/bin/sh",
				   "/bin/sh",
				   "-c",
				   "pactl set-sink-volume @DEFAULT_SINK@ +3%",
				   (char *)NULL);
		}
		else {
			wait(NULL);
		}
	}
	else {
		// volume down
		printf("brightness down\n");

		int pid = fork();

		if (pid == -1) {
			printf("Failed to fork\n");
			return 1;
		}

		if (pid == 0) {
			execlp("/bin/sh",
				   "/bin/sh",
				   "-c",
				   "pactl set-sink-volume @DEFAULT_SINK@ -3%",
				   (char *)NULL);
		}
		else {
			wait(NULL);
		}
	}
	free((void *)outputCurrVol);
	return 0;
}
