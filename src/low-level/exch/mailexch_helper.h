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

#ifndef MAILEXCH_HELPER_H
#define MAILEXCH_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif


#include <curl/curl.h>

#include <libetpan/mailexch_types.h>


int mailexch_prepare_curl(mailexch* exch, const char* username, const char* password, const char* domain);

int mailexch_set_credentials(mailexch* exch, const char* username, const char* password, const char* domain);


int mailexch_allocate_response_buffer(mailexch* exch, size_t min_size);

int mailexch_allocate_more_in_response_buffer(mailexch* exch, size_t size_to_add);

int mailexch_append_to_response_buffer(mailexch* exch, char* data, size_t length);

int mailexch_free_response_buffer(mailexch* exch);


int mailexch_write_response_to_buffer(mailexch* exch, size_t buffer_size_hint);

size_t mailexch_default_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);


#ifdef __cplusplus
}
#endif

#endif
