/* creates initial config directory and file */

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

void create_configs() {
	const char *HOME = getenv("HOME");

	if (HOME == NULL) {
		fprintf(stderr, "Unable to determine the user's home directory.\n");
		return;
	}

	const char *dirConfigDioPanel 			= "/.config/diopanel";
	const char *dirConfigDioPanelApplets	= "/diopanelapplets";
	const char *fileConfigDioPanelApplets	= "/diopanelapplets.conf";
	const char *dirConfigPathFile			= "/.config/diopanel/diopanelapplets/keyboardlayouts.conf";
	char 		dioPanel[700];
	char 		ConfigPathFile[700];
	char 		dioPanelApplets[700];
	char 		dioPanelAppletsCOnfig[700];

	snprintf(dioPanel, sizeof(dioPanel), "%s%s", HOME, dirConfigDioPanel);
	snprintf(dioPanelApplets, sizeof(dioPanelApplets), "%s%s%s", HOME, dirConfigDioPanel,
																	   dirConfigDioPanelApplets);

	DIR *mainDir = opendir(dioPanel);
	DIR *subDir	 = opendir(dioPanelApplets);

	// cheks if the file already exists
	if (mainDir && subDir) {
		// directory exists do nothing
		closedir(mainDir);
		closedir(subDir);
		return;
	}
	if (!mainDir) {
		// main dir not found
		printf("created initial configs\n");
		mkdir(dioPanel, 0755);
		mkdir(dioPanelApplets, 0755);

		closedir(subDir);
		closedir(mainDir);
	}
	if (!subDir) {
		// sub dir not found
		printf("created initial subdir configs\n");
		mkdir(dioPanelApplets, 0755);

		closedir(subDir);
		closedir(mainDir);
	}

	snprintf(ConfigPathFile, sizeof(ConfigPathFile), "%s%s", HOME, dirConfigPathFile);
	snprintf(dioPanelAppletsCOnfig, sizeof(dioPanelAppletsCOnfig), "%s%s%s%s",
																	HOME,
																	dirConfigDioPanel,
    																dirConfigDioPanelApplets,
    																fileConfigDioPanelApplets);

	char *width			= "width=100";
	char *height		= "height=100";
	char *posx			= "posx=700";
	char *posy			= "posy=700";
	char *volup			= "volumeup=default";
	char *voldown		= "volumedown=default";
	char *brightup		= "brightup=default";
	char *brightdown	= "brightdown=default";
	char *currbright	= "currentbrightness=1.0";

	FILE *file = fopen(ConfigPathFile, "w+");
	
	fprintf(file, "%s\n", "us");
	fprintf(file, "%s\n", "add");
	fprintf(file, "%s\n", "add");

	fclose(file);

	FILE *confFile			= fopen(dioPanelAppletsCOnfig, "w+");

	fprintf(confFile, "%s\n", width);
	fprintf(confFile, "%s\n", height);
	fprintf(confFile, "%s\n", posx);
	fprintf(confFile, "%s\n", posy);
	fprintf(confFile, "%s\n", volup);
	fprintf(confFile, "%s\n", voldown);
	fprintf(confFile, "%s\n", brightup);
	fprintf(confFile, "%s\n", brightdown);
	fprintf(confFile, "%s\n", currbright);

	fclose(confFile);
}
