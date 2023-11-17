/* diopanelapplets*/

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include "d_clock.c"
#include "f_notes.c"
#include "b_volume.h"
#include "g_network.c"
#include "c_battery.h"
#include "externvars.h"
#include "configsgen.c"
#include <pango/pango.h>
#include "outputtochar.h"
#include "subprocessrun.h"
#include "replacenthline.h"
#include "b_volume_scale.c"
#include "e_keyboardinput.c"
#include "c_battery_scale.c"
#include "getvaluefromconf.h"
#include "returnnthlinetochar.h"

static GtkWidget		*pop;
static GtkApplication	*app;
static GtkWidget		*entryVolUp;
static GtkWidget		*entryLayout1;
static GtkWidget		*entryLayout2;
static GtkWidget		*entryLayout3;
static GtkWidget		*entryVolDown;
static GtkWidget		*entryBrightUp;
static gchar			*currentLayoutN;
static GtkWidget		*entryBrightDown;

GtkExpression *expressionWidth;
GtkExpressionWatch *watchWidth;

GtkExpression *expressionHeight;
GtkExpressionWatch *watchHeight;

GtkExpression *expressionPosx;
GtkExpressionWatch *watchPosx;

GtkExpression *expressionPosy;
GtkExpressionWatch *watchPosy;

/// instantly applying wigth, height and other changes
static void notify(gpointer data) {
	(void)data;
	// width
  	GValue valueWidth = G_VALUE_INIT;
	gtk_expression_watch_evaluate(watchWidth, &valueWidth);

	// height
  	GValue valueHeight = G_VALUE_INIT;
	gtk_expression_watch_evaluate(watchHeight, &valueHeight);

	// position x
  	GValue valuePosx = G_VALUE_INIT;
	gtk_expression_watch_evaluate(watchPosx, &valuePosx);

	// position y
  	GValue valuePosy = G_VALUE_INIT;
	gtk_expression_watch_evaluate(watchPosy, &valuePosy);

	GtkEntryBuffer *buffentryVolUp;
	buffentryVolUp = gtk_entry_get_buffer(GTK_ENTRY(entryVolUp));
	const char *entryVolUpText = gtk_entry_buffer_get_text(buffentryVolUp);

	GtkEntryBuffer *buffentryVolDown;
	buffentryVolDown = gtk_entry_get_buffer(GTK_ENTRY(entryVolDown));
	const char *entryVolDownText = gtk_entry_buffer_get_text(buffentryVolDown);

	GtkEntryBuffer *buffentryBrightUp;
	buffentryBrightUp = gtk_entry_get_buffer(GTK_ENTRY(entryBrightUp));
	const char *entryBrightUpText = gtk_entry_buffer_get_text(buffentryBrightUp);

	GtkEntryBuffer *buffentryBrightDown;
	buffentryBrightDown = gtk_entry_get_buffer(GTK_ENTRY(entryBrightDown));
	const char *entryBrightDownText = gtk_entry_buffer_get_text(buffentryBrightDown);

	gint widthValue		= (gint)g_value_get_double(&valueWidth);
	gint heightValue	= (gint)g_value_get_double(&valueHeight);
	gint posxValue		= (gint)g_value_get_double(&valuePosx);
	gint posyValue		= (gint)g_value_get_double(&valuePosy);

	char writeWidth[256];
	char writeHeight[256];
	char writePosx[256];
	char writePosy[256];
	char writeVolUp[256];
	char writeVolDown[256];
	char writeBrightUp[256];
	char writeBrightDOwn[256];
	char writeCurrBright[256];

    snprintf(writeWidth, sizeof(writeWidth), "%s=%d", "width", widthValue);
    snprintf(writeHeight, sizeof(writeHeight), "%s=%d", "height", heightValue);
    snprintf(writePosx, sizeof(writePosx), "%s=%d", "posx", posxValue);
    snprintf(writePosy, sizeof(writePosy), "%s=%d", "posy", posyValue);
    snprintf(writeVolUp, sizeof(writeVolUp), "%s=%s", "volumeup", entryVolUpText);
    snprintf(writeVolDown, sizeof(writeVolDown), "%s=%s", "volumedown", entryVolDownText);
    snprintf(writeBrightUp, sizeof(writeBrightUp), "%s=%s", "brightup", entryBrightUpText);
    snprintf(writeBrightDOwn, sizeof(writeBrightDOwn), "%s=%s", "brightdown", entryBrightDownText);
    
	currentBrightness = get_double_value_from_conf(pathToConfig, "currentbrightness");
    snprintf(writeCurrBright, sizeof(writeCurrBright), "currentbrightness=%.1f", currentBrightness);

	FILE *configPathOpen	= fopen(pathToConfig, "w+");

	fprintf(configPathOpen, "%s\n", writeWidth);
	fprintf(configPathOpen, "%s\n", writeHeight);
	fprintf(configPathOpen, "%s\n", writePosx);
	fprintf(configPathOpen, "%s\n", writePosy);
	fprintf(configPathOpen, "%s\n", writeVolUp);
	fprintf(configPathOpen, "%s\n", writeVolDown);
	fprintf(configPathOpen, "%s\n", writeBrightUp);
	fprintf(configPathOpen, "%s\n", writeBrightDOwn);
	fprintf(configPathOpen, "%s\n", writeCurrBright);

	fclose(configPathOpen);

	gtk_popover_popdown(GTK_POPOVER(pop));
	gtk_popover_popup(GTK_POPOVER(pop));

	gtk_widget_set_size_request(GTK_WIDGET(pop), widthValue, heightValue);
	gtk_popover_set_offset(GTK_POPOVER(pop), posxValue, posyValue);

	g_value_unset(&valueWidth);
	g_value_unset(&valueHeight);
	g_value_unset(&valuePosx);
	g_value_unset(&valuePosy);
}

/// freeing resources
static int close_request_cb() {
	gtk_expression_watch_unwatch(watchWidth);
	gtk_expression_unref(expressionWidth);
	gtk_expression_watch_unwatch(watchHeight);
	gtk_expression_unref(expressionHeight);
	gtk_expression_watch_unwatch(watchPosy);
	gtk_expression_unref(expressionPosy);
	gtk_expression_watch_unwatch(watchPosx);
	gtk_expression_unref(expressionPosx);
	return false;
}

/// closes the panel
static void remove_button(GtkWindow *window) {
	state = FALSE;
	g_source_remove(threadID);
	threadID = 0;

	wifi_state = NULL;
	currentLayoutN = NULL;
	wifi_connection_name = NULL;
	wifi_connection_check = NULL;
	free(wifi_state);
	free(currentLayoutN);
	free(wifi_connection_name);
	free(wifi_connection_check);

	g_print("main loop over\n");

	gtk_popover_popdown(GTK_POPOVER(pop));
	gtk_window_close(window);
	g_application_quit(G_APPLICATION(app));
}

/// closes and saves the settings to config
static void close_button(GtkWidget *window) {
	const char *dirConfigPathFile = "/.config/diopanel/diopanelapplets/keyboardlayouts.conf";
	char 		ConfigPathFile[700];

	snprintf(ConfigPathFile, sizeof(ConfigPathFile), "%s%s", HOME, dirConfigPathFile);

	GtkEntryBuffer *buffentryVolUp;
	buffentryVolUp = gtk_entry_get_buffer(GTK_ENTRY(entryVolUp));
	const char *entryVolUpText = gtk_entry_buffer_get_text(buffentryVolUp);

	GtkEntryBuffer *buffentryVolDown;
	buffentryVolDown = gtk_entry_get_buffer(GTK_ENTRY(entryVolDown));
	const char *entryVolDownText = gtk_entry_buffer_get_text(buffentryVolDown);

	GtkEntryBuffer *buffentryBrightUp;
	buffentryBrightUp = gtk_entry_get_buffer(GTK_ENTRY(entryBrightUp));
	const char *entryBrightUpText = gtk_entry_buffer_get_text(buffentryBrightUp);

	GtkEntryBuffer *buffentryBrightDown;
	buffentryBrightDown = gtk_entry_get_buffer(GTK_ENTRY(entryBrightDown));
	const char *entryBrightDownText = gtk_entry_buffer_get_text(buffentryBrightDown);

	GtkEntryBuffer *buffentryLay1;
	buffentryLay1 = gtk_entry_get_buffer(GTK_ENTRY(entryLayout1));
	const char *buffentryLay1Text = gtk_entry_buffer_get_text(buffentryLay1);
	replacenthline(ConfigPathFile, 1, (const char *)buffentryLay1Text);

	GtkEntryBuffer *buffentryLay2;
	buffentryLay2 = gtk_entry_get_buffer(GTK_ENTRY(entryLayout2));
	const char *buffentryLay2Text = gtk_entry_buffer_get_text(buffentryLay2);
	replacenthline(ConfigPathFile, 2, (const char *)buffentryLay2Text);

	GtkEntryBuffer *buffentryLay3;
	buffentryLay3 = gtk_entry_get_buffer(GTK_ENTRY(entryLayout3));
	const char *buffentryLay3Text = gtk_entry_buffer_get_text(buffentryLay3);
	replacenthline(ConfigPathFile, 3, (const char *)buffentryLay3Text);

	const char *volUpText[777];
	snprintf((char *)volUpText, sizeof(volUpText), "volumeup=%s", entryVolUpText);
	replacenthline(pathToConfig, 5, (const char *)volUpText);

	const char *volDownText[777];
	snprintf((char *)volDownText, sizeof(volDownText), "volumedown=%s", entryVolDownText);
	replacenthline(pathToConfig, 6, (const char *)volDownText);

	const char *brightupText[777];
	snprintf((char *)brightupText, sizeof(brightupText), "brightup=%s", entryBrightUpText);
	replacenthline(pathToConfig, 7, (const char *)brightupText);

	const char *brighDownText[777];
	snprintf((char *)brighDownText, sizeof(brighDownText), "brightdown=%s", entryBrightDownText);
	replacenthline(pathToConfig, 8, (const char *)brighDownText);

	gtk_expression_watch_unwatch(watchWidth);
	gtk_expression_watch_unwatch(watchHeight);
	gtk_expression_watch_unwatch(watchPosy);
	gtk_expression_watch_unwatch(watchPosx);

	gtk_window_close(GTK_WINDOW(window));
}

/// open settings on right click on panel
static void on_right_click(GtkWidget *window) {
	const char *dirConfigPathFile = "/.config/diopanel/diopanelapplets/keyboardlayouts.conf";
	char 		ConfigPathFile[700];

	snprintf(ConfigPathFile, sizeof(ConfigPathFile), "%s%s", HOME, dirConfigPathFile);

	gint width				= get_int_value_from_conf(pathToConfig, "width");
	gint height				= get_int_value_from_conf(pathToConfig, "height");
	gint posx				= get_int_value_from_conf(pathToConfig, "posx");
	gint posy				= get_int_value_from_conf(pathToConfig, "posy");
	const gchar *volup		= get_char_value_from_conf(pathToConfig, "volumeup");
	const gchar *voldown	= get_char_value_from_conf(pathToConfig, "volumedown");
	const gchar *brightup	= get_char_value_from_conf(pathToConfig, "brightup");
	const gchar *brightdown	= get_char_value_from_conf(pathToConfig, "brightdown");

	//GtkWidget *window;
	window = gtk_window_new();
	gtk_window_set_title(GTK_WINDOW(window), "DioMenu Settings");
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 0);
	gtk_window_set_icon_name(GTK_WINDOW(window), "application-x-deb");

	g_signal_connect_swapped(window, "close-request", G_CALLBACK(close_request_cb), NULL);

	// panel width
	GtkWidget *labelWidth;
	labelWidth = gtk_label_new("Panel Width");

	GtkAdjustment *adjustmentSizeWidth;
	adjustmentSizeWidth = gtk_adjustment_new(width, 0, 700, 1, 1, 1);

	GtkWidget *entrySizeWidth;
	entrySizeWidth = gtk_spin_button_new(adjustmentSizeWidth, 1, 0);

	// panel height
	GtkWidget *labelHeight;
	labelHeight = gtk_label_new("Panel Height");

	GtkAdjustment *adjustmentSizeHeight;
	adjustmentSizeHeight = gtk_adjustment_new(height, 0, 700, 1, 1, 1);

	GtkWidget *entrySizeHeight;
	entrySizeHeight = gtk_spin_button_new(adjustmentSizeHeight, 1, 0);

	GtkWidget *boxWidthHeight;
	boxWidthHeight = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(boxWidthHeight), TRUE);
	gtk_box_prepend(GTK_BOX(boxWidthHeight), labelWidth);
	gtk_box_append(GTK_BOX(boxWidthHeight), entrySizeWidth);
	gtk_box_append(GTK_BOX(boxWidthHeight), labelHeight);
	gtk_box_append(GTK_BOX(boxWidthHeight), entrySizeHeight);

	// panel position x
	GtkWidget *labelX;
	labelX = gtk_label_new("Position X");

	GtkAdjustment *adjustmentPosx;
	adjustmentPosx = gtk_adjustment_new(posx, 0, 4000, 1, 1, 1);

	GtkWidget *entryPosx;
	entryPosx = gtk_spin_button_new(adjustmentPosx, 1, 0);

	// panel position y
	GtkWidget *labelY;
	labelY = gtk_label_new("Position Y");

	GtkAdjustment *adjustmentPosy;
	adjustmentPosy = gtk_adjustment_new(posy, 0, 4000, 1, 1, 1);

	GtkWidget *entryPosy;
	entryPosy = gtk_spin_button_new(adjustmentPosy, 1, 0);

	GtkWidget *boxPosition;
	boxPosition = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(boxPosition), TRUE);
	gtk_box_prepend(GTK_BOX(boxPosition), labelX);
	gtk_box_append(GTK_BOX(boxPosition), entryPosx);
	gtk_box_append(GTK_BOX(boxPosition), labelY);
	gtk_box_append(GTK_BOX(boxPosition), entryPosy);

	GtkWidget *sizePosition;
	sizePosition = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(sizePosition),
		"<span size=\"12pt\" weight=\"bold\">Size and Position</span>");

	GtkWidget *volumeAdjust;
	volumeAdjust = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(volumeAdjust),
		"<span size=\"12pt\" weight=\"bold\">Volume up and down</span>");

	GtkWidget *volumeUpLabel;
	volumeUpLabel = gtk_label_new("Volume up CMD\n(default or type your own command)");

	GtkEntryBuffer *buffVolUp;
	buffVolUp = gtk_entry_buffer_new(volup, -1);

	entryVolUp = gtk_entry_new_with_buffer(buffVolUp);

	GtkWidget *boxVolUp;
	boxVolUp = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(boxVolUp), TRUE);
	gtk_box_prepend(GTK_BOX(boxVolUp), volumeUpLabel);
	gtk_box_append(GTK_BOX(boxVolUp), entryVolUp);

	GtkWidget *volumeDownLabel;
	volumeDownLabel = gtk_label_new("Volume down CMD\n(default or type your own command)");

	GtkEntryBuffer *buffVolDown;
	buffVolDown = gtk_entry_buffer_new(voldown, -1);

	entryVolDown = gtk_entry_new_with_buffer(buffVolDown);

	GtkWidget *boxVolDown;
	boxVolDown = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(boxVolDown), TRUE);
	gtk_box_prepend(GTK_BOX(boxVolDown), volumeDownLabel);
	gtk_box_append(GTK_BOX(boxVolDown), entryVolDown);

	GtkWidget *brightnessAdjust;
	brightnessAdjust = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(brightnessAdjust),
		"<span size=\"12pt\" weight=\"bold\">Brightness up and down</span>");

	GtkWidget *brightnessUpLabel;
	brightnessUpLabel = gtk_label_new("Brightness up CMD\n(default or type your own command)");

	GtkEntryBuffer *buffBrightUp;
	buffBrightUp = gtk_entry_buffer_new(brightup, -1);

	entryBrightUp = gtk_entry_new_with_buffer(buffBrightUp);

	GtkWidget *boxBrightUp;
	boxBrightUp = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(boxBrightUp), TRUE);
	gtk_box_prepend(GTK_BOX(boxBrightUp), brightnessUpLabel);
	gtk_box_append(GTK_BOX(boxBrightUp), entryBrightUp);

	GtkWidget *brightnessDownLabel;
	brightnessDownLabel = gtk_label_new("Brightness down CMD\n(default or type your own command)");

	GtkEntryBuffer *buffBrightDown;
	buffBrightDown = gtk_entry_buffer_new(brightdown, -1);

	entryBrightDown = gtk_entry_new_with_buffer(buffBrightDown);

	GtkWidget *boxBrightDown;
	boxBrightDown = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(boxBrightDown), TRUE);
	gtk_box_prepend(GTK_BOX(boxBrightDown), brightnessDownLabel);
	gtk_box_append(GTK_BOX(boxBrightDown), entryBrightDown);

	GtkWidget *kbdLabel;
	kbdLabel = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(kbdLabel),
		"<span size=\"12pt\" weight=\"bold\">Keyboard Layouts</span>");

	GtkWidget *layLabel1;
	layLabel1 = gtk_label_new("Layout1:");

	gchar *layout1 = return_nth_line(ConfigPathFile, 1);

	GtkEntryBuffer *buffLayout1;
	buffLayout1 = gtk_entry_buffer_new(layout1, -1);

	entryLayout1 = gtk_entry_new_with_buffer(buffLayout1);

	GtkWidget *layLabel2;
	layLabel2 = gtk_label_new("Layout2:");

	gchar *layout2 = return_nth_line(ConfigPathFile, 2);

	GtkEntryBuffer *buffLayout2;
	buffLayout2 = gtk_entry_buffer_new(layout2, -1);

	entryLayout2 = gtk_entry_new_with_buffer(buffLayout2);

	GtkWidget *layLabel3;
	layLabel3 = gtk_label_new("Layout3:");

	gchar *layout3 = return_nth_line(ConfigPathFile, 3);

	GtkEntryBuffer *buffLayout3;
	buffLayout3 = gtk_entry_buffer_new(layout3, -1);

	entryLayout3 = gtk_entry_new_with_buffer(buffLayout3);

	GtkWidget *boxKBD;
	boxKBD = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(boxKBD), TRUE);
	gtk_box_prepend(GTK_BOX(boxKBD), layLabel1);
	gtk_box_append(GTK_BOX(boxKBD), entryLayout1);
	gtk_box_append(GTK_BOX(boxKBD), layLabel2);
	gtk_box_append(GTK_BOX(boxKBD), entryLayout2);
	gtk_box_append(GTK_BOX(boxKBD), layLabel3);
	gtk_box_append(GTK_BOX(boxKBD), entryLayout3);

	GtkWidget *removeButton;
	removeButton = gtk_button_new_with_label("Remove this panel");
	gtk_widget_set_halign(removeButton, GTK_ALIGN_CENTER);
	gtk_button_set_has_frame(GTK_BUTTON(removeButton), TRUE);

	g_signal_connect_swapped(removeButton, "clicked", G_CALLBACK(remove_button), window);

	GtkWidget *closeButton;
	closeButton = gtk_button_new_with_label("Close");
	gtk_widget_set_halign(closeButton, GTK_ALIGN_CENTER);
	gtk_button_set_has_frame(GTK_BUTTON(closeButton), TRUE);

	g_signal_connect_swapped(closeButton, "clicked", G_CALLBACK(close_button), window);

	// main box that contains both vertical boxes
	GtkWidget *boxAll;
	boxAll = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
	gtk_box_set_homogeneous(GTK_BOX(boxAll), TRUE);
	gtk_box_prepend(GTK_BOX(boxAll), sizePosition);
	gtk_box_append(GTK_BOX(boxAll), boxWidthHeight);
	gtk_box_append(GTK_BOX(boxAll), boxPosition);
	gtk_box_append(GTK_BOX(boxAll), volumeAdjust);
	gtk_box_append(GTK_BOX(boxAll), boxVolUp);
	gtk_box_append(GTK_BOX(boxAll), boxVolDown);
	gtk_box_append(GTK_BOX(boxAll), brightnessAdjust);
	gtk_box_append(GTK_BOX(boxAll), boxBrightUp);
	gtk_box_append(GTK_BOX(boxAll), boxBrightDown);
	gtk_box_append(GTK_BOX(boxAll), kbdLabel);
	gtk_box_append(GTK_BOX(boxAll), boxKBD);
	gtk_box_append(GTK_BOX(boxAll), removeButton);
	gtk_box_append(GTK_BOX(boxAll), closeButton);
	gtk_widget_set_margin_start(boxAll, 40);
	gtk_widget_set_margin_end(boxAll, 50);
	gtk_widget_set_margin_top(boxAll, 10);
	gtk_widget_set_margin_bottom(boxAll, 10);

	expressionHeight = gtk_property_expression_new(GTK_TYPE_ADJUSTMENT, NULL, "value");
	watchHeight = gtk_expression_watch(expressionHeight, adjustmentSizeHeight, notify, NULL, NULL);

	expressionWidth = gtk_property_expression_new(GTK_TYPE_ADJUSTMENT, NULL, "value");
	watchWidth = gtk_expression_watch(expressionWidth, adjustmentSizeWidth, notify, NULL, NULL);

	expressionPosy = gtk_property_expression_new(GTK_TYPE_ADJUSTMENT, NULL, "value");
	watchPosy = gtk_expression_watch(expressionPosy, adjustmentPosy, notify, NULL, NULL);

	expressionPosx = gtk_property_expression_new(GTK_TYPE_ADJUSTMENT, NULL, "value");
	watchPosx = gtk_expression_watch(expressionPosx, adjustmentPosx, notify, NULL, NULL);

	gtk_window_set_child(GTK_WINDOW(window), boxAll);
	gtk_window_present(GTK_WINDOW(window));
}

/// changing brightness on scroll
static void on_battery_scroll(GtkEventController *batteryEvent) {
	GdkEvent *eventController;
	eventController = gtk_event_controller_get_current_event(batteryEvent);

	char *entryBrightUpText = get_char_value_from_conf(pathToConfig, "brightup");
	char *entryBrightDownText = get_char_value_from_conf(pathToConfig, "brightdown");

	switch (gdk_scroll_event_get_direction(eventController)) {
		case GDK_SCROLL_UP:
			if (strcmp(entryBrightUpText, "default") == 0) {
				brightness(GDK_SCROLL_UP);
				break;
			}
			else {
				run_cmd(entryBrightUpText);
				break;
			}
		case GDK_SCROLL_DOWN:
			if (strcmp(entryBrightDownText, "default") == 0) {
				brightness(GDK_SCROLL_DOWN);
            	break;
            	}
			else {
				run_cmd(entryBrightDownText);
				break;
			}
		default:
			printf("Invalid choice.\n");
			break;
	}
}

/// changing volume on scroll
static void on_volume_scroll(GtkEventController *volumeEvent) {
	char *cmd = "pactl list sinks | grep 'Volume:' | head -n 1 | awk '{print $5}' | sed 's/%//'";
	char *outputVol = output_to_char(cmd);
	int currentVolume = atoi(outputVol);
	printf("current volume %d\n", currentVolume);

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

	GdkEvent *eventController;
	eventController = gtk_event_controller_get_current_event(volumeEvent);

	char *entryVolumeUpText = get_char_value_from_conf(pathToConfig, "volumeup");
	char *entryVolumeDownText = get_char_value_from_conf(pathToConfig, "volumedown");

	switch (gdk_scroll_event_get_direction(eventController)) {
		case GDK_SCROLL_UP:
			if (strcmp(entryVolumeUpText, "default") == 0) {
				volume(GDK_SCROLL_UP);
				break;
			}
			else {
				run_cmd(entryVolumeUpText);
				break;
			}
		case GDK_SCROLL_DOWN:
			if (strcmp(entryVolumeDownText, "default") == 0) {
				volume(GDK_SCROLL_DOWN);
            	break;
            	}
			else {
				run_cmd(entryVolumeDownText);
				break;
			}
		default:
			printf("Invalid choice.\n");
			break;
	}
	free(outputVol);
}

static gboolean get_time_each_minute(gpointer data) {
	// set icon according to current layout
	currentLayoutN = output_to_char("setxkbmap -query | \
											grep layout | \
											awk '{print $2}' | \
											head -c 3");

	gtk_label_set_label(GTK_LABEL(inputIcon), currentLayoutN);

	// changing time every 3 sec
	gchar *currenttime = getcurrenttime();
	gtk_label_set_label(GTK_LABEL(data), currenttime);

	if (state == FALSE) {
		g_source_remove(threadID);
		threadID = 0;
		g_print("loop over\n");
		return state;
	}
	else {
		return state;
	}
	free(currenttime);
	free(currentLayoutN);
}

/// activate signal handle
static void activate(GtkApplication *app) {
	int width = get_int_value_from_conf(pathToConfig, "width");
	int height = get_int_value_from_conf(pathToConfig, "height");
	int posy = get_int_value_from_conf(pathToConfig, "posy");
	int posx = get_int_value_from_conf(pathToConfig, "posx");

	GtkWidget *window;
	window = gtk_application_window_new(GTK_APPLICATION(app));

	//******* netwoek widgei
	networkIcon = gtk_image_new();

	// getting wifi state
	wifi_state = output_to_char("nmcli radio  wifi");
	wifi_connection_check = output_to_char("nmcli networking connectivity check");

	if (strcmp(wifi_state, "enabled") == 0 && strcmp(wifi_connection_check, "full") == 0) {
		gtk_image_set_from_icon_name(GTK_IMAGE(networkIcon), "network-wireless");
	}
	else {
		gtk_image_set_from_icon_name(GTK_IMAGE(networkIcon), "network-wired-disconnected-symbolic");
	}
	gtk_image_set_icon_size(GTK_IMAGE(networkIcon), GTK_ICON_SIZE_NORMAL);

	// adds left click action for network widget
	GtkGesture *networkApp;
	networkApp = gtk_gesture_click_new();
	GtkGestureSingle *networkAppClick;
	networkAppClick = GTK_GESTURE_SINGLE(networkApp);
	gtk_gesture_single_set_button(networkAppClick, 1);
	GtkEventController *networkAppLClick;
	networkAppLClick = GTK_EVENT_CONTROLLER(networkApp);
	gtk_event_controller_set_propagation_phase(networkAppLClick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(networkIcon, networkAppLClick);

	g_signal_connect_swapped(networkAppClick, "pressed", G_CALLBACK(on_network_l_click), NULL);

	//******* notes widgei
	GtkWidget *notesIcon;
	notesIcon = gtk_image_new_from_icon_name("text-editor");
	gtk_image_set_icon_size(GTK_IMAGE(notesIcon), GTK_ICON_SIZE_NORMAL);

	// adds left click action for notes
	GtkGesture *notesApp;
	notesApp = gtk_gesture_click_new();
	GtkGestureSingle *notesAppClick;
	notesAppClick = GTK_GESTURE_SINGLE(notesApp);
	gtk_gesture_single_set_button(notesAppClick, 1);
	GtkEventController *notesAppRClick;
	notesAppRClick = GTK_EVENT_CONTROLLER(notesApp);
	gtk_event_controller_set_propagation_phase(notesAppRClick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(notesIcon, notesAppRClick);

	g_signal_connect_swapped(notesAppClick, "pressed", G_CALLBACK(on_notes_l_click), NULL);

	//******* keyboard widget
	inputIcon = gtk_label_new(NULL);

	// set icon according to current layout
	currentLayoutN = output_to_char("setxkbmap -query | \
											grep layout | \
											awk '{print $2}' | \
											head -c 3");

	gtk_label_set_label(GTK_LABEL(inputIcon), currentLayoutN);

	// adds left click action for keyboard layout
	GtkGesture *inputScale;
	inputScale = gtk_gesture_click_new();
	GtkGestureSingle *inputScaleClick;
	inputScaleClick = GTK_GESTURE_SINGLE(inputScale);
	gtk_gesture_single_set_button(inputScaleClick, 1);
	GtkEventController *inputRClick;
	inputRClick = GTK_EVENT_CONTROLLER(inputScale);
	gtk_event_controller_set_propagation_phase(inputRClick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(inputIcon, inputRClick);

	g_signal_connect_swapped(inputScaleClick, "pressed", G_CALLBACK(on_input_r_click), NULL);

	// customizing the style, text size of the GtkEntry Widget
	PangoAttrList *attr_list_KBD = pango_attr_list_new();
	PangoAttribute *attrKBD = pango_attr_size_new_absolute(17 * PANGO_SCALE);
	pango_attr_list_insert(attr_list_KBD, attrKBD);
	PangoLayout *layoutKBD = gtk_widget_create_pango_layout(inputIcon, NULL);
	pango_layout_set_attributes(layoutKBD, attr_list_KBD);
	gtk_label_set_attributes(GTK_LABEL(inputIcon), attr_list_KBD);

	//******* clock widget
	gchar *currenttime = getcurrenttime();

	GtkWidget *clockIcon;
	clockIcon = gtk_label_new(currenttime);

	threadID = g_timeout_add_seconds(3, get_time_each_minute, clockIcon);

	// customizing the style, text size of clock Widget
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(17 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoLayout *layout = gtk_widget_create_pango_layout(clockIcon, NULL);
	pango_layout_set_attributes(layout, attr_list);
	gtk_label_set_attributes(GTK_LABEL(clockIcon), attr_list);

	// adds left click action for clock widget
	GtkGesture *clockApp;
	clockApp = gtk_gesture_click_new();
	GtkGestureSingle *clockAppClick;
	clockAppClick = GTK_GESTURE_SINGLE(clockApp);
	gtk_gesture_single_set_button(clockAppClick, 1);
	GtkEventController *clockAppLClick;
	clockAppLClick = GTK_EVENT_CONTROLLER(clockApp);
	gtk_event_controller_set_propagation_phase(clockAppLClick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(clockIcon, clockAppLClick);

	g_signal_connect_swapped(clockAppLClick, "pressed", G_CALLBACK(on_clock_r_click), NULL);

	//******* battery/brightness widget
	batteryIcon = gtk_image_new();
	gtk_image_set_icon_size(GTK_IMAGE(batteryIcon), GTK_ICON_SIZE_NORMAL);
	gtk_image_set_from_icon_name(GTK_IMAGE(batteryIcon), "display-brightness-symbolic");

	// adds left click action for brightness scale
	GtkGesture *brightScale;
	brightScale = gtk_gesture_click_new();
	GtkGestureSingle *brightScaleClick;
	brightScaleClick = GTK_GESTURE_SINGLE(brightScale);
	gtk_gesture_single_set_button(brightScaleClick, 1);
	GtkEventController *brightRClick;
	brightRClick = GTK_EVENT_CONTROLLER(brightScale);
	gtk_event_controller_set_propagation_phase(brightRClick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(batteryIcon, brightRClick);

	g_signal_connect_swapped(brightScaleClick, "pressed", G_CALLBACK(on_battery_r_click), NULL);

	// adds scroll action for brightness/battery
	GtkEventController *batteryEvent;
	batteryEvent = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
	gtk_widget_add_controller(batteryIcon, batteryEvent);

	g_signal_connect(batteryEvent, "scroll", G_CALLBACK(on_battery_scroll), NULL);

	// ******* volume icon
	volumeIcon = gtk_image_new();

	char *cmd = "pactl list sinks | grep 'Volume:' | head -n 1 | awk '{print $5}' | sed 's/%//'";
	gchar *outputCurrVol = output_to_char(cmd);
	int currentVolume = atoi(outputCurrVol);

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
	gtk_image_set_icon_size(GTK_IMAGE(volumeIcon), GTK_ICON_SIZE_NORMAL);

	// adds left click action for volume scale
	GtkGesture *volumeScale;
	volumeScale = gtk_gesture_click_new();
	GtkGestureSingle *volumeScaleClick;
	volumeScaleClick = GTK_GESTURE_SINGLE(volumeScale);
	gtk_gesture_single_set_button(volumeScaleClick, 1);
	GtkEventController *volumeRClick;
	volumeRClick = GTK_EVENT_CONTROLLER(volumeScale);
	gtk_event_controller_set_propagation_phase(volumeRClick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(volumeIcon, volumeRClick);

	g_signal_connect_swapped(volumeScaleClick, "pressed", G_CALLBACK(on_volume_r_click), NULL);

	// adds scroll action for volume
	GtkEventController *volumeEvent;
	volumeEvent = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);

	g_signal_connect(volumeEvent, "scroll", G_CALLBACK(on_volume_scroll), NULL);

	GtkWidget *popContext;
	popContext = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
	gtk_box_set_homogeneous(GTK_BOX(popContext), TRUE);
	gtk_box_prepend(GTK_BOX(popContext), networkIcon);
	gtk_box_append(GTK_BOX(popContext), notesIcon);
	gtk_box_append(GTK_BOX(popContext), inputIcon);
	gtk_box_append(GTK_BOX(popContext), clockIcon);
	gtk_box_append(GTK_BOX(popContext), batteryIcon);
	gtk_box_append(GTK_BOX(popContext), volumeIcon);

	// the panel bar itself
	pop = gtk_popover_new();
	gtk_widget_set_size_request(pop, width, height);
	gtk_popover_set_autohide(GTK_POPOVER(pop), FALSE);
	gtk_popover_set_offset(GTK_POPOVER(pop), posx, posy);
	gtk_popover_set_has_arrow(GTK_POPOVER(pop), FALSE);
	gtk_popover_set_child(GTK_POPOVER(pop), popContext);
	gtk_widget_add_controller(pop, volumeEvent);

	GtkWidget *box;
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_prepend(GTK_BOX(box), pop);

	// adds right click action for new panel
	GtkGesture *lclick;
	lclick = gtk_gesture_click_new();

	GtkGestureSingle *singleclick;
	singleclick = GTK_GESTURE_SINGLE(lclick);

	gtk_gesture_single_set_button(singleclick, 3);

	GtkEventController *leftclick;
	leftclick = GTK_EVENT_CONTROLLER(lclick);
	gtk_event_controller_set_propagation_phase(leftclick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(pop, leftclick);

	g_signal_connect_swapped(leftclick, "pressed", G_CALLBACK(on_right_click), window);

	gtk_window_set_child(GTK_WINDOW(window), box);
	gtk_popover_popup(GTK_POPOVER(pop));

	free(outputCurrVol);
	free(currentLayoutN);
	g_object_unref(layout);
	free(wifi_connection_name);
	free(wifi_connection_check);
	pango_attr_list_unref(attr_list);
	g_object_unref(layoutKBD);
	pango_attr_list_unref(attr_list_KBD);
}

int main() {
	// deifning HOME
	HOME = getenv("HOME");

	// creating initial configs
	create_configs();

	// setting config file
	snprintf(pathToConfig, 777, "%s%s", HOME, configName);

	gint status;
	app = gtk_application_new("com.github.DiogenesN.diopanelapplets", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), 0, NULL);
	g_object_unref(app);

	return status;
}


