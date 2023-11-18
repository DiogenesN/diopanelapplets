#include <glib.h>
#include <gtk/gtk.h>

gchar line[100];
gchar *ssids[100];
const gchar *HOME;
guint threadID = 0;
gchar *layouts[100];
GtkWidget *inputIcon;
gboolean state = TRUE;
GtkWidget *volumeIcon;
GtkWidget *networkIcon;
GtkWidget *batteryIcon;
float currentBrightness;
gchar pathToConfig[777];
const gchar *wifi_state;
GtkWidget *windowNotes = NULL;
GtkExpression *expressionSwitch;
GtkExpressionWatch *watchSwitch;
const gchar *wifi_connection_name;
const gchar *wifi_connection_check;
gchar *configName = "/.config/diopanel/diopanelapplets/diopanelapplets.conf";
