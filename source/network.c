#include "network.h"
#include <3ds.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Timeout in seconds
static int timeout_seconds = 30;

// User agent
#define USER_AGENT "fedii/1.0"

int network_init() {
    // Initialize socket service
    Result res = socketInit();
    if (R_FAILED(res)) {
        return -1;
    }
    return 0;
}

void network_exit() {
    socketExit();
}

// Parse URL into host, port, and path
static int parse_url(const char *url, char *host, int host_len, int *port, char *path, int path_len) {
    if (!url || !host || !port || !path) return -1;
    
    host[0] = '\0';
    path[0] = '\0';
    *port = 80;
    
    // Check for https
    if (strncmp(url, "https://", 8) == 0) {
        *port = 443;
        url += 8;
    } else if (strncmp(url, "http://", 7) == 0) {
        url += 7;
    }
    
    // Find path separator
    char *path_start = strchr(url, '/');
    
    if (path_start) {
        // Extract host
        int host_len_max = (path_start - url) < host_len ? (path_start - url) : (host_len - 1);
        strncpy(host, url, host_len_max);
        host[host_len_max] = '\0';
        
        // Extract path
        strncpy(path, path_start, path_len - 1);
        path[path_len - 1] = '\0';
    } else {
        // No path, just host
        strncpy(host, url, host_len - 1);
        host[host_len - 1] = '\0';
        strcpy(path, "/");
    }
    
    return 0;
}

// Create HTTP request
static char* create_request(const char *method, const char *path, const char *host, 
                           const char *auth_token, const char *body, int *content_length) {
    static char request[4096];
    char *body_ptr = NULL;
    
    if (body) {
        *content_length = strlen(body);
    } else {
        *content_length = 0;
    }
    
    if (strcmp(method, "GET") == 0) {
        snprintf(request, sizeof(request),
            "%s %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: " USER_AGENT "\r\n"
            "Accept: application/json\r\n"
            "Connection: close\r\n"
            "\r\n",
            method, path, host);
    } else {
        snprintf(request, sizeof(request),
            "%s %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: " USER_AGENT "\r\n"
            "Accept: application/json\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n",
            method, path, host, *content_length);
        body_ptr = (char*)body;
    }
    
    return strdup(request);
}

// Receive all data from socket
static char* receive_all(int sock, int *total_bytes) {
    char *buffer = malloc(16384);
    char *temp = malloc(4096);
    int received = 0;
    int total = 0;
    *total_bytes = 0;
    
    if (!buffer || !temp) {
        free(buffer);
        free(temp);
        return NULL;
    }
    
    while ((received = recv(sock, temp, 4095, 0)) > 0) {
        if (total + received >= 16384) {
            // Resize buffer
            buffer = realloc(buffer, total + received + 1);
        }
        memcpy(buffer + total, temp, received);
        total += received;
        buffer[total] = '\0';
    }
    
    free(temp);
    *total_bytes = total;
    return buffer;
}

// Extract body from HTTP response
static char* extract_body(char *response, int response_len) {
    char *body = strstr(response, "\r\n\r\n");
    if (!body) return NULL;
    
    body += 4;
    
    // Check for chunked encoding
    if (strstr(response, "Transfer-Encoding: chunked")) {
        // Handle chunked encoding (simplified)
        return strdup(body);
    }
    
    // Check for Content-Length
    char *content_length_str = strstr(response, "Content-Length: ");
    if (content_length_str) {
        int content_length = atoi(content_length_str + 16);
        if (content_length > 0) {
            // Allocate and copy body
            char *body_copy = malloc(content_length + 1);
            memcpy(body_copy, body, content_length);
            body_copy[content_length] = '\0';
            return body_copy;
        }
    }
    
    // Default: return from end of headers to end of response
    return strdup(body);
}

int network_get(const char *url, const char *auth_token, char **response) {
    char host[256];
    char path[512];
    int port = 80;
    
    // Parse URL
    if (parse_url(url, host, sizeof(host), &port, path, sizeof(path)) != 0) {
        return -1;
    }
    
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }
    
    // Set timeout
    struct timeval tv;
    tv.tv_sec = timeout_seconds;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    // Resolve host
    struct hostent *he = gethostbyname(host);
    if (!he) {
        close(sock);
        return -1;
    }
    
    // Connect
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    
    // Build request
    int content_length = 0;
    char *http_request = create_request("GET", path, host, auth_token, NULL, &content_length);
    
    // Send request
    if (send(sock, http_request, strlen(http_request), 0) < 0) {
        free(http_request);
        close(sock);
        return -1;
    }
    
    free(http_request);
    
    // Receive response
    int total_bytes = 0;
    char *recv_buffer = receive_all(sock, &total_bytes);
    close(sock);
    
    if (!recv_buffer || total_bytes <= 0) {
        free(recv_buffer);
        return -1;
    }
    
    // Extract body
    *response = extract_body(recv_buffer, total_bytes);
    free(recv_buffer);
    
    if (!*response) {
        return -1;
    }
    
    return 0;
}

int network_post(const char *url, const char *auth_token, const char *body, char **response) {
    char host[256];
    char path[512];
    int port = 80;
    
    // Parse URL
    if (parse_url(url, host, sizeof(host), &port, path, sizeof(path)) != 0) {
        return -1;
    }
    
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }
    
    // Set timeout
    struct timeval tv;
    tv.tv_sec = timeout_seconds;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    // Resolve host
    struct hostent *he = gethostbyname(host);
    if (!he) {
        close(sock);
        return -1;
    }
    
    // Connect
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    
    // Build request
    int content_length = body ? strlen(body) : 0;
    char *http_request = create_request("POST", path, host, auth_token, body, &content_length);
    
    // Send request
    if (send(sock, http_request, strlen(http_request), 0) < 0) {
        free(http_request);
        close(sock);
        return -1;
    }
    
    // Send body
    if (body && content_length > 0) {
        if (send(sock, body, content_length, 0) < 0) {
            free(http_request);
            close(sock);
            return -1;
        }
    }
    
    free(http_request);
    
    // Receive response
    int total_bytes = 0;
    char *recv_buffer = receive_all(sock, &total_bytes);
    close(sock);
    
    if (!recv_buffer || total_bytes <= 0) {
        free(recv_buffer);
        return -1;
    }
    
    // Extract body
    *response = extract_body(recv_buffer, total_bytes);
    free(recv_buffer);
    
    if (!*response) {
        return -1;
    }
    
    return 0;
}

void network_set_timeout(int seconds) {
    timeout_seconds = seconds;
}
