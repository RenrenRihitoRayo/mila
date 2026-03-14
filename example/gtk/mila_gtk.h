#include <gtk-3.0/gtk/gtk.h>
#include <string.h>

typedef struct {
    char* title;
    char* message;
} Args;

static void _activate(GtkApplication* app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *label;

  Args* data = (Args*)user_data;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW (window), data->title /*data->title*/);
  gtk_window_set_default_size(GTK_WINDOW (window), 200, 200);

  label = gtk_label_new(data->message);
  gtk_window_set_role(GTK_WINDOW(window), NULL); // Use gtk_container_add for older GTK versions
  gtk_container_add(GTK_CONTAINER(window), label);

  gtk_widget_show_all(window);
}

int make_win(char* title, char* message) {
  GtkApplication *app;
  int status;

  Args* args = (Args*)g_new0(Args, 1);
  args->title = strdup(title);
  args->message = strdup(message);

  app = gtk_application_new("org.gtk.example",G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(_activate), args);
  status = g_application_run(G_APPLICATION(app), 0, NULL);
  g_object_unref(app);

  return status;
}
