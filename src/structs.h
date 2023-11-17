#include <gtk/gtk.h>

#ifndef STRUCTS_H_
#define STRUCTS_H_

struct dataOptions {
	GtkStringList *stringsNames;
};

struct dataOptions mydataOptions;
struct dataOptions *p_dataOptions = &mydataOptions;

#endif
