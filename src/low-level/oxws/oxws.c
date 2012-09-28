/*
 * libEtPan! -- a mail stuff library
 *   Microsoft Exchange Web Services support
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
#include <libetpan/oxws_requests.h>
#include "helper.h"
#include "types_internal.h"

#include <stdlib.h>
#include <string.h>


const char* oxws_result_name_map[] = {
  "NO_ERROR",
  "ERROR_INVALID_PARAMETER",
  "ERROR_BAD_STATE",
  "ERROR_INTERNAL",
  "ERROR_CONNECT",
  "ERROR_NO_EWS",
  "ERROR_AUTH_FAILED",
  "ERROR_AUTODISCOVER_UNAVAILABLE",
  "ERROR_AUTODISCOVER_EMAIL_DOES_NOT_EXIST",
  "ERROR_REQUEST_FAILED",
  "ERROR_INVALID_RESPONSE",
};
const unsigned short oxws_result_name_map_length =
  sizeof(oxws_result_name_map) / sizeof(const char*);


size_t oxws_test_connection_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);


/*
  oxws structure
*/

oxws* oxws_new() {
  oxws* oxws = calloc(1, sizeof(* oxws));
  if (oxws == NULL)
    return NULL;

  oxws->state = OXWS_STATE_NEW;

  oxws->internal = oxws_internal_new();
  if(oxws->internal == NULL) {
    free(oxws);
    oxws = NULL;
  }

  return oxws;
}

void oxws_free(oxws* oxws) {
  if(oxws == NULL) return;

  free(oxws->connection_settings.as_url);
  free(oxws->connection_settings.oof_url);
  free(oxws->connection_settings.um_url);
  free(oxws->connection_settings.oab_url);

  oxws_internal_free(OXWS_INTERNAL(oxws));

  free(oxws);
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

  int result = oxws_autodiscover(oxws, host, email_address, username, password, domain, &oxws->connection_settings);

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


oxws_result oxws_set_progress_callback(oxws* oxws, mailprogress_function* callback, void* userdata, size_t rate) {
  if(oxws == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  oxws->progress_callback.callback = callback;
  oxws->progress_callback.userdata = userdata;
  oxws->progress_callback.rate = rate;
  return OXWS_NO_ERROR;
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
