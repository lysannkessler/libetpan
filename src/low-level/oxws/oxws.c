/*
 * libEtPan! -- a mail stuff library
 *
 * Copyright (C) 2012 Lysann Kessler
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

#include <libetpan/oxws.h>
#include <libetpan/oxws_autodiscover.h>
#include "helper.h"
#include "types_internal.h"

#include <stdlib.h>
#include <string.h>


size_t oxws_test_connection_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);


/*
  oxws structure
*/

oxws* oxws_new(size_t progr_rate, progress_function* progr_fun) {
  oxws* oxws = calloc(1, sizeof(* oxws));
  if (oxws == NULL)
    return NULL;

  oxws->progr_rate = progr_rate;
  oxws->progr_fun = progr_fun;

  oxws->state = OXWS_STATE_NEW;

  oxws->internal = oxws_internal_new();
  if(oxws->internal == NULL) {
    free(oxws);
    oxws = NULL;
  }

  return oxws;
}

void oxws_free(oxws* oxws) {
  if(!oxws) return;

  if(oxws->connection_settings.as_url)
    free(oxws->connection_settings.as_url);
  if(oxws->connection_settings.oof_url)
    free(oxws->connection_settings.oof_url);
  if(oxws->connection_settings.um_url)
    free(oxws->connection_settings.um_url);
  if(oxws->connection_settings.oab_url)
    free(oxws->connection_settings.oab_url);

  oxws_internal_free(OXWS_INTERNAL(oxws));

  free(oxws);
}

/*
  OXWS_COPY_STRING()

  Copy a string from source to dest, using result to indicate success or
  failure. Will (re)allocate dest to fit the source string, and will free dest
  and set it to NULL upon failure or if source is NULL.
  the function will only attempt to copy if result is OXWS_NO_ERROR in the
  beginning.

  @param result   an int set to either OXWS_NO_ERROR or OXWS_ERROR_*.
                  Copying is attempted only attempted if it's OXWS_NO_ERROR.
                  It will be set to OXWS_ERROR_INTERNAL if memory
                  reallocation of dest fails.
                  If it is not OXWS_NO_ERROR in the end, dest is freed and
                  set to NULL.
  @param dest     copy destination; can be anything that can be passed to
                  realloc() and (if not NULL) free()
  @param source   Copy source; NULL indicates to free and clear the destination

*/
#define OXWS_COPY_STRING(result, dest, source) \
  if(result == OXWS_NO_ERROR && source) { \
    (dest) = realloc((dest), strlen(source) + 1); \
    if(!(dest)) { \
      result = OXWS_ERROR_INTERNAL; \
    } else { \
      memcpy((dest), (source), strlen(source) + 1); \
    } \
  } \
  if(result != OXWS_NO_ERROR || source == NULL) { \
    if(dest) free(dest); \
    (dest) = NULL; \
  }

oxws_result oxws_set_connection_settings(oxws* oxws, oxws_connection_settings* settings) {
  if(oxws == NULL || settings == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;
  if(oxws->state != OXWS_STATE_NEW)
    return OXWS_ERROR_BAD_STATE;

  int result = OXWS_NO_ERROR;
  OXWS_COPY_STRING(result, oxws->connection_settings.as_url, settings->as_url);
  OXWS_COPY_STRING(result, oxws->connection_settings.oof_url, settings->oof_url);
  OXWS_COPY_STRING(result, oxws->connection_settings.um_url, settings->um_url);
  OXWS_COPY_STRING(result, oxws->connection_settings.oab_url, settings->oab_url);

  if(result == OXWS_NO_ERROR)
    oxws->state = OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED;
  return result;
}

oxws_result oxws_autodiscover_connection_settings(oxws* oxws,
        const char* host, const char* email_address, const char* username,
        const char* password, const char* domain) {

  if(oxws == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  if(oxws->state != OXWS_STATE_NEW) return OXWS_ERROR_BAD_STATE;

  int result = oxws_autodiscover(host, email_address, username, password, domain, &oxws->connection_settings);

  if(result == OXWS_NO_ERROR)
    oxws->state = OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED;
  return result;
}


oxws_result oxws_connect(oxws* oxws, const char* username, const char* password, const char* domain) {

  if(oxws == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  oxws_internal* internal = OXWS_INTERNAL(oxws);
  if(internal == NULL) return OXWS_ERROR_INTERNAL;
  if(oxws->state != OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED)
    return OXWS_ERROR_BAD_STATE;

  /* We just do a GET on the given URL to test the connection.
     It should give us a response with code 200, and a WSDL in the body. */

  /* prepare curl: curl object + credentials */
  int result = oxws_prepare_curl(oxws, username, password, domain);
  if(result != OXWS_NO_ERROR) return result;
  CURL* curl = internal->curl;

  /* GET url */
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
  curl_easy_setopt(curl, CURLOPT_URL, oxws->connection_settings.as_url);

  /* Follow redirects, but only to HTTPS. */
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
  curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
  curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 0L); /* paranoia */

  /* result */
  uint8_t found_wsdl = 0;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, oxws_test_connection_write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &found_wsdl);

  /* perform request */
  CURLcode curl_code = curl_easy_perform(curl);
  if(curl_code != CURLE_OK) {
    result = OXWS_ERROR_CONNECT;
  } else {
    long http_response = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_response);
    if(http_response != 200) {
      result = OXWS_ERROR_CONNECT;
    } else if(!found_wsdl) {
      result = OXWS_ERROR_NO_EWS;
    } else {
      result = OXWS_NO_ERROR;
      oxws->state = OXWS_STATE_CONNECTED;
    }
  }

  /* clean up */
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
  return result;
}


/*
  oxws structure callbacks
*/

/*
  oxws_strnstr()

  Find first occurrence of substring in a string's first few bytes.

  @param str      [required] string to search for substr
  @param substr   [required] string to find in str
  @param length   number of bytes to search in str

  @return NULL if a required parameter is missing.
          NULL if substr does not occur in the first 'length' bytes of str;
          the beginning of the first occurrence of substr in str otherwise.

  @note The search will not stop on a 0-byte, with i.e. oxws_strnstr() you
        can search in strings that are note zero-terminated.
*/
const char* oxws_strnstr(const char* str, const char* substr, size_t length) {

  if(str == NULL || substr == NULL) return NULL;

  size_t substr_length = strlen(substr);
  unsigned int i;
  for(i = 0; i <= length - substr_length; i++) {
    if(memcmp(str + i, substr, substr_length) == 0)
      return str + i;
  }
  return NULL;
}

/*
  oxws_test_connection_write_callback()

  A CURL write callback, set via the CURLOPT_WRITEFUNCTION option, that searches
  for WSDL definitions in the HTTP response.

  @param userdata [required] Must be a pointer to a uint8_t that will receive
                  the search result. You must set the CURLOPT_WRITEDATA option
                  to such a pointer so that this callback is called with it as
                  parameter.

  @see CURL documentation for more information.
*/
size_t oxws_test_connection_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {

  size_t response_length = size*nmemb < 1 ? 0 : size*nmemb;
  uint8_t* found_wsdl = ((uint8_t*)userdata);

  if(!*found_wsdl &&
     oxws_strnstr(ptr, "wsdl:definitions", response_length) != NULL) {

    *found_wsdl = 1;
  }

  return response_length;
}
