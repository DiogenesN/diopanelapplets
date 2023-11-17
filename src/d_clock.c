#include <glib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "externvars.h"
#include "getvaluefromconf.h"

/// when calendar autohides close the window to prevent memory leak
static void on_caledar_close(GtkWidget *window) {
	gtk_window_close(GTK_WINDOW(window));
}

static void on_clock_r_click() {
	GtkWidget *window;
	window = gtk_window_new();

	GdkRectangle rect;
	rect.x = get_int_value_from_conf(pathToConfig, "posx") + 20;
	rect.y = get_int_value_from_conf(pathToConfig, "posy") - 3;
	rect.width = 0;
	rect.height = 0;

	GtkWidget *calendar;
	calendar = gtk_calendar_new();
	gtk_widget_set_size_request(calendar, 200, 200);

	GtkWidget *popCalendar;
	popCalendar = gtk_popover_new();
	gtk_popover_set_pointing_to(GTK_POPOVER(popCalendar), &rect);
	gtk_popover_set_autohide(GTK_POPOVER(popCalendar), TRUE);
	gtk_popover_set_has_arrow(GTK_POPOVER(popCalendar), FALSE);
	gtk_popover_set_child(GTK_POPOVER(popCalendar), calendar);

	g_signal_connect_swapped(popCalendar, "closed", G_CALLBACK(on_caledar_close), window);

	gtk_window_set_child(GTK_WINDOW(window), popCalendar);
	gtk_popover_popup(GTK_POPOVER(popCalendar));
}

static gchar *getcurrenttime() {
	// returns microseconds
	gint64 realtime = g_get_real_time();

	// Convert microseconds to seconds
	gint64 seconds = realtime / G_USEC_PER_SEC;

	GDateTime *datetime;
	datetime = g_date_time_new_from_unix_local(seconds);

	gchar *currenttime = g_date_time_format(datetime, "%R");

	g_date_time_unref(datetime);

	return currenttime;
}

