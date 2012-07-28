#include <libetpan/libetpan.h>

#include <stdlib.h>

int main(int argc, char ** argv)
{
  struct mailexch* exch;
  int r;

  /*
  ./exch-sample mygmailaccount@googlemail.com mygmailpassword
  */
  if (argc < 3) {
    fprintf(stderr, "usage: exch-sample [gmail-email-address] [password]\n");
    exit(EXIT_FAILURE);
  }

  exch = mailexch_new(0, NULL);
  /*r = mailexch_ssl_connect(exch, "m.google.com", 993);
  fprintf(stderr, "connect: %i\n", r);
  check_error(r, "could not connect to server");

  r = mailexch_login(exch, argv[1], argv[2]);
  check_error(r, "could not login");

  r = mailexch_select(exch, "INBOX");
  check_error(r, "could not select INBOX");

  fetch_messages(exch);

  mailexch_logout(exch);
  mailexch_free(exch);*/

  exit(EXIT_SUCCESS);
}
