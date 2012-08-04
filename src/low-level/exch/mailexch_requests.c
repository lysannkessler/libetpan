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
#include <libetpan/mailexch_helper.h>

#include <stdlib.h>
#include <string.h>


/* maps from mailexch_distinguished_folder_id to string */
const char* mailexch_distfolder_id_name_map[] = {
  "calendar", "contacts", "deleteditems", "drafts", "inbox", "journal", "notes",
  "outbox", "sentitems", "tasks", "msgfolderroot", "root", "junkemail",
  "searchfolders", "voicemail"
};
const short mailexch_distfolder_id_name_map_length =
  sizeof(mailexch_distfolder_id_name_map) / sizeof(const char*);


int mailexch_list(mailexch* exch,
        mailexch_distinguished_folder_id distfolder_id, const char* folder_id,
        int count, carray** list) {

  if(distfolder_id == MAILEXCH_DISTFOLDER__NONE && folder_id == NULL)
    return MAILEXCH_ERROR_INVALID_PARAMETER;
  if(distfolder_id != MAILEXCH_DISTFOLDER__NONE &&
       (distfolder_id < MAILEXCH_DISTFOLDER__MIN ||
        distfolder_id > MAILEXCH_DISTFOLDER__MAX))
    return MAILEXCH_ERROR_INVALID_PARAMETER;

  const char* request_format =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
    "  <soap:Body>\n"
    "    <FindItem xmlns=\"http://schemas.microsoft.com/exchange/services/2006/messages\"\n"
    "              xmlns:t=\"http://schemas.microsoft.com/exchange/services/2006/types\"\n"
    "              Traversal=\"Shallow\">\n"
    "      <ItemShape>\n"
    "        <t:BaseShape>IdOnly</t:BaseShape>\n"
    "        <t:AdditionalProperties>\n"
    "          <t:FieldURI FieldURI=\"item:Subject\"/>\n"
    "        </t:AdditionalProperties>\n"
    "      </ItemShape>\n"
    "      <IndexedPageItemView%s BasePoint=\"Beginning\" Offset=\"0\" />\n"
    "      <ParentFolderIds>%s</ParentFolderIds>\n"
    "    </FindItem>\n"
    "  </soap:Body>\n"
    "</soap:Envelope>";
  size_t request_length = strlen(request_format) - 4; /* without format chars */
  const char* max_entries_returned_format = " MaxEntriesReturned=\"%d\"";
  const char* distfolder_id_format = "<t:DistinguishedFolderId Id=\"%s\"/>";
  const char* folder_id_format = "<t:FolderId Id=\"%s\"/>";

  char* request, *max_entries_returned = NULL, *folder_distfolder_id = NULL;

  /* max_entries_returned */
  if(count >= 0) {
    /* MaxEntriesReturned="" + max uint length when printed + \0 */
    max_entries_returned = (char*) malloc(21 + 10 + 1);
    if(!max_entries_returned) return MAILEXCH_ERROR_INTERNAL;
    sprintf(max_entries_returned, max_entries_returned_format, count);
    request_length += strlen(max_entries_returned);
  }

  /* folder_distfolder_id */
  if(distfolder_id != MAILEXCH_DISTFOLDER__NONE && distfolder_id >= 0 &&
          distfolder_id < mailexch_distfolder_id_name_map_length) {
    const char* distfolder_id_str =
      mailexch_distfolder_id_name_map[distfolder_id];
    /* <t:DistinguishedFolderId Id=""/> + dist. folder id string + \0 */
    folder_distfolder_id = (char*) malloc(32 + strlen(distfolder_id_str) + 1);
    if(folder_distfolder_id)
      sprintf(folder_distfolder_id, distfolder_id_format, distfolder_id_str);
  } else if(folder_id != NULL) {
    /* <t:FolderId Id=""/> + folder id string + \0 */
    folder_distfolder_id = (char*) malloc(19 + strlen(folder_id) + 1);
    if(folder_distfolder_id)
      sprintf(folder_distfolder_id, folder_id_format, folder_id);
  }
  if(!folder_distfolder_id) {
    if(max_entries_returned) free(max_entries_returned);
    return MAILEXCH_ERROR_INTERNAL;
  }
  request_length += strlen(folder_distfolder_id);

  /* request */
  request = (char*) malloc(request_length + 1);
  if(!request) {
    free(folder_distfolder_id);
    if(max_entries_returned) free(max_entries_returned);
    return MAILEXCH_ERROR_INTERNAL;
  }
  sprintf(request, request_format,
          max_entries_returned ? max_entries_returned : "",
          folder_distfolder_id);

  free(folder_distfolder_id);
  if(max_entries_returned) free(max_entries_returned);

  /* headers */
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: text/xml");
  curl_easy_setopt(exch->curl, CURLOPT_HTTPHEADER, headers);

  /* content */
  curl_easy_setopt(exch->curl, CURLOPT_POSTFIELDS, request);

  /* result */
  if(mailexch_write_response_to_buffer(exch,
        MAILEXCH_DEFAULT_RESPONSE_BUFFER_LENGTH) != MAILEXCH_NO_ERROR) {

    curl_slist_free_all(headers);
    free(request);
    return MAILEXCH_ERROR_INTERNAL;
  }

  /* perform request */
  CURLcode curl_code = curl_easy_perform(exch->curl);
  int result = MAILEXCH_ERROR_CANT_LIST;
  if(curl_code == CURLE_OK) {
    long http_response = 0;
    curl_easy_getinfo (exch->curl, CURLINFO_RESPONSE_CODE, &http_response);
    if(http_response == 200) {
      result = MAILEXCH_NO_ERROR;
      puts(exch->response_buffer->str);
    }
  }

  /* cleanup */
  curl_slist_free_all(headers);
  free(request);
  return result;
}
