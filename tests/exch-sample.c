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
  ./exch-sample myhpiaccount myhpipassword
  */
  if (argc < 3) {
    fprintf(stderr, "usage: exch-sample [HPI user name] [password]\n");
    exit(EXIT_FAILURE);
  }

  exch = mailexch_new(0, NULL);
  if(exch == NULL) {
    fputs(stderr, "Could not create mailexch instance.");
    exit(EXIT_FAILURE);
  }

  result = mailexch_login(exch, "owa2.hpi.uni-potsdam.de", 0, argv[1], argv[2], NULL);
  check_error(result, "could not login");

  result = mailexch_list(exch, "inbox", 10, NULL);
  check_error(result, "could not list messages in inbox");
  puts("");

  mailexch_free(exch);

  return 0;
}
