#include "chatbox.h"
#include "assistant.h"
#include <librsvg/rsvg.h>



// New function to handle the AI response
static void handle_llm_response(const char* ai_response, GtkWidget* message_list) {
    if (ai_response) {
        // Print the same response to the console as shown in the message box
        g_print("AI response: %s\n", ai_response);
        // Create AI message widget
        GtkWidget *ai_message = chatbox_message_ai(ai_response);
        // Add AI message to the list
        gtk_box_pack_start(GTK_BOX(message_list), ai_message, FALSE, FALSE, 0);
        // Show the AI message
        gtk_widget_show_all(ai_message);
        // Scroll to the bottom again to show the AI response
        GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(
            gtk_widget_get_parent(gtk_widget_get_parent(message_list))));
        if (adj) {
            gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));
        }
    }
}

// Callback function for send button click
static void on_send_button_clicked(GtkWidget *button, gpointer user_data) {
    GtkWidget *input_entry = (GtkWidget*)user_data;
    const char *text = gtk_entry_get_text(GTK_ENTRY(input_entry));
    
    if (text && strlen(text) > 0) {
        // Get the callback function from button data
        SendMessageCallback callback = g_object_get_data(G_OBJECT(button), "send_callback");
        if (callback) {
            callback(text);
        }
        
        // Get the message list from button data
        GtkWidget *message_list = g_object_get_data(G_OBJECT(button), "message_list");
        if (message_list) {
            // Create user message widget
            GtkWidget *user_message = chatbox_message_user(text);
            
            // Add message to the list
            gtk_box_pack_start(GTK_BOX(message_list), user_message, FALSE, FALSE, 0);
            
            // Show the new message
            gtk_widget_show_all(user_message);
            
            // Scroll to the bottom to show the new message
            GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(
                gtk_widget_get_parent(gtk_widget_get_parent(message_list))));
            if (adj) {
                gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));
            }
            
            // Get AI response and handle it
            char* ai_response = assistant_send_message(text);
            handle_llm_response(ai_response, message_list);
            assistant_free_response(ai_response);
        }
        
        // Clear the input field
        gtk_entry_set_text(GTK_ENTRY(input_entry), "");
    }
}

// Callback function for Enter key press
static gboolean on_entry_activate(GtkWidget *entry, gpointer user_data) {
    GtkWidget *button = (GtkWidget*)user_data;
    on_send_button_clicked(button, entry);
    return TRUE;
}

GtkWidget* chatbox_send_button(GtkWidget* input_entry, SendMessageCallback callback) {
    // Create send button
    GtkWidget *send_button = gtk_button_new();
    gtk_widget_set_can_focus(send_button, FALSE);
    
    // Add CSS class to send button
    GtkStyleContext *button_context = gtk_widget_get_style_context(send_button);
    gtk_style_context_add_class(button_context, "send-btn");
    
    // Load SVG using librsvg with high resolution
    GtkWidget *send_arrow = NULL;
    GError *error = NULL;
    RsvgHandle *handle = rsvg_handle_new_from_file("assets/icons/send-arrow.svg", &error);
    
    if (handle != NULL) {
        // Set the size for high resolution rendering (2x for retina displays)
        rsvg_handle_set_dpi(handle, 144.0); // Higher DPI for better quality
        
        GdkPixbuf *pixbuf = rsvg_handle_get_pixbuf_and_error(handle, &error);
        if (pixbuf != NULL) {
            // Scale to 48x48 first for high resolution, then scale down to 24x24
            GdkPixbuf *high_res = gdk_pixbuf_scale_simple(pixbuf, 48, 48, GDK_INTERP_HYPER);
            if (high_res) {
                GdkPixbuf *scaled = gdk_pixbuf_scale_simple(high_res, 24, 24, GDK_INTERP_HYPER);
                if (scaled) {
                    send_arrow = gtk_image_new_from_pixbuf(scaled);
                    g_object_unref(scaled);
                } else {
                    send_arrow = gtk_image_new_from_pixbuf(high_res);
                }
                g_object_unref(high_res);
            } else {
                send_arrow = gtk_image_new_from_pixbuf(pixbuf);
            }
            g_object_unref(pixbuf);
        } else {
            send_arrow = gtk_label_new("→");
            g_print("Warning: Could not create pixbuf from SVG: %s\n", error ? error->message : "Unknown error");
            if (error) g_error_free(error);
        }
        g_object_unref(handle);
    } else {
        send_arrow = gtk_label_new("→");
        g_print("Warning: Could not load SVG file: %s\n", error ? error->message : "Unknown error");
        if (error) g_error_free(error);
    }
    
    // Add send arrow to button
    gtk_container_add(GTK_CONTAINER(send_button), send_arrow);
    
    // Store the callback function in the button's data
    g_object_set_data(G_OBJECT(send_button), "send_callback", callback);
    
    // Connect the button click signal
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_button_clicked), input_entry);
    
    // Connect the entry activate signal (Enter key)
    g_signal_connect(input_entry, "activate", G_CALLBACK(on_entry_activate), send_button);
    
    // Load CSS from external file
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "src/styles/chatbox.css", NULL);
    
    // Apply CSS to button
    gtk_style_context_add_provider(button_context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    return send_button;
}

GtkWidget* chatbox_user_input(SendMessageCallback callback, GtkWidget* message_list) {
    // Create input container (horizontal box)
    GtkWidget *input_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_start(input_container, 12);
    gtk_widget_set_margin_end(input_container, 12);
    gtk_widget_set_margin_bottom(input_container, 12);
    
    // Add CSS class to input container
    GtkStyleContext *container_context = gtk_widget_get_style_context(input_container);
    gtk_style_context_add_class(container_context, "input-container");
    
    // Create input box
    GtkWidget *input_box = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(input_box), "Type your message here...");
    
    // Add CSS class to input box
    GtkStyleContext *input_context = gtk_widget_get_style_context(input_box);
    gtk_style_context_add_class(input_context, "chat-input");
    
    // Create send button using the new function
    GtkWidget *send_button = chatbox_send_button(input_box, callback);
    
    // Store the message list in the send button's data
    g_object_set_data(G_OBJECT(send_button), "message_list", message_list);
    
    // Pack input box and send button into container
    gtk_box_pack_start(GTK_BOX(input_container), input_box, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(input_container), send_button, FALSE, FALSE, 0);
    
    // Load CSS from external file
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "src/styles/chatbox.css", NULL);
    
    // Apply CSS to container and input
    gtk_style_context_add_provider(container_context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(input_context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    return input_container;
}

GtkWidget* chatbox_message_ai(const char* message_text) {
    // Create message container
    GtkWidget *message_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_start(message_container, 12);
    gtk_widget_set_margin_end(message_container, 12);
    gtk_widget_set_margin_top(message_container, 8);
    gtk_widget_set_margin_bottom(message_container, 8);
    
    // Add CSS class to message container
    GtkStyleContext *container_context = gtk_widget_get_style_context(message_container);
    gtk_style_context_add_class(container_context, "message-container");
    
    // Create message box (left-aligned for AI)
    GtkWidget *message_box = gtk_label_new(message_text);
    gtk_widget_set_halign(message_box, GTK_ALIGN_START);
    gtk_widget_set_valign(message_box, GTK_ALIGN_START);
    gtk_widget_set_margin_end(message_box, 60); // Leave space for user messages
    // Enable line wrapping
    gtk_label_set_line_wrap(GTK_LABEL(message_box), TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL(message_box), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_max_width_chars(GTK_LABEL(message_box), 60);
    // Add CSS class to message box
    GtkStyleContext *message_context = gtk_widget_get_style_context(message_box);
    gtk_style_context_add_class(message_context, "message-ai");
    
    // Pack message box into container
    gtk_box_pack_start(GTK_BOX(message_container), message_box, FALSE, FALSE, 0);
    
    // Load CSS from external file
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "src/styles/chatbox.css", NULL);
    
    // Apply CSS to container and message
    gtk_style_context_add_provider(container_context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(message_context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    return message_container;
}

GtkWidget* chatbox_message_user(const char* message_text) {
    // Create message container
    GtkWidget *message_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_start(message_container, 12);
    gtk_widget_set_margin_end(message_container, 12);
    gtk_widget_set_margin_top(message_container, 8);
    gtk_widget_set_margin_bottom(message_container, 8);
    
    // Add CSS class to message container
    GtkStyleContext *container_context = gtk_widget_get_style_context(message_container);
    gtk_style_context_add_class(container_context, "message-container");
    
    // Create message box (right-aligned for user)
    GtkWidget *message_box = gtk_label_new(message_text);
    gtk_widget_set_halign(message_box, GTK_ALIGN_END);
    gtk_widget_set_valign(message_box, GTK_ALIGN_START);
    gtk_widget_set_margin_start(message_box, 60); // Leave space for AI messages
    
    // Add CSS class to message box
    GtkStyleContext *message_context = gtk_widget_get_style_context(message_box);
    gtk_style_context_add_class(message_context, "message-user");
    
    // Pack message box into container
    gtk_box_pack_end(GTK_BOX(message_container), message_box, FALSE, FALSE, 0);
    
    // Load CSS from external file
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "src/styles/chatbox.css", NULL);
    
    // Apply CSS to container and message
    gtk_style_context_add_provider(container_context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider(message_context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    return message_container;
}

GtkWidget* chatbox_view() {
    // Create scrollable view for messages
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    
    // Create viewport for messages
    GtkWidget *viewport = gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);
    
    // Create message list container (vertical box)
    GtkWidget *message_list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(message_list, GTK_ALIGN_FILL);
    gtk_widget_set_valign(message_list, GTK_ALIGN_START);
    
    // Add CSS class to message list
    GtkStyleContext *list_context = gtk_widget_get_style_context(message_list);
    gtk_style_context_add_class(list_context, "message-list");
    
    // Add viewport to message list
    gtk_container_add(GTK_CONTAINER(viewport), message_list);
    
    // Store the message list in the scrolled window data for easy access
    g_object_set_data(G_OBJECT(scrolled_window), "message_list", message_list);
    
    // Load CSS from external file
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "src/styles/chatbox.css", NULL);
    
    // Apply CSS to message list
    gtk_style_context_add_provider(list_context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    return scrolled_window;
}

// Default callback function for send button
static void default_send_callback(const char* message) {
    g_print("Message sent: %s\n", message);
    // Call back is implemented above L130
}

GtkWidget* chatbox_window() {
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Create chat view for messages
    GtkWidget *chat_view = chatbox_view();
    
    // Get the message list from the chat view
    GtkWidget *message_list = g_object_get_data(G_OBJECT(chat_view), "message_list");
    
    // Create input using the new function with default callback and message list
    GtkWidget *input_container = chatbox_user_input(default_send_callback, message_list);
    
    // Pack widgets into the main box
    gtk_box_pack_start(GTK_BOX(main_box), chat_view, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(main_box), input_container, FALSE, FALSE, 0);
    
    return main_box;
} 