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

#include <libetpan/mailexch.h>
#include <libetpan/mailexch_helper.h>

#include <stdlib.h>
#include <string.h>


#define MAILEXCH_FREE(obj) \
  if(obj) { \
    free(obj); \
    obj = NULL; \
  }


/* see mailexch_autodiscover.c */
int mailexch_autodiscover(mailexch* exch, const char* email_address, const char* host,
    const char* username, const char* password, const char* domain,
    mailexch_connection_settings* settings);

size_t mailexch_test_connection_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);


/*
  mailexch structure
*/

mailexch* mailexch_new(size_t progr_rate, progress_function* progr_fun)
{
  mailexch* exch;

  exch = calloc(1, sizeof(* exch));
  if (exch == NULL)
    return NULL;

  exch->exch_progr_rate = progr_rate;
  exch->exch_progr_fun = progr_fun;

  exch->response_buffer = mmap_string_sized_new(MAILEXCH_DEFAULT_RESPONSE_BUFFER_LENGTH);

  return exch;
}

void mailexch_free(mailexch* exch) {
  if(!exch) return;

  MAILEXCH_FREE(exch->connection_settings.as_url);
  MAILEXCH_FREE(exch->connection_settings.oof_url);
  MAILEXCH_FREE(exch->connection_settings.um_url);
  MAILEXCH_FREE(exch->connection_settings.oab_url);

  if(exch->curl) {
    curl_easy_cleanup(exch->curl);
    exch->curl = NULL;
  }

  mmap_string_free(exch->response_buffer);
}

#define MAILEXCH_COPY_STRING(result, dest, source) \
  if(result == MAILEXCH_NO_ERROR && source) { \
    (dest) = realloc((dest), strlen(source) + 1); \
    if(!(dest)) { \
      result = MAILEXCH_ERROR_INTERNAL; \
    } else { \
      memcpy((dest), (source), strlen(source) + 1); \
    } \
  } \
  if(result != MAILEXCH_NO_ERROR || source == NULL) { \
    if(dest) free(dest); \
    (dest) = NULL; \
  }

int mailexch_set_connection_settings(mailexch* exch, mailexch_connection_settings* settings) {
  int result = MAILEXCH_NO_ERROR;
  MAILEXCH_COPY_STRING(result, exch->connection_settings.as_url,  settings->as_url);
  MAILEXCH_COPY_STRING(result, exch->connection_settings.oof_url, settings->oof_url);
  MAILEXCH_COPY_STRING(result, exch->connection_settings.um_url,  settings->um_url);
  MAILEXCH_COPY_STRING(result, exch->connection_settings.oab_url, settings->oab_url);
  return result;
}

int mailexch_autodiscover_connection_settings(mailexch* exch, const char* email_address,
    const char* host, const char* username, const char* password, const char* domain) {

  return mailexch_autodiscover(exch, email_address, host, username, password, domain, &exch->connection_settings);
}


int mailexch_connect(mailexch* exch, const char* username, const char* password, const char* domain) {

  /* We just do a GET on the given URL to test the connection.
     It should give us a response with code 200, and a WSDL in the body. */

  /* prepare curl: curl object + credentials */
  int result = mailexch_prepare_curl(exch, username, password, domain);
  if(result != MAILEXCH_NO_ERROR) return result;

  /* GET url */
  curl_easy_setopt(exch->curl, CURLOPT_HTTPGET, 1L);
  curl_easy_setopt(exch->curl, CURLOPT_URL, exch->connection_settings.as_url);

  /* Follow redirects, but only to HTTPS. */
  curl_easy_setopt(exch->curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(exch->curl, CURLOPT_MAXREDIRS, 10L);
  curl_easy_setopt(exch->curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
  curl_easy_setopt(exch->curl, CURLOPT_UNRESTRICTED_AUTH, 0L);

  /* result */
  uint8_t found_wsdl = 0;
  curl_easy_setopt(exch->curl, CURLOPT_WRITEFUNCTION, mailexch_test_connection_write_callback);
  curl_easy_setopt(exch->curl, CURLOPT_WRITEDATA, &found_wsdl);

  /* perform request */
  CURLcode curl_code = curl_easy_perform(exch->curl);
  if(curl_code != CURLE_OK) {
    result = MAILEXCH_ERROR_CONNECT;
  } else {
    long http_response = 0;
    curl_easy_getinfo (exch->curl, CURLINFO_RESPONSE_CODE, &http_response);
    if(http_response != 200) {
      result = MAILEXCH_ERROR_CONNECT;
    } else if(!found_wsdl) {
      result = MAILEXCH_ERROR_NO_EWS;
    } else {
      result = MAILEXCH_NO_ERROR;
    }
  }

  /* from now on we use POST to the given url */
  if(result == MAILEXCH_NO_ERROR) {
    curl_easy_setopt(exch->curl, CURLOPT_POST, 1L);
    curl_easy_setopt(exch->curl, CURLOPT_URL, exch->connection_settings.as_url);
  }

  /* clean up */
  curl_easy_setopt(exch->curl, CURLOPT_FOLLOWLOCATION, 0L);
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
  if(mailexch_write_response_to_buffer(exch, MAILEXCH_DEFAULT_RESPONSE_BUFFER_LENGTH) != MAILEXCH_NO_ERROR) {
    curl_slist_free_all(headers);
    free(request);
    return MAILEXCH_ERROR_INTERNAL;
  }

  /* perform request */
  curl_code = curl_easy_perform(exch->curl);
  if(curl_code != CURLE_OK) {
    result = MAILEXCH_ERROR_CANT_LIST;
  } else {
    curl_easy_getinfo (exch->curl, CURLINFO_RESPONSE_CODE, &http_response);
    if(http_response != 200) {
      result = MAILEXCH_ERROR_CANT_LIST;
    } else {
      puts(exch->response_buffer->str);
    }
  }

  /* cleanup */
  curl_slist_free_all(headers);
  free(request);
  return result;
}

/*
  mailexch structure callbacks
*/

const char* mailexch_strnstr(const char* str, const char* substr, size_t length) {
  size_t substr_length = strlen(substr);
  unsigned int i;
  for(i = 0; i <= length - substr_length; i++) {
    if(str[i] == 0) return NULL;
    if(memcmp(str + i, substr, substr_length) == 0)
      return str + i;
  }
  return NULL;
}

size_t mailexch_test_connection_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t response_length = size*nmemb < 1 ? 0 : size*nmemb;
  uint8_t* found_wsdl = ((uint8_t*)userdata);

  if(!*found_wsdl && mailexch_strnstr(ptr, "wsdl:definitions", response_length) == NULL)
    *found_wsdl = 1;

  return response_length;
}
