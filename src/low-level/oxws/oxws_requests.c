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

#include <libetpan/oxws_requests.h>
#include "types_internal.h"
#include "helper.h"


/* maps from oxws_distinguished_folder_id to string */
const char* oxws_distfolder_id_name_map[] = {
  "calendar", "contacts", "deleteditems", "drafts", "inbox", "journal", "notes",
  "outbox", "sentitems", "tasks", "msgfolderroot", "root", "junkemail",
  "searchfolders", "voicemail"
};
const unsigned short oxws_distfolder_id_name_map_length =
  sizeof(oxws_distfolder_id_name_map) / sizeof(const char*);


int oxws_curl_progress_callback(void* userdata, double dltotal, double dlnow, double ultotal, double ulnow);


oxws_result oxws_prepare_for_requests(oxws* oxws) {
  if(oxws == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  oxws_internal* internal = OXWS_INTERNAL(oxws);
  if(internal == NULL) return OXWS_ERROR_INTERNAL;

  if(oxws->state != OXWS_STATE_READY_FOR_REQUESTS && oxws->state != OXWS_STATE_CONNECTED)
    return OXWS_ERROR_BAD_STATE;

  if(oxws->state != OXWS_STATE_READY_FOR_REQUESTS) {
    /* paranoia */
    curl_easy_setopt(internal->curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(internal->curl, CURLOPT_UNRESTRICTED_AUTH, 0L);

    /* post to AsUrl */
    curl_easy_setopt(internal->curl, CURLOPT_POST, 1L);
    curl_easy_setopt(internal->curl, CURLOPT_URL, oxws->connection_settings.as_url);

    /* Clear headers and set Content-Type to text/xml. */
    if(internal->curl_headers) {
      curl_slist_free_all(internal->curl_headers);
      internal->curl_headers = NULL;
    }
    internal->curl_headers = curl_slist_append(internal->curl_headers, "Content-Type: text/xml");
    curl_easy_setopt(internal->curl, CURLOPT_HTTPHEADER, internal->curl_headers);

    /* set progress callback */
    curl_easy_setopt(internal->curl, CURLOPT_PROGRESSFUNCTION, oxws_curl_progress_callback);
    curl_easy_setopt(internal->curl, CURLOPT_PROGRESSDATA, oxws);
    curl_easy_setopt(internal->curl, CURLOPT_NOPROGRESS, 0L);

    /* clear request string for now */
    curl_easy_setopt(internal->curl, CURLOPT_POSTFIELDS, NULL);

    /* update state */
    oxws->state = OXWS_STATE_READY_FOR_REQUESTS;
  }

  /* reset progress callback state */
  oxws->progress_callback.last = 0;

  return OXWS_NO_ERROR;
}

int oxws_curl_progress_callback(void* userdata, double dltotal, double dlnow, double ultotal, double ulnow) {
  UNUSED(ultotal);
  UNUSED(ulnow);

  oxws* oxws = (struct oxws*) userdata;
  if(oxws == NULL) return 0; /* TODO warn */

  /* call the configured progress callback */
  if(oxws->progress_callback.callback != NULL) {
    size_t dlnow_s = (size_t) dlnow;
    size_t dltotal_s = (size_t) dltotal;
    if(oxws->progress_callback.rate == 0 ||
       dlnow_s - oxws->progress_callback.last > oxws->progress_callback.rate ||
       (dlnow_s == dltotal_s && dlnow_s > oxws->progress_callback.last)) {
      oxws->progress_callback.callback(dlnow_s, dltotal_s, oxws->progress_callback.userdata);
      oxws->progress_callback.last = dlnow_s;
    }
  }

  return 0;
}
