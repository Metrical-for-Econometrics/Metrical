#ifndef ASSISTANT_H
#define ASSISTANT_H

// Function to send a message to the AI and get a response
char* assistant_send_message(const char* message);

// Function to free the memory allocated for AI responses
void assistant_free_response(char* response);

#endif // ASSISTANT_H 