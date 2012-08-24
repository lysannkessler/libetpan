#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _MSC_VER
# include "../src/bsd/getopt.h"
#else
# include <getopt.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <libetpan/libetpan.h>


int parse_options(int argc, char** argv,
      char** user, char** host, char** email, char** password,
      char** ews_url, short* send);
void print_usage();
oxws_result list_items(oxws* oxws, oxws_distinguished_folder_id folder_id);
static void check_error(oxws_result result, char* msg);


int main(int argc, char ** argv) {
  /* parse options */
  char *user, *host, *email, *password, *ews_url;
  short send;
  int r = parse_options(argc, argv, &user, &host, &email, &password, &ews_url, &send);
  if(r < 0) {
    fprintf(stderr, "parse_options: internal error\n");
    exit(EXIT_FAILURE);
  } else if(r > 0) {
    print_usage();
    exit(EXIT_FAILURE);
  }

  /* create instance */
  oxws* oxws = oxws_new();
  if(oxws == NULL) {
    fprintf(stderr, "Could not create oxws instance.");
    exit(EXIT_FAILURE);
  }

  /* configure connection settings */
  oxws_result result;
  if(ews_url) {
    oxws_connection_settings settings;
    memset(&settings, 0, sizeof(settings));
    settings.as_url = ews_url;
    result = oxws_set_connection_settings(oxws, &settings);
  } else {
    result = oxws_autodiscover_connection_settings(oxws, host, user, password, email, NULL);
  }

  /* connect */
  result = oxws_connect(oxws, user, password, NULL);
  check_error(result, "could not connect");

  /* list items in Inbox */
  puts("INBOX");
  result = list_items(oxws, OXWS_DISTFOLDER_INBOX);
  check_error(result, "could not list items in inbox");

  if(send) {
    /* send message and save it to Sent Items */
    oxws_message* message = oxws_message_new();
    oxws_item_set_subject_cstring((oxws_item*) message, "[libetpan test] message from oxws-sample");
    oxws_item_set_body_fields_cstring((oxws_item*) message,
            "This is just another email sent using libetpan's Exchange implementation.\n" \
            "It's ok to feel annoyed by now :P",
            OXWS_BODY_TYPE_TEXT);
    message->to_recipients = carray_new(3);
    oxws_email_address* address = oxws_email_address_new();
    oxws_email_address_set_email_address(address, user);
    carray_add(message->to_recipients, address, NULL);
    oxws_create_item(oxws, (oxws_item*) message, OXWS_MESSAGE_DISPOSITION_SEND_AND_SAVE_COPY, OXWS_DISTFOLDER_SENTITEMS, NULL);

    /* list items in Sent Items */
    puts("SENT ITEMS");
    result = list_items(oxws, OXWS_DISTFOLDER_SENTITEMS);
    check_error(result, "could not list sent items");
  }

  /* cleanup */
  oxws_free(oxws);
  return 0;
}


int parse_options(int argc, char** argv,
      char** user, char** host, char** email, char** password,
      char** ews_url, short* send) {

  static const char* short_options = "u:p:h:e:x:s";
#if HAVE_GETOPT_LONG
  static struct option long_options[] = {
    {"user",     1, 0, 'u'},
    {"host",     1, 0, 'h'},
    {"email",    1, 0, 'e'},
    {"password", 1, 0, 'p'},
    {"url",      1, 0, 'x'},
    {"send",     0, 0, 's'},
  };
#endif
  int r = 0;

  if(user == NULL || password == NULL || host == NULL || email == NULL || \
     ews_url == NULL || send == NULL)
    return -1;

  *user = NULL;
  *password = NULL;
  *host = NULL;
  *email = NULL;
  *ews_url = NULL;
  *send = 0;

  while (r != -1) {
#if HAVE_GETOPT_LONG
    r = getopt_long(argc, argv, short_options, long_options, NULL);
#else
    r = getopt(argc, argv, short_options);
#endif
    switch (r) {
    case 'u': *user = strdup(optarg); break;
    case 'h': *host = strdup(optarg); break;
    case 'e': *email = strdup(optarg); break;

    case 'p': *password = strdup(optarg); break;

    case 'x': *ews_url = strdup(optarg); break;
    case 's': *send = 1; break;

    case -1: break;
    default: fprintf(stderr, "error: unknown argument %c.\n", r); break;
    }
  }

  /* check for existance of password */
  if(*password == NULL) {
    fprintf(stderr, "error: missing argument password.\n");
    return 2;
  }

  /* derive user and host from email */
  if(*user == NULL || *host == NULL) {
    if(*email == NULL) {
      fprintf(stderr, "error: email must be given if user or host are missing.\n");
      return 2;
    }

    char* delim = strstr(*email, "@");
    if(delim == NULL) {
      fprintf(stderr, "error: missing @ in email.\n");
      return 3;
    }
    /* user */
    if(*user == NULL) {
      size_t length = delim - *email;
      *user = (char*) malloc(length + 1);
      if(*user == NULL)
        return -2;
      memcpy(*user, *email, length);
      *user[length] = 0;
    }
    /* host */
    if(*host == NULL) {
      if(*(delim + 1) == 0) {
        fprintf(stderr, "error: no host found in email.\n");
        return 3;
      }
      *host = delim + 1;
    }
  }

  /* derive email from user and host */
  if(*email == NULL) {
    if(*user == NULL || *host == NULL) {
      fprintf(stderr, "error: user and host must be given if email is missing.\n");
      return 2;
    }
    size_t length = strlen(*user) + strlen(*host) + 1;
    *email = (char*) malloc(length);
    if(*email == NULL)
      return -2;
    sprintf(*email, "%s@%s", *user, *host);
  }

  return 0;
}

void print_usage() {
  fprintf(stderr, "usage: oxws-sample [options]\n\n");
  fprintf(stderr, "options:\n" \
                  "    --user     -u [USER]    username (optional). If missing, derive from email.\n" \
                  "    --host     -h [HOST]    host (optional). If missing, derive from email.\n" \
                  "    --email    -e [EMAIL]   email address (optional). If missing, derive from user and host.\n\n" \
                  "    --password -p [PWD]     password (required)\n\n" \
                  "    --url      -x [URL]     EWS URL (optional). If missing, use autodiscover.\n" \
                  "    --send     -s           send a test email to oneself\n");
}


oxws_result list_items(oxws* oxws, oxws_distinguished_folder_id folder_id) {
  /* perform request */
  carray* items = NULL;
  oxws_result result = oxws_find_item(oxws, folder_id, NULL, 10, &items);
  if(result != OXWS_NO_ERROR) return result;

  /* list all items */
  unsigned int i;
  for(i = 0; i < items->len; i++) {
    oxws_item* item = carray_get(items, i);
    printf("- %s\n", item->subject ? item->subject->str : NULL);
    if(item->size != OXWS_OPTIONAL_INT32__NOT_SET)
      printf("  - size: %d\n", item->size);

    if(OXWS_ITEM_IS_MESSAGE(item)) {
      oxws_message* message = (oxws_message*) item;

      if(message->is_read == OXWS_OPTIONAL_BOOLEAN_TRUE) {
        puts("  - is_read: true");
      } else if(message->is_read == OXWS_OPTIONAL_BOOLEAN_FALSE) {
        puts("  - is_read: false");
      }

      if(message->from != NULL) {
        if(message->from->name != NULL && message->from->email_address != NULL) {
          printf("  - from: %s <%s>\n", message->from->name, message->from->email_address);
        } else if(message->from->name != NULL) {
          printf("  - from: %s\n", message->from->name);
        } else if(message->from->email_address != NULL) {
          printf("  - from: <%s>\n", message->from->email_address);
        }
      }
    }
  }

  /* clean up */
  oxws_item_array_free(items);
  return result;
}

static void check_error(oxws_result result, char* msg) {
  if (result == OXWS_NO_ERROR)
    return;

  /* print error and exit */
  fprintf(stderr, "%s. result: %s (%d)\n", msg, OXWS_ERROR_NAME(result), result);
  exit(EXIT_FAILURE);
}
