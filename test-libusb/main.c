#include <stdio.h>
#include <dlfcn.h>
#include <gtk/gtk.h>
#include <sys/types.h>


int
main(int c, char *a[])
{
	GtkWidget *w = gtk_window_new(GTK_WINDOW_POPUP);
	printf("Hello world, from a sandbox.\n");
	printf("modified.\n");

	return 0;
}//end main
