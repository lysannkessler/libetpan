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


int mailexch_prepare_curl(mailexch* exch, const char* username,
        const char* password, const char* domain) {

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

int mailexch_set_credentials(mailexch* exch, const char* username,
        const char* password, const char* domain) {

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

  curl_easy_setopt(exch->curl, CURLOPT_USERPWD, userpwd);
  free(userpwd);

  /* allow any authentication protocol */
  curl_easy_setopt(exch->curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);

  return MAILEXCH_NO_ERROR;
}


int mailexch_write_response_to_buffer(mailexch* exch, size_t buffer_size_hint) {
  curl_easy_setopt(exch->curl, CURLOPT_WRITEFUNCTION,
                   mailexch_default_write_callback);
  curl_easy_setopt(exch->curl, CURLOPT_WRITEDATA, exch);
  mmap_string_set_size(exch->response_buffer, buffer_size_hint);
  mmap_string_truncate(exch->response_buffer, 0);
  return MAILEXCH_NO_ERROR;
}

size_t mailexch_default_write_callback(char *ptr, size_t size, size_t nmemb,
        void *userdata) {

  size_t length = size*nmemb < 1 ? 0 : size*nmemb;
  mailexch* exch = (mailexch*) userdata;

  mmap_string_append_len(exch->response_buffer, ptr, length);
  return length;
}