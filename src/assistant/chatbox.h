#ifndef CHATBOX_H
#define CHATBOX_H

#include <gtk/gtk.h>

// Callback function type for send button
typedef void (*SendMessageCallback)(const char* message);

GtkWidget* chatbox_window();
GtkWidget* chatbox_user_input(SendMessageCallback callback, GtkWidget* message_list);
GtkWidget* chatbox_send_button(GtkWidget* input_entry, SendMessageCallback callback);
GtkWidget* chatbox_message_ai(const char* message_text);
GtkWidget* chatbox_message_user(const char* message_text);
GtkWidget* chatbox_view();

#endif // CHATBOX_H 