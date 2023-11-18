/*** battery scale ***/

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <sys/wait.h>
#include "externvars.h"
#include "replacenthline.h"
#include "getvaluefromconf.h"

/// when scale autohides close the window to prevent memory leak
static void on_scale_close(GtkWidget *window) {
	gtk_window_close(GTK_WINDOW(window));
}

/// changing brightness from scale
static void brighness_scale(GtkAdjustment *adj) {
	currentBrightness = gtk_adjustment_get_value(adj);

	const char *newContent[777];
	snprintf((char *)newContent, sizeof(newContent), "currentbrightness=%.1f", currentBrightness);
	replacenthline(pathToConfig, 9, (const char *)newContent);

	char command[128];

	snprintf(command, sizeof(command),
	"xrandr --output `xrandr | awk '/ primary / {print $1}'` --brightness %.1f", currentBrightness);

	int pid = fork();

	if (pid == -1) {
		printf("Failed to fork\n");
		return;
	}

	if (pid == 0) {
		execlp("/bin/sh", "/bin/sh", "-c", command, (char *)NULL);
	}
	else {
		wait(NULL);
	}
}

/// right click on battert shows scale
static void on_battery_r_click() {
	GtkWidget *window;
	window = gtk_window_new();

	GtkWidget *plus;
	plus = gtk_label_new("+");

	GtkWidget *minus;
	minus = gtk_label_new("-");

	GdkRectangle rect;
    rect.x = get_int_value_from_conf(pathToConfig, "posx") + 87;
    rect.y = get_int_value_from_conf(pathToConfig, "posy") - 206;
    rect.width = 0;
    rect.height = 0;

	GtkAdjustment *adjScale;
	adjScale = gtk_adjustment_new(get_double_value_from_conf(pathToConfig, "currentbrightness"),
																		0.3, 1.1, 0.1, 0.1, 0.1);

	GtkWidget *scale;
	scale = gtk_scale_new(GTK_ORIENTATION_VERTICAL, adjScale);
	gtk_scale_set_digits(GTK_SCALE(scale), 1);
	gtk_range_set_inverted(GTK_RANGE(scale), TRUE);
	gtk_widget_set_size_request(scale, 0, 140);

	g_signal_connect(adjScale, "value-changed", G_CALLBACK(brighness_scale), scale);

	GtkWidget *vBox;
	vBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_box_set_homogeneous(GTK_BOX(vBox), FALSE);
	gtk_box_prepend(GTK_BOX(vBox), plus);
	gtk_box_append(GTK_BOX(vBox), scale);
	gtk_box_append(GTK_BOX(vBox), minus);

	GtkWidget *popBrightScale;
	popBrightScale = gtk_popover_new();
	gtk_widget_set_size_request(popBrightScale, 1, 200);
	gtk_popover_set_pointing_to(GTK_POPOVER(popBrightScale), &rect);
	gtk_popover_set_autohide(GTK_POPOVER(popBrightScale), TRUE);
	gtk_popover_set_has_arrow(GTK_POPOVER(popBrightScale), FALSE);
	gtk_popover_set_child(GTK_POPOVER(popBrightScale), vBox);

	g_signal_connect_swapped(popBrightScale, "closed", G_CALLBACK(on_scale_close), window);

	gtk_window_set_child(GTK_WINDOW(window), popBrightScale);
	gtk_popover_popup(GTK_POPOVER(popBrightScale));
}
