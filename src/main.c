#include <gtk/gtk.h>
#include <fontconfig/fontconfig.h>
#include "assistant/chatbox.h"
#include "assistant/assistant.h"

GtkWidget* create_main_window() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Metrical for Econometrics");
    gtk_window_set_default_size(GTK_WINDOW(window), 960, 540);
    gtk_window_fullscreen(GTK_WINDOW(window));
    
    // Load CSS from external file for main window
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "src/styles/main.css", NULL);
    
    GtkStyleContext *context = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider(context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Create a horizontal paned widget for left/right split
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(window), paned);

    // Use chatbox_window for the left (assistant UI)
    GtkWidget *left = chatbox_window();
    GtkWidget *right = gtk_label_new("Right Side");

    gtk_paned_pack1(GTK_PANED(paned), left, TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(paned), right, TRUE, FALSE);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    return window;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    // Load custom font from fonts directory
    FcConfigAppFontAddFile(NULL, (const FcChar8 *)"fonts/Switzer-Variable.ttf");
    GtkWidget *window = create_main_window();
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}