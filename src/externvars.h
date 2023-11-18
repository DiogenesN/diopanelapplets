#include <glib.h>
#include <gtk/gtk.h>

#ifndef EXTERNVARS_H_
#define EXTERNVARS_H_

	extern gchar line[];
	extern gchar *ssids[];
	extern gboolean state;
	extern guint threadID;
	extern gchar *layouts[];
	extern const gchar *HOME;
	extern gchar pathToConfig[];
	extern GtkWidget *inputIcon;
	extern GtkWidget *volumeIcon;
	extern GtkWidget *networkIcon;
	extern GtkWidget *batteryIcon;
	extern GtkWidget *windowNotes;
	extern float currentBrightness;
	extern const gchar *configName;
	extern const gchar *wifi_state;
	extern GtkExpression *expressionSwitch;
	extern GtkExpressionWatch *watchSwitch;
	extern const gchar *wifi_connection_name;
	extern const gchar *wifi_connection_check;

#endif
