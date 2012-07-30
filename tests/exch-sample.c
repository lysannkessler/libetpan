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
  result = mailexch_connect(exch, "https://owa2.hpi.uni-potsdam.de/EWS/Exchange.asmx", argv[2], argv[3], NULL);
#else
  result = mailexch_connect_autodiscover(exch, argv[1], "owa2.hpi.uni-potsdam.de", argv[2], argv[3], NULL);
#endif
  check_error(result, "could not login");

  result = mailexch_list(exch, "inbox", 10, NULL);
  check_error(result, "could not list messages in inbox");

  mailexch_free(exch);

  return 0;
}
