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
# include <config.h>
#endif

#include "helper.h"
#include "types_internal.h"

#include <stdlib.h>
#include <string.h>


oxws_result oxws_prepare_curl(oxws* oxws, const char* username, const char* password, const char* domain) {
  if(oxws == NULL || username == NULL || password == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;
  oxws_internal* internal = OXWS_INTERNAL(oxws);
  if(internal == NULL) return OXWS_ERROR_INTERNAL;

  /* do this only once */
  if(internal->curl != NULL)
    return OXWS_NO_ERROR;
  else if (oxws->state != OXWS_STATE_NEW &&
           oxws->state != OXWS_STATE_CONNECTION_SETTINGS_CONFIGURED)
    return OXWS_ERROR_BAD_STATE;

  CURL* curl = NULL;
  oxws_result result = oxws_prepare_curl_internal(internal, &curl, username, password, domain);
  if(result == OXWS_NO_ERROR && curl != NULL) {
    internal->curl = curl;
  }
  return result;
}

oxws_result oxws_prepare_curl_internal(oxws_internal* internal, CURL** curl, const char* username, const char* password, const char* domain) {
  if(curl == NULL || username == NULL || password == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;

  /* initialize */
  *curl = curl_easy_init();
  if(*curl == NULL) return OXWS_ERROR_INTERNAL;
#ifdef DEBUG_CURL
  curl_easy_setopt(*curl, CURLOPT_VERBOSE, 1L);
#endif

  /* set credentials */
  int result = oxws_set_credentials(*curl, username, password, domain);
  if(result != OXWS_NO_ERROR) {
    curl_easy_cleanup(*curl);
    *curl = NULL;
    return result;
  }

  /* invoke callback */
  if(internal != NULL && internal->curl_init_callback != NULL) {
    internal->curl_init_callback(*curl);
  }

  return OXWS_NO_ERROR;
}

oxws_result oxws_set_credentials(CURL* curl, const char* username, const char* password, const char* domain) {

  if(curl == NULL || username == NULL || password == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;

  /* set userpwd */
  size_t username_length = username ? strlen(username) : 0;
  size_t password_length = password ? strlen(password) : 0;
  size_t domain_length = domain ? strlen(domain) : 0;
  char* userpwd = NULL;

  if(domain_length > 0) {
    userpwd = (char*) malloc(domain_length + 1 + username_length + 1 + password_length + 1);
    if(!userpwd) return OXWS_ERROR_INTERNAL;
    sprintf(userpwd, "%s\\%s:%s", domain, username, password);
  } else {
    userpwd = (char*) malloc(username_length + 1 + password_length + 1);
    if(!userpwd) return OXWS_ERROR_INTERNAL;
    sprintf(userpwd, "%s:%s", username, password);
  }

  curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
  free(userpwd);

  /* allow any authentication protocol */
  curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);

  return OXWS_NO_ERROR;
}


oxws_result oxws_write_response_to_buffer(oxws* oxws, size_t buffer_size_hint) {
  if(oxws == NULL) return OXWS_ERROR_INVALID_PARAMETER;

  oxws_internal* internal = OXWS_INTERNAL(oxws);
  if(internal == NULL) return OXWS_ERROR_INTERNAL;

  curl_easy_setopt(internal->curl, CURLOPT_WRITEFUNCTION, oxws_write_response_to_buffer_callback);
  curl_easy_setopt(internal->curl, CURLOPT_WRITEDATA, internal);

  /* (re)allocate and clear response buffer */
  if(internal->response_buffer) {
    mmap_string_set_size(internal->response_buffer, buffer_size_hint);
    mmap_string_truncate(internal->response_buffer, 0);
  } else {
    internal->response_buffer = mmap_string_sized_new(buffer_size_hint);
  }

  return OXWS_NO_ERROR;
}

size_t oxws_write_response_to_buffer_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  oxws_internal* internal = (oxws_internal*) userdata;

  if(internal != NULL && internal->response_buffer) {
    size_t length = size * nmemb < 1 ? 0 : size * nmemb;
    mmap_string_append_len(internal->response_buffer, ptr, length);
    return length;
  } else {
    return 0; /* error */
  }
}
