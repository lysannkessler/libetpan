#include <libetpan/libetpan.h>

#include <stdlib.h>
#include <string.h>


oxws_result list_items(oxws* oxws, oxws_distinguished_folder_id folder_id);
static void check_error(oxws_result result, char* msg);


int main(int argc, char ** argv) {
  /* ./oxws-sample myhpiemail myhpiaccount myhpipassword */
  if (argc < 4) {
    fprintf(stderr, "usage: oxws-sample [HPI email address] [HPI user name] [password]\n");
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
#if 1
  oxws_connection_settings settings;
  memset(&settings, 0, sizeof(settings));
  settings.as_url = "https://owa2.hpi.uni-potsdam.de/EWS/Exchange.asmx";
  result = oxws_set_connection_settings(oxws, &settings);
#else
  result = oxws_autodiscover_connection_settings(oxws, "owa2.hpi.uni-potsdam.de", argv[1], argv[2], argv[3], NULL);
#endif

  /* connect */
  result = oxws_connect(oxws, argv[2], argv[3], NULL);
  check_error(result, "could not connect");

  /* list items in Inbox */
  puts("INBOX");
  result = list_items(oxws, OXWS_DISTFOLDER_INBOX);
  check_error(result, "could not list items in inbox");

#if 0
  /* send message and save it to Sent Items */
  oxws_message* message = oxws_message_new();
  oxws_item_set_subject((oxws_item*) message, "[libetpan test] message from oxws-sample");
  oxws_item_set_body_fields((oxws_item*) message,
          "This is just another email sent using libetpan's Exchange implementation.\n" \
          "It's ok to feel annoyed by now :P",
          OXWS_BODY_TYPE_TEXT);
  message->to_recipients = carray_new(3);
  oxws_email_address* address = oxws_email_address_new(NULL, argv[1], NULL, OXWS_MAILBOX_TYPE__NOT_SET, NULL);
  carray_add(message->to_recipients, address, NULL);
  oxws_create_item(oxws, (oxws_item*) message, OXWS_MESSAGE_DISPOSITION_SEND_AND_SAVE_COPY, OXWS_DISTFOLDER_SENTITEMS, NULL);
#endif

#if 0
  /* list items in Sent Items */
  puts("SENT ITEMS");
  result = list_items(oxws, OXWS_DISTFOLDER_SENTITEMS);
  check_error(result, "could not list sent items");
#endif

  /* cleanup */
  oxws_free(oxws);
  return 0;
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
