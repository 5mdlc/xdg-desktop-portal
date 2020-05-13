#include <dlfcn.h>
#include <gtk/gtk.h>

# define RTLD_DEFAULT	((void *) 0)



GtkWidget *window;
void print_msg(GtkWidget *widget, gpointer _window)
{
  const gchar *title = "Native";
  GtkWindow* parent_window = GTK_WINDOW(window);
  GtkWidget *dialog;

  const gchar *accept_label = "pepe";

  void  * (*sGtkFileChooserNativeNewPtr)(const gchar*, GtkWindow*, GtkFileChooserAction, const gchar*, const gchar*);
  printf("Native.\n");

  sGtkFileChooserNativeNewPtr = (void* (*)(const gchar*, GtkWindow*, GtkFileChooserAction, const gchar*,
                 const gchar*))dlsym(RTLD_DEFAULT, "gtk_file_chooser_native_new");
  fprintf(stderr, "sGtkFileChooserNativeNewPtr := 0x%x\n", sGtkFileChooserNativeNewPtr );
  if (sGtkFileChooserNativeNewPtr != NULL) {
    dialog = (*sGtkFileChooserNativeNewPtr)(title, parent_window, GTK_FILE_CHOOSER_ACTION_OPEN, accept_label, NULL);

    fprintf(stderr, "dialog := 0x%x\n", dialog );

    if(gtk_native_dialog_run (GTK_NATIVE_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT){
      char *filename;

      GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
      filename = gtk_file_chooser_get_filename (chooser);

      fprintf(stderr, "open_file (%s);\n",filename);
      fprintf(stderr, "g_free (%s);\n",filename);
    }

    //gtk_widget_destroy (dialog);
    //gtk_object_unref (dialog);
  }

  printf("Hello world, from a sandbox.\n\n");
}

void print_msg2(GtkWidget *widget, gpointer _window)
{
  const gchar *title = "Dialog";
  GtkWindow* parent_window = GTK_WINDOW(window);
  GtkWidget *dialog;

  printf("Dialog.\n");

  if (1){
    dialog = gtk_file_chooser_dialog_new (title, parent_window, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

    fprintf(stderr, "dialog := 0x%x\n", dialog );

    if(gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT){
      char *filename;
      fprintf(stderr, "GTK_RESPONSE_ACCEPT\n");

      GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
      filename = gtk_file_chooser_get_filename (chooser);

      fprintf(stderr, "open_file (%s);\n",filename);
      fprintf(stderr, "g_free (%s);\n",filename);
    }

    gtk_widget_destroy (dialog);
    //gtk_object_unref (dialog);
  }

  printf("Hello world, from a sandbox.\n\n");
}

int
main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *grid;
  GtkWidget *button;

  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "testgtk");
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

  grid = gtk_grid_new ();

  gtk_container_add (GTK_CONTAINER (window), grid);

  button = gtk_button_new_with_mnemonic("_Native");
  g_signal_connect (button, "clicked", G_CALLBACK (print_msg), NULL);

  gtk_grid_attach (GTK_GRID (grid), button, 0, 0, 1, 1);

  button = gtk_button_new_with_mnemonic("_Dialog");
  g_signal_connect (button, "clicked", G_CALLBACK (print_msg2), NULL);

  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);

  button = gtk_button_new_with_mnemonic ("_Quit");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);

  gtk_grid_attach (GTK_GRID (grid), button, 0, 1, 2, 1);

  gtk_widget_show_all (window);

  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL); 

  gtk_main();

  return 0;
}

