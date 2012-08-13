#include <libetpan/libetpan.h>

#include <stdlib.h>
#include <string.h>

static void check_error(int result, char* msg) {
  if (result == MAILEXCH_NO_ERROR) return;
  fprintf(stderr, "%s. result: %d\n", msg, result);
  exit(EXIT_FAILURE);
}

mailexch_result list_items(mailexch* exch, mailexch_distinguished_folder_id folder_id) {
  carray* items = NULL;
  mailexch_result result = mailexch_list(exch, folder_id, NULL, 10, &items);
  if(result != MAILEXCH_NO_ERROR) return result;

  unsigned int i;
  for(i = 0; i < items->len; i++) {
    mailexch_type_item* item = carray_get(items, i);
    printf("  %s\n", item->subject);
  }
  mailexch_type_item_array_free(items);

  return result;
}

int main(int argc, char ** argv) {
  /* ./exch-sample myhpiemail myhpiaccount myhpipassword */
  if (argc < 4) {
    fprintf(stderr, "usage: exch-sample [HPI email address] [HPI user name] [password]\n");
    exit(EXIT_FAILURE);
  }

  mailexch* exch = mailexch_new(0, NULL);
  if(exch == NULL) {
    fprintf(stderr, "Could not create mailexch instance.");
    exit(EXIT_FAILURE);
  }

  mailexch_result result;
#if 1
  mailexch_connection_settings settings;
  memset(&settings, 0, sizeof(settings));
  settings.as_url = "https://owa2.hpi.uni-potsdam.de/EWS/Exchange.asmx";
  result = mailexch_set_connection_settings(exch, &settings);
#else
  result = mailexch_autodiscover_connection_settings(exch, "owa2.hpi.uni-potsdam.de", argv[1], argv[2], argv[3], NULL);
#endif
  result = mailexch_connect(exch, argv[2], argv[3], NULL);
  check_error(result, "could not connect");

  puts("INBOX");
  result = list_items(exch, MAILEXCH_DISTFOLDER_INBOX);
  check_error(result, "could not list items in inbox");

  puts("SENT ITEMS");
  result = list_items(exch, MAILEXCH_DISTFOLDER_SENTITEMS);
  check_error(result, "could not list sent items");

  mailexch_free(exch);

  return 0;
}
