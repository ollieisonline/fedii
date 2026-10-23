#ifndef CONFIG_H
#define CONFIG_H

// Configuration structure
typedef struct {
    char *instance_url;
    char *access_token;
    char *username;
    int theme_color;
} config_t;

// Initialize config
int config_init();

// Load config from file
int config_load(config_t *config);

// Save config to file
int config_save(config_t *config);

// Free config
void config_free(config_t *config);

#endif // CONFIG_H
