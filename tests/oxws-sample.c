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

  /* cleanup */
  oxws_free(oxws);
  return 0;
}

oxws_result list_items(oxws* oxws, oxws_distinguished_folder_id folder_id) {
  /* perform request */
  carray* items = NULL;
  oxws_result result = oxws_list(oxws, folder_id, NULL, 10, &items);
  if(result != OXWS_NO_ERROR) return result;

  /* list all items */
  unsigned int i;
  for(i = 0; i < items->len; i++) {
    oxws_type_item* item = carray_get(items, i);
    printf("  %s\n", item->subject ? item->subject->str : NULL);
  }

  /* clean up */
  oxws_type_item_array_free(items);
  return result;
}

static void check_error(oxws_result result, char* msg) {
  if (result == OXWS_NO_ERROR)
    return;

  /* print error and exit */
  fprintf(stderr, "%s. result: %s (%d)\n", msg, OXWS_ERROR_NAME(result), result);
  exit(EXIT_FAILURE);
}
