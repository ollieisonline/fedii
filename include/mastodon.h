#ifndef MASTODON_H
#define MASTODON_H

#include <jansson.h>

// Status/toot structure
typedef struct {
    char *id;
    char *content;
    char *author_username;
    char *author_display_name;
    char *created_at;
} mastodon_status_t;

// Account structure
typedef struct {
    char *id;
    char *username;
    char *display_name;
    char *avatar_url;
} mastodon_account_t;

// Initialize Mastodon client
int mastodon_init(const char *instance_url, const char *access_token);

// Cleanup Mastodon client
void mastodon_exit();

// Fetch timeline
mastodon_status_t* mastodon_fetch_timeline(int *count);

// Post a status
int mastodon_post_status(const char *text);

// Get account info
mastodon_account_t* mastodon_get_account();

// Free status array
void mastodon_free_statuses(mastodon_status_t *statuses, int count);

// Free account
void mastodon_free_account(mastodon_account_t *account);

#endif // MASTODON_H
