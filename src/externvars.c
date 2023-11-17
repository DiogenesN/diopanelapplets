#include <glib.h>
#include <gtk/gtk.h>

gchar line[100];
gchar *ssids[100];
const gchar *HOME;
gchar *wifi_state;
guint threadID = 0;
gchar *layouts[100];
GtkWidget *inputIcon;
gboolean state = TRUE;
GtkWidget *volumeIcon;
GtkWidget *networkIcon;
GtkWidget *batteryIcon;
float currentBrightness;
gchar pathToConfig[777];
gchar *wifi_connection_name;
gchar *wifi_connection_check;
GtkWidget *windowNotes = NULL;
GtkExpression *expressionSwitch;
GtkExpressionWatch *watchSwitch;
gchar *configName = "/.config/diopanel/diopanelapplets/diopanelapplets.conf";
