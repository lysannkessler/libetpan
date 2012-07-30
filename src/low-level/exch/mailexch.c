/*
 * libEtPan! -- a mail stuff library
 *
 * exhange support: Copyright (C) 2012 Lysann Kessler
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the libEtPan! project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "mailexch.h"

#include <stdlib.h>
#include <string.h>


size_t mailexch_header_callback(void* ptr, size_t size, size_t nmemb, void* userdata);
size_t mailexch_write_callback( char *ptr, size_t size, size_t nmemb, void *userdata);

/*
  mailexch structure
*/

mailexch* mailexch_new(size_t progr_rate, progress_function* progr_fun)
{
  mailexch* exch;

  exch = malloc(sizeof(* exch));
  if (exch == NULL)
    return NULL;

  exch->exch_progr_rate = progr_rate;
  exch->exch_progr_fun = progr_fun;

  return exch;
}

void mailexch_free(mailexch* exch) {
  if(!exch) return;

  if(exch->host) {
    free(exch->host);
    exch->host = NULL;
  }

  if(exch->curl) {
    curl_easy_cleanup(exch->curl);
    exch->curl = NULL;
  }
}

int mailexch_ssl_connect(mailexch* exch, const char* host, uint16_t port) {
  size_t host_length;
  char* url;
  CURLcode curl_code;
  long http_response = 0;
  int result = MAILEXCH_NO_ERROR;

  exch->curl = curl_easy_init();
  if(!exch->curl) return MAILEXCH_ERROR_INTERNAL;
  curl_easy_setopt(exch->curl, CURLOPT_VERBOSE, 1L);

  /* For connection, we just GET the wsdl. This will probably result in a 401,
     but that's ok right now. Thus, after the connection attempt we know which
     authentication protocol to use. */

  /* url and port */
  host_length = strlen(host);
  exch->host = (char*) malloc(host_length+1);
  if(!exch->host) return MAILEXCH_ERROR_INTERNAL;
  memcpy(exch->host, host, host_length+1);
  /* url = https://[host]/ews/services.wsdl\0 */
  url = (char*) malloc(8 + host_length + 18 + 1);
  if(!url) return MAILEXCH_ERROR_INTERNAL;
  sprintf(url, "https://%s/ews/services.wsdl", host);
  curl_easy_setopt(exch->curl, CURLOPT_URL, url);
  free(url);
  if(port != 0) curl_easy_setopt(exch->curl, CURLOPT_PORT, port);

  /* we want to read the header only */
  curl_easy_setopt(exch->curl, CURLOPT_NOBODY, 1L);
  curl_easy_setopt(exch->curl, CURLOPT_HEADERFUNCTION, mailexch_header_callback);
  curl_easy_setopt(exch->curl, CURLOPT_WRITEHEADER, exch);
  exch->auth_protocol = 0;
  /* This way the mailexch_header_callback sets the auth protocol according
     to any WWW-Authenticate header. */

  /* perform request */
  curl_code = curl_easy_perform(exch->curl);
  if(curl_code != CURLE_OK) {
    result = MAILEXCH_ERROR_CONNECT;
  } else {
    curl_easy_getinfo (exch->curl, CURLINFO_RESPONSE_CODE, &http_response);
    if(http_response != 200 && http_response != 401) {
      result = MAILEXCH_ERROR_CONNECT;
    }
  }

  return result;
}

int mailexch_login(mailexch* exch, const char* username, const char* password, const char* domain) {
  size_t username_length = username ? strlen(username) : 0;
  size_t password_length = password ? strlen(password) : 0;
  size_t domain_length = domain ? strlen(domain) : 0;
  char* userpwd = NULL;

  CURLcode curl_code;
  long http_response = 0;
  int result = MAILEXCH_NO_ERROR;

  char* url;

  /* Here we set credentials and the determined authentication protocol.
     The server should respond with 200. */

  if(domain_length > 0) {
    userpwd = (char*) malloc(domain_length + 1 + username_length + 1 + password_length + 1); /* last +1 for \0 */
    if(!userpwd) return MAILEXCH_ERROR_INTERNAL;
    sprintf(userpwd, "%s\\%s:%s", domain, username, password);
  } else {
    userpwd = (char*) malloc(username_length + 1 + password_length + 1); /* last +1 for \0 */
    if(!userpwd) return MAILEXCH_ERROR_INTERNAL;
    sprintf(userpwd, "%s:%s", username, password);
  }

  curl_easy_setopt(exch->curl, CURLOPT_USERPWD, userpwd);
  free(userpwd);
  curl_easy_setopt(exch->curl, CURLOPT_HTTPAUTH, exch->auth_protocol);

  /* we need the response code only */
  curl_easy_setopt(exch->curl, CURLOPT_NOBODY, 1L);

  /* perform request */
  curl_code = curl_easy_perform(exch->curl);
  if(curl_code != CURLE_OK) {
    result = MAILEXCH_ERROR_CONNECT;
  } else {
    curl_easy_getinfo (exch->curl, CURLINFO_RESPONSE_CODE, &http_response);
    if(http_response == 200) {
      /* from now on we use POST to the asmx */
      curl_easy_setopt(exch->curl, CURLOPT_POST, 1L);
      /* url = https://[host]/EWS/Exchange.asmx\0 */
      url = (char*) malloc(8 + strlen(exch->host) + 18 + 1);
      if(!url) return MAILEXCH_ERROR_INTERNAL;
      sprintf(url, "https://%s/EWS/Exchange.asmx", exch->host);
      curl_easy_setopt(exch->curl, CURLOPT_URL, url);
      free(url);
    } else {
      result = MAILEXCH_ERROR_CONNECT;
    }
  }

  return result;
}

int mailexch_list(mailexch* exch, const char* folder_name, int count, carray** list) {
  CURLcode curl_code;
  long http_response = 0;
  int result = MAILEXCH_NO_ERROR;

  const char* request_format =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
    "  <soap:Body>\n"
    "    <FindItem xmlns=\"http://schemas.microsoft.com/exchange/services/2006/messages\"\n"
    "              xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\"\n"
    "              Traversal=\"Shallow\">\n"
    "      <ItemShape>\n"
    "        <t:BaseShape>IdOnly</t:BaseShape>\n"
    "        <t:AdditionalProperties>\n"
    "          <t:FieldURI FieldURI=\"item:Subject\"/>\n"
    "        </t:AdditionalProperties>\n"
    "      </ItemShape>\n"
    "      <IndexedPageItemView %s BasePoint=\"Beginning\" Offset=\"0\" />\n"
    "      <ParentFolderIds>\n"
    "        <t:DistinguishedFolderId Id=\"inbox\"/>\n"
    "      </ParentFolderIds>\n"
    "    </FindItem>\n"
    "  </soap:Body>\n"
    "</soap:Envelope>";
  size_t request_length = strlen(request_format) - 2; /* without format arguments */
  const char* max_entries_returned_format = "MaxEntriesReturned=\"%d\"";
  char* request, *max_entries_returned = NULL;

  if(count >= 0) {
    /* MaxEntriesReturned="" + max uint length when printed + \0 */
    max_entries_returned = (char*) malloc(21 + 10 + 1);
    if(!max_entries_returned) return MAILEXCH_ERROR_INTERNAL;
    sprintf(max_entries_returned, max_entries_returned_format, count);
    request_length += strlen(max_entries_returned);
  }

  request = (char*) malloc(request_length + 1);
  if(!request) {
    if(max_entries_returned) free(max_entries_returned);
    return MAILEXCH_ERROR_INTERNAL;
  }
  sprintf(request, request_format,
          max_entries_returned ? max_entries_returned : "");

  if(max_entries_returned) free(max_entries_returned);

  /* headers */
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: text/xml");
  curl_easy_setopt(exch->curl, CURLOPT_HTTPHEADER, headers);

  /* content */
  curl_easy_setopt(exch->curl, CURLOPT_POSTFIELDS, request);

  /* result */
  curl_easy_setopt(exch->curl, CURLOPT_WRITEFUNCTION, mailexch_write_callback);
  curl_easy_setopt(exch->curl, CURLOPT_WRITEDATA, exch);

  /* perform request */
  curl_code = curl_easy_perform(exch->curl);
  if(curl_code != CURLE_OK) {
    result = MAILEXCH_ERROR_CANT_LIST;
  } else {
    curl_easy_getinfo (exch->curl, CURLINFO_RESPONSE_CODE, &http_response);
    if(http_response != 200) {
      result = MAILEXCH_ERROR_CANT_LIST;
    }
  }

  free(request);
  curl_slist_free_all(headers);
  return result;
}

/*
  mailexch structure callbacks
*/

size_t mailexch_header_callback(void* ptr, size_t size, size_t nmemb, void* userdata) {
  mailexch* exch = (mailexch*) userdata;
  const char* auth_header = "WWW-Authenticate: ";
  const size_t auth_header_length = strlen(auth_header);
  size_t header_length = size * nmemb;

  /* ptr may not be null terminated! */

  if(exch->auth_protocol == 0) {
    if(strncmp(ptr, auth_header, header_length < auth_header_length ? header_length : auth_header_length) == 0) {
      ptr += auth_header_length;
      header_length -= auth_header_length;

      if(strncmp(ptr, "Negotiate", header_length < 9 ? header_length : 9) == 0) {
        /* no-op. we just don't want to invoke the else branch. */
      } else if(strncmp(ptr, "NTLM", header_length < 4 ? header_length : 4) == 0) {
        exch->auth_protocol = CURLAUTH_NTLM;
      } else if(strncmp(ptr, "Basic", header_length < 5 ? header_length : 5) == 0) {
        exch->auth_protocol = CURLAUTH_BASIC;
      } else {
        exch->auth_protocol = CURLAUTH_ANY;
      }
    }
  }

  return size * nmemb;
}

size_t mailexch_write_callback( char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t response_length = size*nmemb < 1 ? 0 : size*nmemb;
  char* response = (char*) malloc(response_length + 1);
  memcpy(response, ptr, response_length);
  response[response_length] = 0;
  printf("%s", response);

  return response_length;
}
