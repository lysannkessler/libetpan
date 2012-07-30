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

#include <libetpan/mailexch_helper.h>

#include <stdlib.h>
#include <string.h>


int mailexch_prepare_curl(mailexch* exch, const char* username, const char* password, const char* domain) {
  exch->curl = curl_easy_init();
  if(!exch->curl) return MAILEXCH_ERROR_INTERNAL;
#if 0
  curl_easy_setopt(exch->curl, CURLOPT_VERBOSE, 1L);
#endif

  int result = mailexch_set_credentials(exch, username, password, domain);
  if(result != MAILEXCH_NO_ERROR) {
    curl_easy_cleanup(exch->curl);
    exch->curl = NULL;
  }
  return result;
}

int mailexch_set_credentials(mailexch* exch, const char* username, const char* password, const char* domain) {
  /* set userpwd */
  size_t username_length = username ? strlen(username) : 0;
  size_t password_length = password ? strlen(password) : 0;
  size_t domain_length = domain ? strlen(domain) : 0;
  char* userpwd = NULL;

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

  /* allow any authentication protocol */
  curl_easy_setopt(exch->curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);

  return MAILEXCH_NO_ERROR;
}

int mailexch_allocate_response_buffer(mailexch* exch, size_t min_size) {
  if(min_size == 0) {
    /* free */
    if(exch->response_buffer) free(exch->response_buffer);
    exch->response_buffer = NULL;
    exch->response_buffer_length = 0;
    exch->response_buffer_length_used = 0;
    return MAILEXCH_NO_ERROR;

  } else if(exch->response_buffer == NULL || exch->response_buffer_length < min_size) {
    /* allocate new or reallocate */
    void* new_buffer = realloc(exch->response_buffer, min_size);
    if(new_buffer) {
      /* success */
      exch->response_buffer = new_buffer;
      exch->response_buffer_length = min_size;
      if(exch->response_buffer_length < exch->response_buffer_length_used)
        exch->response_buffer_length_used = exch->response_buffer_length;
      return MAILEXCH_NO_ERROR;
    } else {
      /* failure */
      mailexch_free_response_buffer(exch);
      return MAILEXCH_ERROR_INTERNAL;
    }

  } else {
    /* keep existing buffer */
    return MAILEXCH_NO_ERROR;
  }
}

int mailexch_allocate_more_in_response_buffer(mailexch* exch, size_t size_to_add) {
  size_t new_length = exch->response_buffer_length_used;
  if(new_length == 0) new_length += 1; /* add space for \0 */
  new_length += size_to_add;
  return mailexch_allocate_response_buffer(exch, new_length);
}

int mailexch_append_to_response_buffer(mailexch* exch, char* data, size_t length) {
  /* allocate memory */
  int result = mailexch_allocate_more_in_response_buffer(exch, length);
  if(result != MAILEXCH_NO_ERROR) return result;

  /* remove zero-termination */
  char* buffer_end = exch->response_buffer;
  if(exch->response_buffer_length_used > 0)
    buffer_end += exch->response_buffer_length_used - 1;

  /* copy data and update length */
  memcpy(buffer_end, data, length);
  exch->response_buffer_length_used += length;

  /* add zero-termination */
  buffer_end += length + 1;
  *buffer_end = 0;

  return MAILEXCH_NO_ERROR;
}

int mailexch_free_response_buffer(mailexch* exch) {
  return mailexch_allocate_response_buffer(exch, 0);
}

int mailexch_write_response_to_buffer(mailexch* exch, size_t buffer_size_hint) {
  curl_easy_setopt(exch->curl, CURLOPT_WRITEFUNCTION, mailexch_default_write_callback);
  curl_easy_setopt(exch->curl, CURLOPT_WRITEDATA, exch);
  int result = mailexch_allocate_response_buffer(exch, buffer_size_hint);
  if(result == MAILEXCH_NO_ERROR)
    exch->response_buffer_length_used = 0;
  return result;
}

size_t mailexch_default_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t length = size*nmemb < 1 ? 0 : size*nmemb;
  mailexch* exch = (mailexch*) userdata;

  if(mailexch_append_to_response_buffer(exch, ptr, length) == MAILEXCH_NO_ERROR)
    return length;
  else
    return 0;
}
