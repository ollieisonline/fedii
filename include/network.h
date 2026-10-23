#ifndef NETWORK_H
#define NETWORK_H

// HTTP methods
typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE
} http_method_t;

// Initialize network
int network_init();

// Exit network
void network_exit();

// GET request
int network_get(const char *url, const char *auth_token, char **response);

// POST request
int network_post(const char *url, const char *auth_token, const char *body, char **response);

// Set timeout
void network_set_timeout(int seconds);

#endif // NETWORK_H
