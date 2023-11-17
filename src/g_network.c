//*** network ***//
/* NOTE: there are a few bugs in nmcli
 * 1) sometimes scanning for networks will double the last entry in the list
 * 2) if you have already a saved network and you manualy modify it
 		(e.g. it was previously free and now you set up a password)
 		then network-manager will create a duplicate file for that network config
 		make sure you remove it with: nm-connection-editor
 		because it will not assign 'disconnect' label correctly.
 */

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include "structs.h"
#include <gtk/gtk.h>
#include <sys/wait.h>
#include "externvars.h"
#include "outputtochar.h"
#include "subprocessrun.h"
#include "getvaluefromconf.h"
#include "outputtochararray.h"

GtkWidget *noWiFi;
GtkWidget *boxNet;
GtkWidget *labelSSID;
gchar *freeRnot[100];
GtkWidget *passWindow;
GtkWidget *labelSSIDS;
GtkWidget *popNetwork;
GtkWidget *netScrolled;
GtkWidget *refreshButton;
GtkWidget *hotSpotButton;
GtkStringList *networkList;
const gchar *connectCMD[256];
GtkSingleSelection *mySingleSelection;

static void connect_cb(GtkWidget *connectBtn);

// creating labels to fill up the listview
static void setup_listitem_net(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory;
	
	GtkWidget *connectBtn;
	connectBtn = gtk_button_new();
	gtk_widget_set_hexpand(connectBtn, FALSE);
	gtk_widget_set_vexpand(connectBtn, FALSE);
	gtk_widget_set_halign(connectBtn, GTK_ALIGN_START);
	gtk_button_set_has_frame(GTK_BUTTON(connectBtn), TRUE);

	g_signal_connect_swapped(connectBtn, "clicked", G_CALLBACK(connect_cb), connectBtn);

	labelSSID = gtk_label_new(NULL);

	// defining the size of text in label
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(17 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoLayout *layout = gtk_widget_create_pango_layout(labelSSID, NULL);
	pango_layout_set_attributes(layout, attr_list);

	gtk_label_set_attributes(GTK_LABEL(labelSSID), attr_list);

	GtkWidget *freeOrNot;
	freeOrNot = gtk_label_new("secured");
	
	GtkWidget *boxButtons;
	boxButtons = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_prepend(GTK_BOX(boxButtons), connectBtn);
	gtk_box_append(GTK_BOX(boxButtons), labelSSID);
	gtk_box_append(GTK_BOX(boxButtons), freeOrNot);
	gtk_widget_set_margin_start(boxButtons, 10);
	gtk_widget_set_margin_end(boxButtons, 10);

	// adds GtkBox as child of GtkListItem
	gtk_list_item_set_child(list_item, boxButtons);
	
	// freing resources
	g_object_unref(layout);
	pango_attr_list_unref(attr_list);
}

// binding the items to the list, this function runs in a loop
static void bind_listitem_net(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory;

	// Get the GtkBox widget
	GtkWidget *box;
	box = gtk_list_item_get_child(list_item);

	// Find the GtkImage widget within the GtkBox
	GtkWidget *connectBtn;
	connectBtn = gtk_widget_get_first_child(box);

	// Find the next child after GtkImage whoch is a label 
	GtkWidget *labelSSID;
	labelSSID = gtk_widget_get_next_sibling(connectBtn);

	// Find the next child after GtkImage whoch is a label 
	GtkWidget *freeOrNot;
	freeOrNot = gtk_widget_get_next_sibling(labelSSID);

	// gets the strings from GtkStringList *myStrings in startup function
	guint itemPos = gtk_list_item_get_position(list_item); // index of items

	// getttins names to populate the menu
	const gchar *stringsNames = gtk_string_list_get_string(p_dataOptions->stringsNames, itemPos);

	// setting disconnect label for the button of the current connected network
	if (strcmp(stringsNames, wifi_connection_name) == 0) {
		// sets button label for the active connection
		gtk_button_set_label(GTK_BUTTON(connectBtn), "disconnect");
		// sets label text according to SSIDs one at a time
		// defining the size of text in label
		PangoAttrList *attr_list = pango_attr_list_new();
		PangoAttribute *attr = pango_attr_size_new_absolute(22 * PANGO_SCALE);
		pango_attr_list_insert(attr_list, attr);
		PangoLayout *layout = gtk_widget_create_pango_layout(labelSSID, NULL);
		pango_layout_set_attributes(layout, attr_list);

		gtk_label_set_attributes(GTK_LABEL(labelSSID), attr_list);

		gtk_label_set_label(GTK_LABEL(labelSSID), stringsNames);
		gtk_label_set_label(GTK_LABEL(freeOrNot), freeRnot[itemPos]);

		// freing resources
		g_object_unref(layout);
		pango_attr_list_unref(attr_list);
	}
	else {
		gtk_button_set_label(GTK_BUTTON(connectBtn), "connect");

		// "full" means that it's currently connected to a network
		if (strcmp(wifi_connection_check, "full") == 0) {
			gtk_widget_set_sensitive(connectBtn, FALSE);
			gtk_widget_set_sensitive(labelSSID, FALSE);
		}
		// sets label text according to SSIDs one at a time
		gtk_label_set_label(GTK_LABEL(labelSSID), stringsNames);
		gtk_label_set_label(GTK_LABEL(freeOrNot), freeRnot[itemPos]);
	}
}

/// closing network app window
static void on_network_close(GtkWindow *window) {
	// getting wifi state
	wifi_state = output_to_char("nmcli radio  wifi");
	wifi_connection_check = output_to_char("nmcli networking connectivity check");

	if (strcmp(wifi_state, "enabled") == 0 && strcmp(wifi_connection_check, "full") == 0) {
		gtk_image_set_from_icon_name(GTK_IMAGE(networkIcon), "network-wireless");
	}
	else {
		gtk_image_set_from_icon_name(GTK_IMAGE(networkIcon), "network-wired-disconnected-symbolic");
	}

	for (int i = 0; ssids[i] != NULL; i++) {
		ssids[i] = NULL;
		free(ssids[i]);
		freeRnot[i] = NULL;
		free(freeRnot[i]);
	}

	wifi_state = NULL;
	free(wifi_state);
	wifi_connection_check = NULL;
	free(wifi_connection_check);
	wifi_connection_name = NULL;
	free(wifi_connection_name);
	gtk_window_close(window);
}

/// populating listview with SSIDS
gboolean waiting_ssids(gpointer data) {
	// if wifi switch is enabled
	if (strcmp(wifi_state, "enabled") == 0) {
		g_print("Available Networks\n");
		output_to_char_array(ssids, "nmcli --fields SSID device wifi | \
									sed '1d' | \
									awk '{$1=$1;print}'");
		
		output_to_char_array(freeRnot, "nmcli \
										--fields=SECURITY device wifi | \
										sed '1d' | \
							  			awk '{$1=$1;print}' | \
							  			sed 's/--/Free/g'");

		//g_print("found networks:\n");
		gtk_label_set_markup(GTK_LABEL(labelSSIDS),
			"<span size=\"12pt\" weight=\"bold\">Available Networks</span>");
		for (int i = 0; ssids[i] != NULL; i++) {
			//g_print("%s\n", ssids[i]);
			gtk_string_list_take(data, g_strdup_printf("%s", ssids[i]));
	}
    return G_SOURCE_REMOVE | FALSE;
    }
    else {
    	return G_SOURCE_REMOVE | FALSE;
    }
    return G_SOURCE_REMOVE | FALSE;
}

/// refreshing the network list
static void refresh_cb(GListModel *myGlistModel) {
	g_print("refreshing network list...\n");
	
	// setting button label
	wifi_connection_check = output_to_char("nmcli networking connectivity check");
	wifi_connection_name = output_to_char("nmcli \
										  --field=NAME \
										  connection \
										  show --active | \
										  awk \"NR==2\" | \
										  awk '{$1=$1;print}'");

	gtk_label_set_markup(GTK_LABEL(labelSSIDS),
		"<span size=\"12pt\" weight=\"bold\">Scanning please wait...</span>");
	gtk_string_list_splice(networkList,
							0,
							g_list_model_get_n_items(myGlistModel),
							NULL);
	g_timeout_add_seconds(1, waiting_ssids, networkList);
}

/// callback for connect button in the password prompt
static void pasconnect_cb(GtkEntryBuffer *passBuf) {
	g_print("connect\n");
	const gchar *passwd = gtk_entry_buffer_get_text(passBuf);

	// Calculate the required buffer size
	size_t buffer_size = snprintf(NULL, 0, "%s %s", (char *)connectCMD, (char *)passwd) + 14;

	// Allocate memory for the string
	char *cmdAndPsswdSize = (char *)malloc(buffer_size);

	if (cmdAndPsswdSize == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		return;
	}

	// Build the string using snprintf
	snprintf(cmdAndPsswdSize, buffer_size, "%s password \"%s\"", (char *)connectCMD, (char *)passwd);
	run_cmd(cmdAndPsswdSize);
	free(cmdAndPsswdSize);

	gtk_window_close(GTK_WINDOW(passWindow));
}

/// action on connecting to a network
static void connect_cb(GtkWidget *connectBtn) {
	GObject *obj;
	obj = gtk_single_selection_get_selected_item(mySingleSelection);

	const gchar *selectedNetwork = gtk_string_object_get_string(GTK_STRING_OBJECT(obj));
	g_print("selected network: %s\n", selectedNetwork);

	const gchar *connectLabel = gtk_button_get_label(GTK_BUTTON(connectBtn));
	g_print("button state: %s\n", connectLabel);

	const gchar disconnectCMD[256];
	const gchar chkIfSavedCMD[256];
	const gchar chkIfFreeCMD[256];

	snprintf((char *)connectCMD, sizeof(connectCMD), "nmcli device wifi connect \"%s\"",
																			selectedNetwork);

	snprintf((char *)disconnectCMD, sizeof(disconnectCMD), "nmcli connection down id \"%s\"",
																		selectedNetwork);
	snprintf((char *)chkIfSavedCMD, sizeof(chkIfSavedCMD), "nmcli connection show | \
																		grep -w -o \"%s\"",
																		selectedNetwork);
	snprintf((char *)chkIfFreeCMD, sizeof(chkIfFreeCMD), "nmcli --fields SSID,SECURITY device wifi\
															| sed 's/--/Free/g' | \
															grep -w \"%s\" | \
															sed 's/ *$//g' | \
															tail -c5", selectedNetwork);

	gboolean isSaved =  FALSE;
	gboolean isFree = FALSE;

	gchar *chkIfSaved;
	chkIfSaved = output_to_char((char *)chkIfSavedCMD);
	if (chkIfSaved == NULL) { chkIfSaved = "<::Not_Saved_Network::>"; };
	//g_print("chkIfSaved: %s\n", chkIfSaved);

	gchar *chkIfFree = output_to_char((char *)chkIfFreeCMD);
	//g_print("chkIfFree: %s\n", chkIfFree);

	// isSaved assigning bool variables according to output
	if (strcmp(chkIfSaved, selectedNetwork) == 0) {
		isSaved = TRUE;
	}
	else {
		isSaved = FALSE;
	}

	// isFree assigning bool variables according to output
	if (strcmp(chkIfFree, "Free") == 0) {
		isFree = TRUE;
	}
	else {
		isFree = FALSE;
	}

	//g_print("isSaved: %d\n", isSaved);
	//g_print("isFree: %d\n", isFree);

	// if connected then disconnect otherwise connect
	if (strcmp(connectLabel, "disconnect") == 0) {
		// disconnect selected network
		run_cmd((char *)disconnectCMD);
		g_signal_emit_by_name(refreshButton, "clicked");
		gtk_widget_set_sensitive(hotSpotButton, TRUE);
	}
	else {
		// connect selected network without passwd if it's saved or free
		if (isSaved || isFree) {
			run_cmd((char *)connectCMD);
			g_signal_emit_by_name(refreshButton, "clicked");
			gtk_widget_set_sensitive(hotSpotButton, FALSE);
		}
		else {
			gtk_widget_set_sensitive(hotSpotButton, FALSE);
			// the network is secured and needs password prompt
			passWindow = gtk_window_new();
			gtk_window_set_modal(GTK_WINDOW(passWindow), TRUE);
			gtk_window_set_title(GTK_WINDOW(passWindow), "Wifi Password");
			gtk_window_set_default_size(GTK_WINDOW(passWindow), 333, 0);
			gtk_window_set_icon_name(GTK_WINDOW(passWindow), "dialog-password-symbolic");

			GtkWidget *btnCancel;
			btnCancel = gtk_button_new_with_label("Cancel");
			gtk_widget_set_halign(btnCancel, GTK_ALIGN_START);
			gtk_button_set_has_frame(GTK_BUTTON(btnCancel), TRUE);

			g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_window_close), passWindow);

			GtkWidget *btnConnect;
			btnConnect = gtk_button_new_with_label("Connect");
			gtk_widget_set_halign(btnConnect, GTK_ALIGN_END);
			gtk_button_set_has_frame(GTK_BUTTON(btnConnect), TRUE);

			GtkEntryBuffer *passBuff;
			passBuff = gtk_entry_buffer_new(NULL, -1);

			GtkWidget *passPrompt;
			passPrompt = gtk_entry_new();
			gtk_entry_set_visibility(GTK_ENTRY(passPrompt), FALSE);
			gtk_entry_set_buffer(GTK_ENTRY(passPrompt), passBuff);
			gtk_entry_set_input_purpose(GTK_ENTRY(passPrompt), GTK_INPUT_PURPOSE_PASSWORD);
			gtk_entry_set_placeholder_text(GTK_ENTRY(passPrompt), "Enter your password here");

			g_signal_connect_swapped(btnConnect, "clicked", G_CALLBACK(pasconnect_cb), passBuff);

			GtkWidget *passBoxV;
			passBoxV = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
			gtk_box_prepend(GTK_BOX(passBoxV), passPrompt);
			gtk_box_set_homogeneous(GTK_BOX(passBoxV), TRUE);

			GtkWidget *passBoxH;
			passBoxH = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
			gtk_box_append(GTK_BOX(passBoxH), btnCancel);
			gtk_box_append(GTK_BOX(passBoxH), btnConnect);
			gtk_box_set_homogeneous(GTK_BOX(passBoxH), TRUE);

			GtkWidget *passBoxAll;
			passBoxAll = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
			gtk_box_prepend(GTK_BOX(passBoxAll), passBoxV);
			gtk_box_append(GTK_BOX(passBoxAll), passBoxH);
			gtk_box_set_homogeneous(GTK_BOX(passBoxAll), TRUE);
			gtk_widget_set_margin_start(passBoxAll, 3);
			gtk_widget_set_margin_end(passBoxAll, 3);

			gtk_window_set_child(GTK_WINDOW(passWindow), passBoxAll);
			gtk_window_present(GTK_WINDOW(passWindow));
			
		}
	}
	chkIfSaved = NULL;
	free(chkIfSaved);
	chkIfFree = NULL;
	free(chkIfFree);
}

/// creating hotspot
static void hotspot_cb() {
	g_print("hotspot\n");
	//*******[ NOTE the quick hotspot is not password protected!!! ] *******//
	// check if hotspot is already saved so you don't create it but just connect
	gboolean isActive = FALSE;

	gchar *chkIfActive;
	chkIfActive = output_to_char("nmcli connection show | grep -w -o quick-hotspot");
	if (chkIfActive == NULL) { chkIfActive = "<::Not_Saved_Network::>"; };

	// isActive assigning bool variables according to output
	if (strcmp(chkIfActive, "quick-hotspot") == 0) {
		isActive = TRUE;
	}
	else {
		isActive = FALSE;
	}

	if (isActive) {
		g_print("hotspot connected\n");
		run_cmd("nmcli con up quick-hotspot");
		g_signal_emit_by_name(refreshButton, "clicked");
	}
	else {
		g_print("creating hotspot\n");
		run_cmd("nmcli \
				connection add \
				type wifi con-name quick-hotspot \
				autoconnect no \
				wifi.mode ap \
				wifi.ssid quick-hotspot \
				ipv4.method shared \
				ipv6.method shared");
		g_print("connect to the newly created hotspot\n");
		run_cmd("nmcli con up quick-hotspot");
		g_signal_emit_by_name(refreshButton, "clicked");
	}

	chkIfActive = NULL;
	free(chkIfActive);
}

/// switching wifi on/off
static void notifySwitch(gpointer data) {
	(void)data;

	// wifi switch state
  	GValue valueSwitch = G_VALUE_INIT;
	gtk_expression_watch_evaluate(watchSwitch, &valueSwitch);

	if (G_VALUE_HOLDS_BOOLEAN(&valueSwitch)) {
		gboolean vSwitch = g_value_get_boolean(&valueSwitch);
		if (vSwitch == TRUE) {
			// if switching on
			run_cmd("nmcli radio wifi on; sleep 7");
			//g_timeout_add_seconds(1, waiting_ssids, networkList);
			g_signal_emit_by_name(refreshButton, "clicked");
			gtk_widget_set_visible(noWiFi, FALSE);
			gtk_widget_set_visible(netScrolled, TRUE);
			//gtk_widget_set_sensitive(hotSpotButton, TRUE);
			gtk_widget_set_sensitive(refreshButton, TRUE);
			gtk_label_set_markup(GTK_LABEL(labelSSIDS),
				"<span size=\"12pt\" weight=\"bold\">Scanning please wait...</span>");
			g_print("wifi enabled\n");
		}
		else {
			// if switching off
			run_cmd("nmcli radio wifi off");
			gtk_widget_set_visible(netScrolled, FALSE);
			gtk_widget_set_visible(noWiFi, TRUE);
			gtk_widget_set_sensitive(hotSpotButton, FALSE);
			gtk_widget_set_sensitive(refreshButton, FALSE);
			gtk_label_set_markup(GTK_LABEL(labelSSIDS),
				"<span size=\"12pt\" weight=\"bold\">Wifi disabled</span>");
			g_print("wifi disabled\n");
		}
	}
	g_value_unset(&valueSwitch);

	// getting wifi state
	wifi_state = output_to_char("nmcli radio  wifi");
	wifi_connection_check = output_to_char("nmcli networking connectivity check");

	if (strcmp(wifi_state, "enabled") == 0 && strcmp(wifi_connection_check, "full") == 0) {
		gtk_image_set_from_icon_name(GTK_IMAGE(networkIcon), "network-wireless");
	}
	else {
		gtk_image_set_from_icon_name(GTK_IMAGE(networkIcon), "network-wired-disconnected-symbolic");
	}
}

/// open network menu applet
static void on_network_l_click() {
	GtkWidget *window;
	window = gtk_window_new();

	GdkRectangle rect;
	rect.x = get_int_value_from_conf(pathToConfig, "posx") - 100;
	rect.y = get_int_value_from_conf(pathToConfig, "posy") - 3;
	rect.width = 0;
	rect.height = 0;

	wifi_connection_check = output_to_char("nmcli networking connectivity check");
	wifi_connection_name = output_to_char("nmcli \
										  --field=NAME \
										  connection \
										  show --active | \
										  awk \"NR==2\" | \
										  awk '{$1=$1;print}'");

	networkList = gtk_string_list_new((const gchar * const *)ssids);

	p_dataOptions->stringsNames = networkList;

	GListModel *myGlistModel;
	myGlistModel = G_LIST_MODEL(networkList);

	GtkListItemFactory *myGtkListItemFactory;
	myGtkListItemFactory = gtk_signal_list_item_factory_new();

	g_signal_connect(myGtkListItemFactory, "setup", G_CALLBACK(setup_listitem_net), NULL);
	g_signal_connect(myGtkListItemFactory, "bind", G_CALLBACK(bind_listitem_net), NULL);

	mySingleSelection = gtk_single_selection_new(G_LIST_MODEL(myGlistModel));
	
	GtkSelectionModel *mySelectionModel;
	mySelectionModel = GTK_SELECTION_MODEL(mySingleSelection);

	GtkWidget *myGtkListView;
	myGtkListView = gtk_list_view_new(mySelectionModel, myGtkListItemFactory);
	gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(myGtkListView), TRUE);
	gtk_widget_set_margin_start(myGtkListView, 30);
	gtk_widget_set_margin_end(myGtkListView, 30);

	netScrolled = gtk_scrolled_window_new();
	gtk_widget_set_visible(netScrolled, FALSE);
	gtk_widget_set_size_request(netScrolled, 500, 500);
	gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(netScrolled), TRUE);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(netScrolled), myGtkListView);

	//******* switching wifi on/off
	GtkWidget *wifiState;
	wifiState = gtk_switch_new();
	gtk_widget_set_halign(wifiState, GTK_ALIGN_START);

	// getting wifi state
	wifi_state = output_to_char("nmcli radio  wifi");
	if (strcmp(wifi_state, "enabled") == 0) {
		gtk_switch_set_state(GTK_SWITCH(wifiState), TRUE);
	}
	else {
		gtk_switch_set_state(GTK_SWITCH(wifiState), FALSE);
	}

	expressionSwitch = gtk_property_expression_new(GTK_TYPE_SWITCH, NULL, "state");
	watchSwitch = gtk_expression_watch(expressionSwitch, wifiState, notifySwitch, NULL, NULL);

	GtkWidget *labelState;
	labelState = gtk_label_new("enable WiFi");
	gtk_widget_set_halign(labelState, GTK_ALIGN_START);

	refreshButton = gtk_button_new_with_label("Refresh Network List");
	gtk_button_set_has_frame(GTK_BUTTON(refreshButton), TRUE);
	gtk_widget_set_sensitive(refreshButton, FALSE);

	hotSpotButton = gtk_button_new_with_label(" \tQuick Hotspot\t ");
	gtk_button_set_has_frame(GTK_BUTTON(hotSpotButton), TRUE);

	if (strcmp(wifi_connection_check, "full") == 0) {
		gtk_widget_set_sensitive(hotSpotButton, FALSE);
	}
	else {
		gtk_widget_set_sensitive(hotSpotButton, TRUE);
	}

	g_signal_connect_swapped(hotSpotButton, "clicked", G_CALLBACK(hotspot_cb), NULL);

	GtkWidget *boxState;
	boxState = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
	gtk_box_prepend(GTK_BOX(boxState), wifiState);
	gtk_box_append(GTK_BOX(boxState), labelState);
	gtk_box_append(GTK_BOX(boxState), refreshButton);
	gtk_box_append(GTK_BOX(boxState), hotSpotButton);
	gtk_widget_set_margin_start(boxState, 10);
	gtk_widget_set_margin_end(boxState, 10);
	gtk_widget_set_margin_top(boxState, 10);
	gtk_widget_set_margin_bottom(boxState, 10);

	labelSSIDS = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(labelSSIDS),
		"<span size=\"12pt\" weight=\"bold\">Scanning please wait...</span>");

	noWiFi = gtk_label_new("WiFi is disabled! Please enable WiFi");
	gtk_widget_set_size_request(noWiFi, 500, 500);
	gtk_widget_set_visible(noWiFi, FALSE);
	
	boxNet = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_prepend(GTK_BOX(boxNet), boxState);
	gtk_box_append(GTK_BOX(boxNet), labelSSIDS);
	gtk_box_append(GTK_BOX(boxNet), netScrolled);
	gtk_box_append(GTK_BOX(boxNet), noWiFi);

	// if wifi is disabled display the message
	if (strcmp(wifi_state, "enabled") == 0) {
		gtk_label_set_markup(GTK_LABEL(labelSSIDS),
			"<span size=\"12pt\" weight=\"bold\">Scanning please wait...</span>");
		gtk_widget_set_visible(netScrolled, TRUE);
		gtk_widget_set_sensitive(refreshButton, TRUE);
		//gtk_widget_set_sensitive(hotSpotButton, TRUE);
	}
	else {
		gtk_label_set_markup(GTK_LABEL(labelSSIDS),
			"<span size=\"12pt\" weight=\"bold\">Wifi disabled</span>");
		gtk_widget_set_visible(noWiFi, TRUE);
	}

	popNetwork = gtk_popover_new();
	gtk_popover_set_has_arrow(GTK_POPOVER(popNetwork), FALSE);
	gtk_popover_set_pointing_to(GTK_POPOVER(popNetwork), &rect);
	gtk_popover_set_child(GTK_POPOVER(popNetwork), boxNet);

	g_signal_connect_swapped(popNetwork, "closed", G_CALLBACK(on_network_close), window);
	g_signal_connect_swapped(refreshButton, "clicked", G_CALLBACK(refresh_cb), myGlistModel);

	GtkWidget *box;
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_append(GTK_BOX(box), popNetwork);
	gtk_box_set_homogeneous(GTK_BOX(box), TRUE);

	gtk_window_set_child(GTK_WINDOW(window), box);
	gtk_popover_popup(GTK_POPOVER(popNetwork));

	g_timeout_add_seconds(1, waiting_ssids, networkList);
}
