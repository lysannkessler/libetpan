#include <libetpan/libetpan.h>

#include <curl/curl.h>

#include <stdlib.h>
#include <string.h>

size_t write_callback( char *ptr, size_t size, size_t nmemb, void *userdata)
{
  size_t response_length = size*nmemb < 1 ? 0 : size*nmemb;
  char* response = (char*) malloc(response_length + 1);
  memcpy(response, ptr, response_length);
  response[response_length] = 0;
  printf("%s", response);

  return response_length;
}

int main(int argc, char ** argv)
{
  /*
  ./exch-sample myhpiaccount myhpipassword
  */
  if (argc < 3) {
    fprintf(stderr, "usage: exch-sample [HPI user name] [password]\n");
    exit(EXIT_FAILURE);
  }

#if 0
  struct mailexch* exch;
  int r;

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
#endif

  CURL *curl;
  CURLcode res;
  char* message =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
    "  <soap:Body>\n"
    "    <GetFolder xmlns=\"http://schemas.microsoft.com/exchange/services/2006/messages\"\n"
    "               xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\">\n"
    "      <FolderShape>\n"
    "        <t:BaseShape>Default</t:BaseShape>\n"
    "      </FolderShape>\n"
    "      <FolderIds>\n"
    "        <t:DistinguishedFolderId Id=\"inbox\"/>\n"
    "        <t:DistinguishedFolderId Id=\"sentitems\"/>\n"
    "      </FolderIds>\n"
    "    </GetFolder>\n"
    "  </soap:Body>\n"
    "</soap:Envelope>";
  long response_code = 0;

  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();
  if(curl) {
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    // method and url
    curl_easy_setopt(curl, CURLOPT_URL, "https://owa2.hpi.uni-potsdam.de/EWS/Exchange.asmx");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // authentication
    curl_easy_setopt(curl, CURLOPT_USERNAME, argv[1]);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, argv[2]);

    // headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: text/xml");
    headers = curl_slist_append(headers, "SOAPAction: ");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // content
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message);

    // result
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // perform request
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    else {
      curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &response_code);
      if(response_code == 401) {
        // try with NTLM authentication
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_NTLM);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      }
      puts("");
    }

    // clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return 0;
}