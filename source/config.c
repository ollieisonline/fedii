#include "config.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_FILE "sdmc:/fedii/config.json"

int config_init() {
    // Create directory if it doesn't exist
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
    
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), fp)) {
        // Simple JSON parsing (would use jansson in full implementation)
        char *token = strtok(buffer, "{":\r\n\t ");
        while (token) {
            if (strstr(token, "instance_url")) {
                token = strtok(NULL, "\"\r\n\t ");
                if (token) config->instance_url = strdup(token);
            } else if (strstr(token, "access_token")) {
                token = strtok(NULL, "\"\r\n\t ");
                if (token) config->access_token = strdup(token);
            } else if (strstr(token, "username")) {
                token = strtok(NULL, "\"\r\n\t ");
                if (token) config->username = strdup(token);
            }
            token = strtok(NULL, "{":\r\n\t ");
        }
    }
    
    fclose(fp);
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
    }
}
