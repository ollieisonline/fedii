#include "mastodon.h"
#include "network.h"
#include <jansson.h>
#include <string.h>
#include <stdlib.h>

// Instance configuration
static char *instance_url = NULL;
static char *access_token = NULL;

int mastodon_init(const char *url, const char *token) {
    if (!url || !token) return -1;
    
    instance_url = strdup(url);
    access_token = strdup(token);
    
    return 0;
}

void mastodon_exit() {
    if (instance_url) free(instance_url);
    if (access_token) free(access_token);
}

mastodon_status_t* mastodon_fetch_timeline(int *count) {
    char url[512];
    char *response = NULL;
    json_t *root = NULL;
    json_t *statuses = NULL;
    mastodon_status_t *result = NULL;
    
    // Build URL
    snprintf(url, sizeof(url), "%s%s/timelines/home", instance_url, MASTODON_API_V1);
    
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
    
    statuses = json_object_get(root, "statuses");
    if (!statuses || !json_is_array(statuses)) {
        json_decref(root);
        return NULL;
    }
    
    // Count and allocate
    *count = json_array_size(statuses);
    result = calloc(*count, sizeof(mastodon_status_t));
    
    // Parse each status
    for (int i = 0; i < *count; i++) {
        json_t *status = json_array_get(statuses, i);
        if (!status) continue;
        
        result[i].id = strdup(json_string_value(json_object_get(status, "id")));
        result[i].content = strdup(json_string_value(json_object_get(status, "content")));
        
        json_t *account = json_object_get(status, "account");
        if (account) {
            result[i].author_username = strdup(json_string_value(json_object_get(account, "username")));
            result[i].author_display_name = strdup(json_string_value(json_object_get(account, "display_name")));
        }
        
        result[i].created_at = strdup(json_string_value(json_object_get(status, "created_at")));
    }
    
    json_decref(root);
    return result;
}

int mastodon_post_status(const char *text) {
    char url[512];
    char *response = NULL;
    json_t *root = NULL;
    int result = -1;
    
    if (!text) return -1;
    
    // Build URL
    snprintf(url, sizeof(url), "%s%s/statuses", instance_url, MASTODON_API_V1);
    
    // Build JSON payload
    json_t *payload = json_object();
    json_object_set_new(payload, "status", json_string(text));
    
    char *payload_str = json_dumps(payload, 0);
    json_decref(payload);
    
    // POST to API
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
    char url[512];
    char *response = NULL;
    json_t *root = NULL;
    mastodon_account_t *account = NULL;
    
    // Build URL
    snprintf(url, sizeof(url), "%s%s/accounts/verify_credentials", instance_url, MASTODON_API_V1);
    
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
    account->id = strdup(json_string_value(json_object_get(root, "id")));
    account->username = strdup(json_string_value(json_object_get(root, "username")));
    account->display_name = strdup(json_string_value(json_object_get(root, "display_name")));
    account->avatar_url = strdup(json_string_value(json_object_get(root, "avatar")));
    
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
