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

#include <libetpan/mailexch_requests.h>
#include "types_internal.h"


/* maps from mailexch_distinguished_folder_id to string */
const char* mailexch_distfolder_id_name_map[] = {
  "calendar", "contacts", "deleteditems", "drafts", "inbox", "journal", "notes",
  "outbox", "sentitems", "tasks", "msgfolderroot", "root", "junkemail",
  "searchfolders", "voicemail"
};
const short mailexch_distfolder_id_name_map_length =
  sizeof(mailexch_distfolder_id_name_map) / sizeof(const char*);


mailexch_result mailexch_prepare_for_requests(mailexch* exch) {
  if(exch == NULL) return MAILEXCH_ERROR_INVALID_PARAMETER;
  mailexch_internal* internal = MAILEXCH_INTERNAL(exch);
  if(internal == NULL) return MAILEXCH_ERROR_INTERNAL;
  if(exch->state == MAILEXCH_STATE_READY_FOR_REQUESTS)
    return MAILEXCH_NO_ERROR;

  /* paranoia */
  curl_easy_setopt(internal->curl, CURLOPT_FOLLOWLOCATION, 0L);
  curl_easy_setopt(internal->curl, CURLOPT_UNRESTRICTED_AUTH, 0L);

  /* post to AsUrl */
  curl_easy_setopt(internal->curl, CURLOPT_POST, 1L);
  curl_easy_setopt(internal->curl, CURLOPT_URL,
          exch->connection_settings.as_url);

  /* Clear headers and set Content-Type to text/xml. */
  if(internal->curl_headers) {
    curl_slist_free_all(internal->curl_headers);
    internal->curl_headers = NULL;
  }
  internal->curl_headers = curl_slist_append(internal->curl_headers,
          "Content-Type: text/xml");
  curl_easy_setopt(internal->curl, CURLOPT_HTTPHEADER, internal->curl_headers);

  /* clear request string for now */
  curl_easy_setopt(internal->curl, CURLOPT_POSTFIELDS, NULL);

  /* update state */
  exch->state = MAILEXCH_STATE_READY_FOR_REQUESTS;
  return MAILEXCH_NO_ERROR;
}
