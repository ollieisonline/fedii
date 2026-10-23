#include "mastodon.h"
#include "network.h"
#include <jansson.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Instance configuration
static char *instance_url = NULL;
static char *access_token = NULL;

int mastodon_init(const char *url, const char *token) {
    // Free existing config
    mastodon_exit();
    
    if (!url) return -1;
    
    instance_url = strdup(url);
    
    // Ensure URL ends with /
    int len = strlen(instance_url);
    if (len > 0 && instance_url[len - 1] != '/') {
        instance_url = realloc(instance_url, len + 2);
        instance_url[len] = '/';
        instance_url[len + 1] = '\0';
    }
    
    if (token) {
        access_token = strdup(token);
    }
    
    return 0;
}

void mastodon_exit() {
    free(instance_url);
    free(access_token);
    instance_url = NULL;
    access_token = NULL;
}

// Helper to extract string from JSON or return default
static const char* json_get_string(json_t *obj, const char *key, const char *def) {
    json_t *val = json_object_get(obj, key);
    if (val && json_is_string(val)) {
        return json_string_value(val);
    }
    return def;
}

mastodon_status_t* mastodon_fetch_timeline(int *count) {
    char url[1024];
    char *response = NULL;
    json_t *root = NULL;
    json_t *statuses = NULL;
    mastodon_status_t *result = NULL;
    
    if (!instance_url) return NULL;
    
    // Build URL
    snprintf(url, sizeof(url), "%sapi/v1/timelines/home", instance_url);
    
    // Fetch from API
    printf("Fetching timeline from: %s\n", url);
    if (network_get(url, access_token, &response) != 0) {
        printf("Network error fetching timeline\n");
        return NULL;
    }
    
    // Parse JSON
    root = json_loads(response, 0, NULL);
    free(response);
    
    if (!root) {
        printf("JSON parse error\n");
        return NULL;
    }
    
    statuses = json_object_get(root, "statuses");
    if (!statuses || !json_is_array(statuses)) {
        printf("No statuses in response\n");
        json_decref(root);
        return NULL;
    }
    
    // Count and allocate
    *count = json_array_size(statuses);
    if (*count == 0) {
        json_decref(root);
        return NULL;
    }
    
    result = calloc(*count, sizeof(mastodon_status_t));
    if (!result) {
        json_decref(root);
        return NULL;
    }
    
    // Parse each status
    for (int i = 0; i < *count; i++) {
        json_t *status = json_array_get(statuses, i);
        if (!status) continue;
        
        result[i].id = strdup(json_get_string(status, "id", ""));
        
        // Get content - handle HTML
        const char *content = json_get_string(status, "content", "");
        // Simple HTML tag removal
        char *content_clean = malloc(strlen(content) + 1);
        int j = 0;
        int in_tag = 0;
        for (int k = 0; content[k]; k++) {
            if (content[k] == '<') in_tag = 1;
            if (!in_tag) content_clean[j++] = content[k];
            if (content[k] == '>') in_tag = 0;
        }
        content_clean[j] = '\0';
        result[i].content = content_clean;
        
        // Get account info
        json_t *account = json_object_get(status, "account");
        if (account) {
            result[i].author_username = strdup(json_get_string(account, "username", ""));
            result[i].author_display_name = strdup(json_get_string(account, "display_name", ""));
        } else {
            result[i].author_username = strdup("");
            result[i].author_display_name = strdup("");
        }
        
        result[i].created_at = strdup(json_get_string(status, "created_at", ""));
    }
    
    json_decref(root);
    return result;
}

int mastodon_post_status(const char *text) {
    char url[1024];
    char *response = NULL;
    json_t *root = NULL;
    int result = -1;
    
    if (!instance_url || !text) return -1;
    
    // Build URL
    snprintf(url, sizeof(url), "%sapi/v1/statuses", instance_url);
    
    // Build JSON payload
    json_t *payload = json_object();
    json_object_set_new(payload, "status", json_string(text));
    
    char *payload_str = json_dumps(payload, 0);
    json_decref(payload);
    
    // POST to API
    printf("Posting status to: %s\n", url);
    if (network_post(url, access_token, payload_str, &response) == 0) {
        root = json_loads(response, 0, NULL);
        if (root) {
            result = 0;
            json_decref(root);
        }
    }
    
    free(payload_str);
    free(response);
    return result;
}

mastodon_account_t* mastodon_get_account() {
    char url[1024];
    char *response = NULL;
    json_t *root = NULL;
    mastodon_account_t *account = NULL;
    
    if (!instance_url || !access_token) return NULL;
    
    // Build URL
    snprintf(url, sizeof(url), "%sapi/v1/accounts/verify_credentials", instance_url);
    
    // Fetch from API
    if (network_get(url, access_token, &response) != 0) {
        return NULL;
    }
    
    // Parse JSON
    root = json_loads(response, 0, NULL);
    free(response);
    
    if (!root) {
        return NULL;
    }
    
    account = calloc(1, sizeof(mastodon_account_t));
    account->id = strdup(json_get_string(root, "id", ""));
    account->username = strdup(json_get_string(root, "username", ""));
    account->display_name = strdup(json_get_string(root, "display_name", ""));
    account->avatar_url = strdup(json_get_string(root, "avatar", ""));
    
    json_decref(root);
    return account;
}

void mastodon_free_statuses(mastodon_status_t *statuses, int count) {
    if (!statuses) return;
    
    for (int i = 0; i < count; i++) {
        free(statuses[i].id);
        free(statuses[i].content);
        free(statuses[i].author_username);
        free(statuses[i].author_display_name);
        free(statuses[i].created_at);
    }
    free(statuses);
}

void mastodon_free_account(mastodon_account_t *account) {
    if (!account) return;
    free(account->id);
    free(account->username);
    free(account->display_name);
    free(account->avatar_url);
    free(account);
}
