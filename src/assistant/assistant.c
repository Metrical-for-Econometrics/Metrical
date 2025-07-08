#include "assistant.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* assistant_send_message(const char* message) {
    // For now, just return an error message since AI is not set up
    // TODO: Implement actual API call to AI service
    
    char* response = malloc(100);
    if (response) {
        strcpy(response, "Error, AI not set up :(");
    }
    
    return response;
}

void assistant_free_response(char* response) {
    if (response) {
        free(response);
    }
} 