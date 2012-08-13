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
# include <config.h>
#endif

#include <curl/curl.h>

#include <stdlib.h>
#include <string.h>

#include "helper.h"
#include "types_internal.h"
#include <libetpan/mmapstring.h>


#define OXWS_AUTODISCOVER_REQUEST_FORMAT ( \
  "<Autodiscover xmlns=\"http://schemas.microsoft.com/exchange/autodiscover/outlook/requestschema/2006\">\n" \
  "  <Request>\n" \
  "    <EMailAddress>%s</EMailAddress>\n" \
  "    <AcceptableResponseSchema>http://schemas.microsoft.com/exchange/autodiscover/outlook/responseschema/2006a</AcceptableResponseSchema>\n" \
  "  </Request>\n" \
  "</Autodiscover>")

#define OXWS_AUTODISCOVER_MIN_RESPONSE_BUFFER_LENGTH 3000

#define OXWS_AUTODISCOVER_STEP1_URL_FORMAT "https://%s/autodiscover/autodiscover.xml"
#define OXWS_AUTODISCOVER_STEP2_URL_FORMAT "https://autodiscover.%s/autodiscover/autodiscover.xml"
#define OXWS_AUTODISCOVER_LONGEST_URL_FORMAT OXWS_AUTODISCOVER_STEP2_URL_FORMAT
#define OXWS_AUTODISCOVER_URL_LENGTH strlen(OXWS_AUTODISCOVER_LONGEST_URL_FORMAT - 2)


#define OXWS_AUTODISCOVER_TRY_STEP_LONG(step, curl, response_buffer, url_buffer, settings, host, result) \
  do { \
    sprintf(url_buffer, OXWS_AUTODISCOVER_STEP##step##_URL_FORMAT, host); \
    result = oxws_autodiscover_try_url(curl, response_buffer, url_buffer, settings); \
    mmap_string_truncate(response_buffer, 0); \
  } while(0);

#define OXWS_AUTODISCOVER_TRY_STEP(step) \
    OXWS_AUTODISCOVER_TRY_STEP_LONG(step, curl, response_buffer, url, settings, host, result)


/*
  oxws_autodiscover_try_url()

  Try to extract autodiscover information from given URL, and save them in the
  given settings structure.

  @param curl            [required] CURL object to use for HTTP requests.
  @param response_buffer [required] the response buffer that will receive the
                         HTTP response body.
  @param url             [required] URL to try
  @param settings        [required] Upon success, the connection settings are
                         stored in the structure ponited at by this parameter.

  @return - OXWS_NO_ERROR indicated success
          - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing.
          - OXWS_ERROR_CONNECT: cannot connect to given URL
          - OXWS_ERROR_AUTODISCOVER_UNAVAILABLE: given URL does not seem to
            point to a Exchange autodiscover service
          - OXWS_ERROR_INTERNAL: arbitrary failure
*/
oxws_result oxws_autodiscover_try_url(CURL* curl, MMAPString* response_buffer, const char* url, oxws_connection_settings* settings);

size_t oxws_autodiscover_curl_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);


oxws_result oxws_autodiscover(const char* host, const char* email_address,
        const char* username, const char* password, const char* domain,
        oxws_connection_settings* settings) {
  /* http://msdn.microsoft.com/en-us/library/exchange/ee332364(v=exchg.140).aspx */

  if(email_address == NULL || username == NULL || password == NULL || settings == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;

  /* get host name */
  if(host == NULL) {
    host = strstr(email_address, "@");
    if(host == NULL)
      return OXWS_ERROR_INVALID_PARAMETER;
    host += 1;
    if(*host == 0) {
      /* end of string after @, i.e. empty host name */
      return OXWS_ERROR_INVALID_PARAMETER;
    }
  }

  /* prepare curl: curl object + credentials */
  CURL* curl = NULL;
  int result = oxws_prepare_curl_internal(&curl, username, password, domain);
  if(result != OXWS_NO_ERROR) return result;

  /* headers */
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: text/xml");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  /* Follow redirects, but only to HTTPS. */
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
  curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
  curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 0L);

  /* content */
  char* request = (char*) malloc(
    strlen(OXWS_AUTODISCOVER_REQUEST_FORMAT) - 2 + /* remove format chars */
    strlen(email_address) + 1);      /* add email address and null terminator */
  if(!request) {
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return OXWS_ERROR_INTERNAL;
  }
  sprintf(request, OXWS_AUTODISCOVER_REQUEST_FORMAT, email_address);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request);

  /* buffer response. TODO use SAX interface */
  MMAPString* response_buffer = mmap_string_sized_new(OXWS_AUTODISCOVER_MIN_RESPONSE_BUFFER_LENGTH);
  if(response_buffer == NULL) {
    free(request);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return OXWS_ERROR_INTERNAL;
  }
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, oxws_autodiscover_curl_write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_buffer);

  /* try steps: */
  /*   allocate buffer */
  char* url = malloc(OXWS_AUTODISCOVER_URL_LENGTH + strlen(host) + 1);
  if(!url) {
    mmap_string_free(response_buffer);
    free(request);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return OXWS_ERROR_INTERNAL;
  }
  /*   try */
  OXWS_AUTODISCOVER_TRY_STEP(1);
  if(result != OXWS_NO_ERROR)
    OXWS_AUTODISCOVER_TRY_STEP(2);
  /*   set result */
  if(result != OXWS_NO_ERROR)
    result = OXWS_ERROR_AUTODISCOVER_UNAVAILABLE;

  /* clean up */
  free(url);
  mmap_string_free(response_buffer);
  free(request);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return result;
}

oxws_result oxws_autodiscover_try_url(CURL* curl, MMAPString* response_buffer, const char* url, oxws_connection_settings* settings) {

  if(curl == NULL || response_buffer == NULL || url == NULL || settings == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;

  curl_easy_setopt(curl, CURLOPT_URL, url);
  CURLcode curl_code = curl_easy_perform(curl);

  int result = OXWS_ERROR_CONNECT;
  if(curl_code == CURLE_OK) {
    long http_response = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_response);
    if(http_response == 200) {
      result = OXWS_ERROR_AUTODISCOVER_UNAVAILABLE;

      /* parse ASUrl */
      char* as_url = strstr(response_buffer->str, "<ASUrl>");
      if(as_url != NULL) {
        as_url += 7;
        char* as_url_end = strstr(as_url, "</ASUrl>");
        if(as_url_end != NULL) {

          /* copy ASUrl */
          size_t as_url_length = as_url_end - as_url;
          settings->as_url = (char*) malloc(as_url_length + 1);
          memcpy(settings->as_url, as_url, as_url_length);
          settings->as_url[as_url_length] = 0;

          /* TODO copy other settings */

          result = OXWS_NO_ERROR;
        }
      }
    }
  }

  return result;
}

size_t oxws_autodiscover_curl_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  MMAPString* buffer = (MMAPString*) userdata;
  if(buffer != NULL) {
    size_t length = size * nmemb < 1 ? 0 : size * nmemb;
    mmap_string_append_len(buffer, ptr, length);
    return length;
  } else {
    return 0; /* error */
  }
}

