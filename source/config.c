#include "config.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_FILE "sdmc:/fedii/config.json"

int config_init() {
    // Ensure directory exists
    Result res = FSUSER_CreateDirectory(
        fsMakePath(PATH_ASCII, "/fedii"),
        FS_ATTRIBUTE_DIRECTORY
    );
    
    // Ignore error if directory already exists
    return 0;
}

int config_load(config_t *config) {
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp) {
        return -1;
    }
    
    // Initialize config
    memset(config, 0, sizeof(config_t));
    
    // Read file
    char buffer[4096];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[bytes_read] = '\0';
    fclose(fp);
    
    // Simple JSON parsing
    char *instance_start = strstr(buffer, "\"instance_url\":");
    char *token_start = strstr(buffer, "\"access_token\":");
    char *username_start = strstr(buffer, "\"username\":");
    
    if (instance_start) {
        instance_start = strchr(instance_start, '"') + 1;
        char *instance_end = strchr(instance_start, '"');
        if (instance_end) {
            size_t len = instance_end - instance_start;
            config->instance_url = malloc(len + 1);
            strncpy(config->instance_url, instance_start, len);
            config->instance_url[len] = '\0';
        }
    }
    
    if (token_start) {
        token_start = strchr(token_start, '"') + 1;
        char *token_end = strchr(token_start, '"');
        if (token_end) {
            size_t len = token_end - token_start;
            config->access_token = malloc(len + 1);
            strncpy(config->access_token, token_start, len);
            config->access_token[len] = '\0';
        }
    }
    
    if (username_start) {
        username_start = strchr(username_start, '"') + 1;
        char *username_end = strchr(username_start, '"');
        if (username_end) {
            size_t len = username_end - username_start;
            config->username = malloc(len + 1);
            strncpy(config->username, username_start, len);
            config->username[len] = '\0';
        }
    }
    
    return 0;
}

int config_save(config_t *config) {
    FILE *fp = fopen(CONFIG_FILE, "w");
    if (!fp) {
        return -1;
    }
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"instance_url\": \"%s\",\n", config->instance_url ? config->instance_url : "");
    fprintf(fp, "  \"access_token\": \"%s\",\n", config->access_token ? config->access_token : "");
    fprintf(fp, "  \"username\": \"%s\"\n", config->username ? config->username : "");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

void config_free(config_t *config) {
    if (config) {
        free(config->instance_url);
        free(config->access_token);
        free(config->username);
        config->instance_url = NULL;
        config->access_token = NULL;
        config->username = NULL;
    }
}

// Create default config
void config_set_defaults(config_t *config) {
    memset(config, 0, sizeof(config_t));
    config->instance_url = strdup("https://mastodon.social");
    config->access_token = strdup("");
    config->username = strdup("");
    config->theme_color = COLOR_PINK;
}
