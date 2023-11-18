//*** changing layouts from widget
// man xkeyboard-config (to see available layouts)

#include <glib.h>
#include <gtk/gtk.h>
#include "externvars.h"
#include "outputtochar.h"
#include "getlinenrcontainingstring.h"

GtkWidget *popInput;
GtkSingleSelection *kbdSingleSelection;

/// when kbdlayout autohides close the window to prevent memory leak
static void on_kbdlayout_close(GtkWidget *window) {
	// Free the allocated memory for each line
	for (int i = 0; i < 3; i++) {
		free(layouts[i]);
	}
	gtk_window_close(GTK_WINDOW(window));
}

// creating labels to fill up the listview
static void setup_listitem(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory; // Marking the parameter as unused
	GtkWidget *label;
	label = gtk_label_new(NULL);

	// defining the size of text in label
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(20 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoLayout *layout = gtk_widget_create_pango_layout(label, NULL);
	pango_layout_set_attributes(layout, attr_list);

	gtk_label_set_attributes(GTK_LABEL(label), attr_list);

	// adds GtkBox as child of GtkListItem
	gtk_list_item_set_child(list_item, label);
	
	// freing resources
	g_object_unref(layout);
	pango_attr_list_unref(attr_list);
}

// binding the items to the list, this function runs in a loop
static void bind_listitem(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory; // Marking the parameter as unused

	// Get the GtkBox widget
	GtkWidget *label;
	label = gtk_list_item_get_child(list_item);

	// gets the strings from GtkStringList *myStrings in startup function
	GtkStringObject *obj = gtk_list_item_get_item(list_item);
	const char *strings = gtk_string_object_get_string(GTK_STRING_OBJECT(obj));

	// sets label text to strings one at a time
	gtk_label_set_label(GTK_LABEL(label), strings);
}

static void kbd_change(GtkSingleSelection *selection) {
	// getting selected item in the keyboard layout chooser
	GtkStringObject *strObj;
	strObj = gtk_single_selection_get_selected_item(selection);
	const gchar* layout = gtk_string_object_get_string(strObj);

	gchar cmd[100];
	snprintf(cmd, sizeof(cmd), "setxkbmap -layout \"%s\"", layout);

	// setting the keyboard layout to the selected item when clicking
	run_cmd(cmd);

	gtk_popover_popdown(GTK_POPOVER(popInput));

	// set icon according to current layout
	const gchar *currentLayoutN;
	currentLayoutN = output_to_char("setxkbmap -query | \
									grep layout | \
									awk '{print $2}' | \
									head -c 2");

	gtk_label_set_label(GTK_LABEL(inputIcon), currentLayoutN);
	free((void *)currentLayoutN);
}

/// opening keyboars layouts
static void on_input_r_click() {
	// creating config file
	gint counter_index = 0;

	const char *dirConfigPathFile = "/.config/diopanel/diopanelapplets/keyboardlayouts.conf";
	char 		ConfigPathFile[700];

	snprintf(ConfigPathFile, sizeof(ConfigPathFile), "%s%s", HOME, dirConfigPathFile);

	FILE *file = fopen(ConfigPathFile, "r");

	// if config doesn't exist then create it
	if (file == NULL) {
		g_print("created config file\n");
		file = fopen(ConfigPathFile, "w+");
		fprintf(file, "%s\n", "us");
		fclose(file);
	}
	else {
		while (fgets(line, 100, file) != NULL) {
			// Remove the newline character, if present
			char *newline = strchr(line, '\n');
			if (newline != NULL) {
				*newline = '\0';
			}

			// Allocate memory for the string and copy it
			layouts[counter_index] = strdup(line);
			counter_index++;

			if (counter_index >= 3) {
				//printf("3 items limit reached\n");
				break;  // Stop reading after 30 lines
			}
		}
		
		fclose(file);
	}

	GtkWidget *window;
	window = gtk_window_new();

	GdkRectangle rect;
    rect.x = get_int_value_from_conf(pathToConfig, "posx") - 25;
    rect.y = get_int_value_from_conf(pathToConfig, "posy") - 123;
    rect.width = 0;
    rect.height = 0;

	// turns all strings into objects
	GtkStringList *myStrings;
	myStrings = gtk_string_list_new((const char * const *)layouts);

	// creates a list model from the object of string objects
	GListModel *myGlistModel;
	myGlistModel = G_LIST_MODEL(myStrings);

	// creats a list item factory to setup and bind the string object to the list view
	GtkListItemFactory *myGtkListItemFactory;
	myGtkListItemFactory = gtk_signal_list_item_factory_new();

	// callbacks (funtions) to run on signals that widgets emit
	g_signal_connect(myGtkListItemFactory, "setup", G_CALLBACK(setup_listitem), NULL);
	g_signal_connect(myGtkListItemFactory, "bind", G_CALLBACK(bind_listitem), NULL);

	// defining how selection of items behaves
	kbdSingleSelection = gtk_single_selection_new(G_LIST_MODEL(myGlistModel));

	GtkSelectionModel *mySelectionModel;
	mySelectionModel = GTK_SELECTION_MODEL(kbdSingleSelection);

	// creating the list view to display the list items but it needs to be a child of a widget
	GtkWidget *myGtkListView;
	myGtkListView = gtk_list_view_new(mySelectionModel, myGtkListItemFactory);
	gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(myGtkListView), TRUE);

	g_signal_connect_swapped(myGtkListView, "activate", G_CALLBACK(kbd_change), kbdSingleSelection);

	// adding the list view as the child
	GtkWidget *myScrolledWindow;
	myScrolledWindow = gtk_scrolled_window_new();
	gtk_widget_set_size_request(myScrolledWindow, 100, 100);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(myScrolledWindow), myGtkListView);

	popInput = gtk_popover_new();
	gtk_popover_set_pointing_to(GTK_POPOVER(popInput), &rect);
	gtk_popover_set_autohide(GTK_POPOVER(popInput), TRUE);
	gtk_popover_set_has_arrow(GTK_POPOVER(popInput), FALSE);
	gtk_popover_set_child(GTK_POPOVER(popInput), myScrolledWindow);

	g_signal_connect_swapped(popInput, "closed", G_CALLBACK(on_kbdlayout_close), window);

	gtk_window_set_child(GTK_WINDOW(window), popInput);

	// set the item in the list to be selected according to current layout
	const gchar *currentLayout = output_to_char("setxkbmap -query | grep layout | awk '{print $2}'");
	gint lineNrOfLayout = get_line_number_containing_string(ConfigPathFile, (char *)currentLayout);
	gtk_selection_model_select_item(mySelectionModel, lineNrOfLayout, TRUE);

	free((void *)currentLayout);

	gtk_popover_popup(GTK_POPOVER(popInput));
}
