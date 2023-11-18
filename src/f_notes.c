//*** notes ***//
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "externvars.h"

gchar *contents;
gsize len;
GError *err = NULL;
char fileNotesBuff[700];
char notesCssBuff[700];
gint toggle_notes = 0;
GtkTextIter start;
GtkTextIter end;
GtkWidget *popNotes = NULL;
GtkWidget *notesContext = NULL;

/// when notes autohides close the window to prevent memory leak
static void on_notes_close(GtkTextBuffer *textBuff) {
	gtk_text_buffer_get_iter_at_offset(textBuff, &start, 0);
	gtk_text_buffer_get_iter_at_offset(textBuff, &end, -1);

	gchar *text = gtk_text_buffer_get_text(textBuff, &start, &end, TRUE);

	FILE *fNotes = fopen(fileNotesBuff, "w+");
		fprintf(fNotes, "%s", text);
	fclose(fNotes);

	contents = NULL;
	g_free(contents);
}

static void on_notescontext_l_click(GtkWindow *window) {
	gtk_widget_set_visible(GTK_WIDGET(window), TRUE);
	gtk_window_present(GTK_WINDOW(window));
}

static void on_notes_l_click() {
	// create config file
	const char *fileNotes = "/.config/diopanel/diopanelapplets/notes";
	const char *notesCss = "/.config/diopanel/diopanelapplets/notes.css";

	snprintf(fileNotesBuff, sizeof(fileNotesBuff), "%s%s", HOME, fileNotes);
	snprintf(notesCssBuff, sizeof(notesCssBuff), "%s%s", HOME, notesCss);

	FILE *fNotes = fopen(fileNotesBuff, "r");
	FILE *fNotesCss = fopen(notesCssBuff, "r");
	
	if (fNotes == NULL && fNotesCss == NULL) {
		g_print("created config files\n");

		FILE *fNotes = fopen(fileNotesBuff, "w+");
		FILE *fNotesCss = fopen(notesCssBuff, "w+");

		fprintf(fNotes, "%s\n", "Notes:");
		fprintf(fNotesCss, "%s\n", "textview.view { font-size: 20px; }");

		fclose(fNotes);
		fclose(fNotesCss);
	}
	else {
		fclose(fNotes);
		fclose(fNotesCss);
	}

	// don't create a new window each time
	if (windowNotes == NULL) {
		windowNotes = gtk_window_new();
		gtk_window_set_title(GTK_WINDOW(windowNotes), "Notes");
		gtk_window_set_decorated(GTK_WINDOW(windowNotes), FALSE);
		gtk_window_set_default_size(GTK_WINDOW(windowNotes), 1, 1);
		gtk_window_set_icon_name(GTK_WINDOW(windowNotes), "text-editor");

		GdkRectangle rect;
		rect.x = 0;
		rect.y = -103;
		rect.width = 1300;
		rect.height = 0;

		GdkDisplay *display;
		display = gdk_display_get_default();

		GtkCssProvider *cssprovider;
		cssprovider = gtk_css_provider_new();
		gtk_css_provider_load_from_path(cssprovider, notesCssBuff);

		gtk_style_context_add_provider_for_display(display,
												   GTK_STYLE_PROVIDER(cssprovider),
												   GTK_STYLE_PROVIDER_PRIORITY_USER);

		notesContext = gtk_text_view_new();
		gtk_text_view_set_editable(GTK_TEXT_VIEW(notesContext), TRUE);
		gtk_text_view_set_overwrite(GTK_TEXT_VIEW(notesContext), TRUE);
		gtk_text_view_set_monospace(GTK_TEXT_VIEW(notesContext), TRUE);
		gtk_text_view_set_accepts_tab(GTK_TEXT_VIEW(notesContext), TRUE);
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(notesContext), GTK_WRAP_WORD);
		gtk_text_view_set_justification(GTK_TEXT_VIEW(notesContext), GTK_JUSTIFY_LEFT);

		// get content of the notes file and assign it to the buffer
		if (!g_file_get_contents(fileNotesBuff, &contents, &len, &err)) {
			g_warning("Error loading file: %s\n", err->message);
			g_clear_error(&err);
			return;
		}

		GtkTextBuffer *textBuff;
		textBuff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(notesContext));
		gtk_text_buffer_set_text(textBuff, contents, -1);

		// adds left click action for notes context
		GtkGesture *notesApp;
		notesApp = gtk_gesture_click_new();
		GtkGestureSingle *notesAppClick;
		notesAppClick = GTK_GESTURE_SINGLE(notesApp);
		gtk_gesture_single_set_button(notesAppClick, 1);
		GtkEventController *notesAppRClick;
		notesAppRClick = GTK_EVENT_CONTROLLER(notesApp);
		gtk_event_controller_set_propagation_phase(notesAppRClick, GTK_PHASE_CAPTURE);
		gtk_widget_add_controller(notesContext, notesAppRClick);

		g_signal_connect_swapped(notesAppClick, "pressed", G_CALLBACK(on_notescontext_l_click),
																					windowNotes);

		GtkWidget *textScrolled;
		textScrolled = gtk_scrolled_window_new();
		gtk_widget_set_size_request(textScrolled, 570, 570);
		gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(textScrolled), TRUE);
		gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(textScrolled), notesContext);

		popNotes = gtk_popover_new();
		gtk_popover_set_autohide(GTK_POPOVER(popNotes), FALSE);
		gtk_popover_set_has_arrow(GTK_POPOVER(popNotes), FALSE);
		gtk_popover_set_pointing_to(GTK_POPOVER(popNotes), &rect);
		gtk_popover_set_child(GTK_POPOVER(popNotes), textScrolled);

		g_signal_connect_swapped(popNotes, "closed", G_CALLBACK(on_notes_close), textBuff);

		GtkWidget *box;
		box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
		gtk_box_prepend(GTK_BOX(box), popNotes);
		gtk_box_set_homogeneous(GTK_BOX(box), TRUE);

		GtkWidget *frame;
		frame = gtk_frame_new(NULL);
		gtk_frame_set_child(GTK_FRAME(frame), box);

		gtk_window_set_child(GTK_WINDOW(windowNotes), frame);
		gtk_window_present(GTK_WINDOW(windowNotes));
		gtk_widget_grab_focus(notesContext);
	}

	if (toggle_notes == 0) {
		toggle_notes = 1;
		gtk_widget_set_visible(windowNotes, TRUE);
		gtk_popover_popup(GTK_POPOVER(popNotes));
		gtk_widget_grab_focus(notesContext);
	}
	else {
		toggle_notes = 0;
		gtk_widget_set_visible(windowNotes, FALSE);
		gtk_popover_popdown(GTK_POPOVER(popNotes));
	}
}
