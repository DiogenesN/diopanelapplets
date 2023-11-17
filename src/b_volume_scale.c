/*** volume scale ***/

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <sys/wait.h>
#include "externvars.h"
#include "outputtochar.h"
#include "replacenthline.h"
#include "getvaluefromconf.h"

/// when scale autohides close the window to prevent memory leak
static void on_volume_scale_close(GtkWidget *window) {
	gtk_window_close(GTK_WINDOW(window));
}

/// changing volume from scale
static void volume_scale(GtkAdjustment *adj) {
	int currentVolume = gtk_adjustment_get_value(adj);

	// set the icon
	if (currentVolume <= 3) {
		gtk_image_set_from_icon_name(GTK_IMAGE(volumeIcon), "audio-volume-muted");
	}
	else if (currentVolume <= 30 && currentVolume >= 0) {
		gtk_image_set_from_icon_name(GTK_IMAGE(volumeIcon), "audio-volume-low");
	}
	else if (currentVolume > 30 && currentVolume <= 70) {
		gtk_image_set_from_icon_name(GTK_IMAGE(volumeIcon), "audio-volume-medium");
	}
	else {
		gtk_image_set_from_icon_name(GTK_IMAGE(volumeIcon), "audio-volume-high");
	}

	char command[128];

	snprintf(command, sizeof(command), "pactl set-sink-volume @DEFAULT_SINK@ %d%%", currentVolume);

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

/// right click on volume shows scale
static void on_volume_r_click() {
	char *cmd = "pactl list sinks | grep 'Volume:' | head -n 1 | awk '{print $5}' | sed 's/%//'";
	gchar *outputCurrVol = output_to_char(cmd);
	int currentVolume = atoi(outputCurrVol);

	GtkWidget *window;
	window = gtk_window_new();

	GtkWidget *plus;
	plus = gtk_label_new("+");

	GtkWidget *minus;
	minus = gtk_label_new("-");

	GdkRectangle rect;
    rect.x = get_int_value_from_conf(pathToConfig, "posx") + 140;
    rect.y = get_int_value_from_conf(pathToConfig, "posy") - 206;
    rect.width = 0;
    rect.height = 0;

	GtkAdjustment *adjScale;
	adjScale = gtk_adjustment_new(currentVolume, 0, 150, 3, 3, 3);

	GtkWidget *scale;
	scale = gtk_scale_new(GTK_ORIENTATION_VERTICAL, adjScale);
	gtk_scale_set_digits(GTK_SCALE(scale), 1);
	gtk_range_set_inverted(GTK_RANGE(scale), TRUE);
	gtk_widget_set_size_request(scale, 0, 140);

	g_signal_connect(adjScale, "value-changed", G_CALLBACK(volume_scale), scale);

	GtkWidget *vBox;
	vBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_box_set_homogeneous(GTK_BOX(vBox), FALSE);
	gtk_box_prepend(GTK_BOX(vBox), plus);
	gtk_box_append(GTK_BOX(vBox), scale);
	gtk_box_append(GTK_BOX(vBox), minus);

	GtkWidget *popVolumeScale;
	popVolumeScale = gtk_popover_new();
	gtk_widget_set_size_request(popVolumeScale, 1, 200);
	gtk_popover_set_pointing_to(GTK_POPOVER(popVolumeScale), &rect);
	gtk_popover_set_autohide(GTK_POPOVER(popVolumeScale), TRUE);
	gtk_popover_set_has_arrow(GTK_POPOVER(popVolumeScale), FALSE);
	gtk_popover_set_child(GTK_POPOVER(popVolumeScale), vBox);

	g_signal_connect_swapped(popVolumeScale, "closed", G_CALLBACK(on_volume_scale_close), window);

	free(outputCurrVol);

	gtk_window_set_child(GTK_WINDOW(window), popVolumeScale);
	gtk_popover_popup(GTK_POPOVER(popVolumeScale));
}
