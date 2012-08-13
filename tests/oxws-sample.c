#include <libetpan/libetpan.h>

#include <stdlib.h>
#include <string.h>

static void check_error(int result, char* msg) {
  if (result == OXWS_NO_ERROR) return;
  fprintf(stderr, "%s. result: %d\n", msg, result);
  exit(EXIT_FAILURE);
}

oxws_result list_items(oxws* oxws, oxws_distinguished_folder_id folder_id) {
  carray* items = NULL;
  oxws_result result = oxws_list(oxws, folder_id, NULL, 10, &items);
  if(result != OXWS_NO_ERROR) return result;

  unsigned int i;
  for(i = 0; i < items->len; i++) {
    oxws_type_item* item = carray_get(items, i);
    printf("  %s\n", item->subject);
  }
  oxws_type_item_array_free(items);

  return result;
}

int main(int argc, char ** argv) {
  /* ./oxws-sample myhpiemail myhpiaccount myhpipassword */
  if (argc < 4) {
    fprintf(stderr, "usage: oxws-sample [HPI email address] [HPI user name] [password]\n");
    exit(EXIT_FAILURE);
  }

  oxws* oxws = oxws_new();
  if(oxws == NULL) {
    fprintf(stderr, "Could not create oxws instance.");
    exit(EXIT_FAILURE);
  }

  oxws_result result;
#if 1
  oxws_connection_settings settings;
  memset(&settings, 0, sizeof(settings));
  settings.as_url = "https://owa2.hpi.uni-potsdam.de/EWS/Exchange.asmx";
  result = oxws_set_connection_settings(oxws, &settings);
#else
  result = oxws_autodiscover_connection_settings(oxws, "owa2.hpi.uni-potsdam.de", argv[1], argv[2], argv[3], NULL);
#endif
  result = oxws_connect(oxws, argv[2], argv[3], NULL);
  check_error(result, "could not connect");

  puts("INBOX");
  result = list_items(oxws, OXWS_DISTFOLDER_INBOX);
  check_error(result, "could not list items in inbox");

  puts("SENT ITEMS");
  result = list_items(oxws, OXWS_DISTFOLDER_SENTITEMS);
  check_error(result, "could not list sent items");

  oxws_free(oxws);

  return 0;
}
