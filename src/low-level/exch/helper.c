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
# include <config.h>
#endif

#include "helper.h"
#include "types_internal.h"

#include <stdlib.h>
#include <string.h>


mailexch_result mailexch_prepare_curl(mailexch* exch, const char* username,
        const char* password, const char* domain) {

  /* do this only once */
  if(MAILEXCH_INTERNAL(exch)->curl != NULL)
    return MAILEXCH_NO_ERROR;
  else if (exch->state != MAILEXCH_STATE_NEW &&
           exch->state != MAILEXCH_STATE_CONNECTION_SETTINGS_CONFIGURED)
    return MAILEXCH_ERROR_BAD_STATE;

  CURL* curl = curl_easy_init();
  if(!curl) return MAILEXCH_ERROR_INTERNAL;
  MAILEXCH_INTERNAL(exch)->curl = curl;
#if 0
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

  int result = mailexch_set_credentials(exch, username, password, domain);
  if(result != MAILEXCH_NO_ERROR) {
    curl_easy_cleanup(curl);
    MAILEXCH_INTERNAL(exch)->curl = NULL;
  }

  return result;
}

mailexch_result mailexch_set_credentials(mailexch* exch, const char* username,
        const char* password, const char* domain) {

  if(exch->state != MAILEXCH_STATE_NEW &&
     exch->state != MAILEXCH_STATE_CONNECTION_SETTINGS_CONFIGURED)
    return MAILEXCH_ERROR_BAD_STATE;

  /* set userpwd */
  size_t username_length = username ? strlen(username) : 0;
  size_t password_length = password ? strlen(password) : 0;
  size_t domain_length = domain ? strlen(domain) : 0;
  char* userpwd = NULL;

  if(domain_length > 0) {
    userpwd = (char*) malloc(domain_length + 1 + username_length + 1 +
                             password_length + 1);
    if(!userpwd) return MAILEXCH_ERROR_INTERNAL;
    sprintf(userpwd, "%s\\%s:%s", domain, username, password);
  } else {
    userpwd = (char*) malloc(username_length + 1 + password_length + 1);
    if(!userpwd) return MAILEXCH_ERROR_INTERNAL;
    sprintf(userpwd, "%s:%s", username, password);
  }

  curl_easy_setopt(MAILEXCH_INTERNAL(exch)->curl, CURLOPT_USERPWD, userpwd);
  free(userpwd);

  /* allow any authentication protocol */
  curl_easy_setopt(MAILEXCH_INTERNAL(exch)->curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);

  return MAILEXCH_NO_ERROR;
}


mailexch_result mailexch_write_response_to_buffer(mailexch* exch,
        size_t buffer_size_hint) {

  mailexch_internal* internal = MAILEXCH_INTERNAL(exch);

  curl_easy_setopt(internal->curl, CURLOPT_WRITEFUNCTION,
                   mailexch_write_response_to_buffer_callback);
  curl_easy_setopt(internal->curl, CURLOPT_WRITEDATA, internal);

  /* (re)allocate and clear response buffer */
  if(internal->response_buffer) {
    mmap_string_set_size(internal->response_buffer, buffer_size_hint);
    mmap_string_truncate(internal->response_buffer, 0);
  } else {
    internal->response_buffer = mmap_string_sized_new(buffer_size_hint);
  }

  return MAILEXCH_NO_ERROR;
}

size_t mailexch_write_response_to_buffer_callback(char *ptr, size_t size,
        size_t nmemb, void *userdata) {

  size_t length = size*nmemb < 1 ? 0 : size*nmemb;
  mailexch_internal* internal = (mailexch_internal*) userdata;

  if(internal->response_buffer) {
    mmap_string_append_len(internal->response_buffer, ptr, length);
    return length;
  } else {
    return 0; /* error */
  }
}
