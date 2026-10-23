#include "network.h"
#include <3ds.h>
#include <stdlib.h>
#include <string.h>

// Timeout in seconds
static int timeout_seconds = 30;

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

int network_get(const char *url, const char *auth_token, char **response) {
    // Parse URL
    char host[256];
    char path[512];
    int port = 80;
    
    // Simple URL parsing (would use proper parser in full implementation)
    if (strncmp(url, "https://", 8) == 0) {
        port = 443;
        sscanf(url + 8, "%255[^/]/%511s", host, path);
    } else if (strncmp(url, "http://", 7) == 0) {
        sscanf(url + 7, "%255[^/]/%511s", host, path);
    } else {
        sscanf(url, "%255[^/]/%511s", host, path);
    }
    
    if (path[0] == 0) strcpy(path, "/");
    
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
    char request[1024];
    snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Authorization: Bearer %s\r\n"
        "Accept: application/json\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, host, auth_token ? auth_token : "");
    
    // Send request
    if (send(sock, request, strlen(request), 0) < 0) {
        close(sock);
        return -1;
    }
    
    // Receive response
    char *buffer = malloc(4096);
    int received = 0;
    int total = 0;
    
    while ((received = recv(sock, buffer + total, 4096 - total, 0)) > 0) {
        total += received;
        if (total >= 4096 - 1) break;
    }
    
    close(sock);
    
    if (total <= 0) {
        free(buffer);
        return -1;
    }
    
    buffer[total] = '\0';
    
    // Extract body from HTTP response (skip headers)
    char *body = strstr(buffer, "\r\n\r\n");
    if (!body) {
        free(buffer);
        return -1;
    }
    
    body += 4;
    *response = strdup(body);
    free(buffer);
    
    return 0;
}

int network_post(const char *url, const char *auth_token, const char *body, char **response) {
    // Parse URL
    char host[256];
    char path[512];
    int port = 80;
    
    if (strncmp(url, "https://", 8) == 0) {
        port = 443;
        sscanf(url + 8, "%255[^/]/%511s", host, path);
    } else if (strncmp(url, "http://", 7) == 0) {
        sscanf(url + 7, "%255[^/]/%511s", host, path);
    } else {
        sscanf(url, "%255[^/]/%511s", host, path);
    }
    
    if (path[0] == 0) strcpy(path, "/");
    
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
    int body_len = body ? strlen(body) : 0;
    char request[2048];
    snprintf(request, sizeof(request),
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Authorization: Bearer %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Accept: application/json\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, host, auth_token ? auth_token : "", body_len);
    
    // Send request
    if (send(sock, request, strlen(request), 0) < 0) {
        close(sock);
        return -1;
    }
    
    // Send body
    if (body_len > 0) {
        if (send(sock, body, body_len, 0) < 0) {
            close(sock);
            return -1;
        }
    }
    
    // Receive response
    char *recv_buffer = malloc(4096);
    int received = 0;
    int total = 0;
    
    while ((received = recv(sock, recv_buffer + total, 4096 - total, 0)) > 0) {
        total += received;
        if (total >= 4096 - 1) break;
    }
    
    close(sock);
    
    if (total <= 0) {
        free(recv_buffer);
        return -1;
    }
    
    recv_buffer[total] = '\0';
    
    // Extract body from HTTP response
    char *response_body = strstr(recv_buffer, "\r\n\r\n");
    if (!response_body) {
        free(recv_buffer);
        return -1;
    }
    
    response_body += 4;
    *response = strdup(response_body);
    free(recv_buffer);
    
    return 0;
}

void network_set_timeout(int seconds) {
    timeout_seconds = seconds;
}
