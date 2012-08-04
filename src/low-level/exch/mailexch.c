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
#include <libetpan/mailexch_autodiscover.h>

#include <stdlib.h>
#include <string.h>


size_t mailexch_test_connection_write_callback(char *ptr, size_t size,
        size_t nmemb, void *userdata);


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

  exch->response_buffer =
    mmap_string_sized_new(MAILEXCH_DEFAULT_RESPONSE_BUFFER_LENGTH);

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
  if(exch->curl_headers) {
    curl_slist_free_all(exch->curl_headers);
    exch->curl_headers = NULL;
  }

  mmap_string_free(exch->response_buffer);
  exch->response_buffer = NULL;
}

/*
  MAILEXCH_COPY_STRING()

  Copy a string from source to dest, using result to indicate success or
  failure. Will (re)allocate dest to fit the source string, and will free dest
  and set it to NULL upon failure or if source is NULL.
  the function will only attempt to copy if result is MAILEXCH_NO_ERROR in the
  beginning.

  @param result   an int set to either MAILEXCH_NO_ERROR or MAILEXCH_ERROR_*.
                  Copying is attempted only attempted if it's MAILEXCH_NO_ERROR.
                  It will be set to MAILEXCH_ERROR_INTERNAL if memory
                  reallocation of dest fails.
                  If it is not MAILEXCH_NO_ERROR in the end, dest is freed and
                  set to NULL.
  @param dest     copy destination; can be anything that can be passed to
                  realloc() and (if not NULL) free()
  @param source   Copy source; NULL indicates to free and clear the destination

*/
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

int mailexch_set_connection_settings(mailexch* exch,
        mailexch_connection_settings* settings) {

  int result = MAILEXCH_NO_ERROR;
  MAILEXCH_COPY_STRING(result, exch->connection_settings.as_url,
                       settings->as_url);
  MAILEXCH_COPY_STRING(result, exch->connection_settings.oof_url,
                       settings->oof_url);
  MAILEXCH_COPY_STRING(result, exch->connection_settings.um_url,
                       settings->um_url);
  MAILEXCH_COPY_STRING(result, exch->connection_settings.oab_url,
                       settings->oab_url);
  return result;
}

int mailexch_autodiscover_connection_settings(mailexch* exch, const char* host,
        const char* email_address, const char* username,
        const char* password, const char* domain) {

  return mailexch_autodiscover(exch, host, email_address, username, password,
                               domain, &exch->connection_settings);
}


int mailexch_connect(mailexch* exch, const char* username, const char* password,
        const char* domain) {

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
  curl_easy_setopt(exch->curl, CURLOPT_UNRESTRICTED_AUTH, 0L); /* paranoia */

  /* result */
  uint8_t found_wsdl = 0;
  curl_easy_setopt(exch->curl, CURLOPT_WRITEFUNCTION,
                   mailexch_test_connection_write_callback);
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

  if(result == MAILEXCH_NO_ERROR)
    mailexch_prepare_for_requests(exch); /* TODO: do this at beginning of request dependant on current state; catch errors */

  /* clean up */
  curl_easy_setopt(exch->curl, CURLOPT_FOLLOWLOCATION, 0L);
  return result;
}


/*
  mailexch structure callbacks
*/

/*
  mailexch_strnstr()

  Find first occurrence of substring in a string's first few bytes.

  @param str      string to search for substr
  @param substr   string to find in str
  @param length   number of bytes to search in str

  @return NULL if substr does not occur in the first 'length' bytes of str;
          the beginning of the first occurrence of substr in str otherwise.

  @note The search will not stop on a 0-byte, with i.e. mailexch_strnstr() you
        can search in strings that are note zero-terminated.
*/
const char* mailexch_strnstr(const char* str, const char* substr,
        size_t length) {

  size_t substr_length = strlen(substr);
  unsigned int i;
  for(i = 0; i <= length - substr_length; i++) {
    if(memcmp(str + i, substr, substr_length) == 0)
      return str + i;
  }
  return NULL;
}

/*
  mailexch_test_connection_write_callback()

  A CURL write callback, set via the CURLOPT_WRITEFUNCTION option, that searches
  for WSDL definitions in the HTTP response.

  @param userdata   Must be a pointer to a uint8_t that will receive the search
                    result. You must set the CURLOPT_WRITEDATA option to such a
                    pointer so that this callback is called with it as
                    parameter.
*/
size_t mailexch_test_connection_write_callback(char *ptr, size_t size,
        size_t nmemb, void *userdata) {

  size_t response_length = size*nmemb < 1 ? 0 : size*nmemb;
  uint8_t* found_wsdl = ((uint8_t*)userdata);

  if(!*found_wsdl &&
     mailexch_strnstr(ptr, "wsdl:definitions", response_length) != NULL) {

    *found_wsdl = 1;
  }

  return response_length;
}
