#include "assistant.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

// Helper struct for dynamic response
struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) return 0; // out of memory!
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

// Helper to extract text between <answer> and </answer>
static char* extract_answer(const char* text) {
    const char* start = strstr(text, "<answer>");
    if (!start) return NULL;
    start += strlen("<answer>");
    const char* end = strstr(start, "</answer>");
    if (!end) return NULL;
    size_t len = end - start;
    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;
    strncpy(result, start, len);
    result[len] = '\0';
    return result;
}

char* assistant_send_message(const char* message) {
    CURL *curl = curl_easy_init();
    if (!curl) return strdup("Error: Could not initialize curl");

    // OpenRouter API key and endpoint
    const char* api_key = getenv("OPENROUTER_API_KEY");
    const char* url = "https://openrouter.ai/api/v1/chat/completions";
    char post_data[2048];
    snprintf(post_data, sizeof(post_data),
        "{\"model\": \"tencent/hunyuan-a13b-instruct:free\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}]}" , message);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
    headers = curl_slist_append(headers, auth_header);

    struct MemoryStruct chunk = {malloc(1), 0};

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if(res != CURLE_OK) {
        free(chunk.memory);
        return strdup("Error: API call failed");
    }

    // Parse JSON to extract the actual response text
    cJSON *root = cJSON_Parse(chunk.memory);
    free(chunk.memory);
    if (!root) return strdup("Error: Invalid JSON response");
    cJSON *choices = cJSON_GetObjectItem(root, "choices");
    if (!choices || !cJSON_IsArray(choices) || cJSON_GetArraySize(choices) == 0) {
        cJSON_Delete(root);
        return strdup("Error: No choices in response");
    }
    cJSON *first_choice = cJSON_GetArrayItem(choices, 0);
    cJSON *message_obj = cJSON_GetObjectItem(first_choice, "message");
    if (!message_obj) {
        cJSON_Delete(root);
        return strdup("Error: No message in response");
    }
    cJSON *content = cJSON_GetObjectItem(message_obj, "content");
    if (!content || !cJSON_IsString(content)) {
        cJSON_Delete(root);
        return strdup("Error: No content in response");
    }
    char* answer = extract_answer(content->valuestring);
    cJSON_Delete(root);
    if (!answer) return strdup("Error: <answer> tag not found");
    return answer;
}

void assistant_free_response(char* response) {
    if (response) {
        free(response);
    }
} 