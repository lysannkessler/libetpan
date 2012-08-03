#include <libetpan/libetpan.h>

#include <stdlib.h>

static void check_error(int result, char* msg) {
  if (result == MAILEXCH_NO_ERROR) return;
  fprintf(stderr, "%s. result: %d\n", msg, result);
  exit(EXIT_FAILURE);
}

int main(int argc, char ** argv) {
  struct mailexch* exch;
  int result;

  /*
  ./exch-sample myhpiemail myhpiaccount myhpipassword
  */
  if (argc < 4) {
    fprintf(stderr, "usage: exch-sample [HPI email address] [HPI user name] [password]\n");
    exit(EXIT_FAILURE);
  }

  exch = mailexch_new(0, NULL);
  if(exch == NULL) {
    fprintf(stderr, "Could not create mailexch instance.");
    exit(EXIT_FAILURE);
  }

#if 1
  mailexch_connection_settings settings;
  settings.as_url = "https://owa2.hpi.uni-potsdam.de/EWS/Exchange.asmx";
  result = mailexch_set_connection_settings(exch, &settings);
#else
  result = mailexch_autodiscover_connection_settings(exch,
      "owa2.hpi.uni-potsdam.de", argv[1], argv[2], argv[3], NULL);
#endif
  result = mailexch_connect(exch, argv[2], argv[3], NULL);
  check_error(result, "could not connect");

  puts("INBOX");
  result = mailexch_list(exch, MAILEXCH_DISTFOLDER_INBOX, NULL, 10, NULL);
  check_error(result, "could not list items in inbox");

  puts("SENT ITEMS");
  result = mailexch_list(exch, MAILEXCH_DISTFOLDER_SENTITEMS, NULL, 10, NULL);
  check_error(result, "could not list sent items");

  mailexch_free(exch);

  return 0;
}
