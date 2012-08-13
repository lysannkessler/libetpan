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

#include "types_internal.h"

#include <stdlib.h>
#include <string.h>

mailexch_internal* mailexch_internal_new() {
  mailexch_internal* internal = calloc(1, sizeof(mailexch_internal));
  if (internal == NULL)
    return NULL;

  return internal;
}

void mailexch_internal_free(mailexch_internal* internal) {
  if(!internal) return;

  if(internal->curl)
    curl_easy_cleanup(internal->curl);
  if(internal->curl_headers)
    curl_slist_free_all(internal->curl_headers);

  if(internal->response_buffer)
    mmap_string_free(internal->response_buffer);
  if(internal->response_xml_parser)
    xmlFreeParserCtxt(internal->response_xml_parser);

  free(internal);
}


void mailexch_internal_response_buffer_free(mailexch_internal* internal) {
  if(internal != NULL && internal->response_buffer != NULL) {
    mmap_string_free(internal->response_buffer);
    internal->response_buffer = NULL;
  }
}
