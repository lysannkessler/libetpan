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
#include "helper.h"
#include "types_internal.h"
#include "xml.h"

#include <stdlib.h>
#include <string.h>

#include <libxml/tree.h>


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

  /* check parameters */
  if(distfolder_id == MAILEXCH_DISTFOLDER__NONE && folder_id == NULL)
    return MAILEXCH_ERROR_INVALID_PARAMETER;
  if(distfolder_id != MAILEXCH_DISTFOLDER__NONE &&
    (distfolder_id < MAILEXCH_DISTFOLDER__MIN ||
     distfolder_id > MAILEXCH_DISTFOLDER__MAX)) {
      return MAILEXCH_ERROR_INVALID_PARAMETER;
  }

  if(mailexch_prepare_for_requests(exch) != MAILEXCH_NO_ERROR)
    return MAILEXCH_ERROR_INTERNAL;
  if(exch->state != MAILEXCH_STATE_READY_FOR_REQUESTS)
    return MAILEXCH_ERROR_BAD_STATE;

  /* build request body:
    <FindItem xmlns="http://schemas.microsoft.com/exchange/services/2006/messages"
              xmlns:t="http://schemas.microsoft.com/exchange/services/2006/types"
              Traversal="Shallow">
      <ItemShape>
        <t:BaseShape>IdOnly</t:BaseShape>
        <t:AdditionalProperties>
          <t:FieldURI FieldURI="item:Subject"/>
        </t:AdditionalProperties>
      </ItemShape>
      <IndexedPageItemView BasePoint="Beginning" Offset="0"
                           MaxEntriesReturned="[count]"/> <!-- if count >= 0 -->
      <ParentFolderIds>
        <t:DistinguishedFolderId Id="[distfolder_id]"/> <!-- or -->
        <t:FolderId Id="[folder_id]"/>
      </ParentFolderIds>
    </FindItem>
  */

  xmlNodePtr node_findItem = NULL;
  xmlNsPtr ns_exch_messages = NULL, ns_exch_types = NULL;
  mailexch_prepare_xml_request_method_node("FindItem", &node_findItem,
          &ns_exch_messages, &ns_exch_types);
  xmlNewProp(node_findItem, BAD_CAST "Traversal", BAD_CAST "Shallow");

  xmlNodePtr node_itemShape = xmlNewChild(node_findItem, ns_exch_messages,
          BAD_CAST "ItemShape", NULL);
  xmlNewChild(node_itemShape, ns_exch_types, BAD_CAST "BaseShape",
          BAD_CAST "IdOnly");
  xmlNodePtr node_props = xmlNewChild(node_itemShape, ns_exch_types,
          BAD_CAST "AdditionalProperties", NULL);
  xmlNodePtr node_fieldUri = xmlNewChild(node_props, ns_exch_types,
          BAD_CAST "FieldURI", NULL);
  xmlNewProp(node_fieldUri, BAD_CAST "FieldURI", BAD_CAST "item:Subject");

  xmlNodePtr node_indexedPageItemView = xmlNewChild(node_findItem,
          ns_exch_messages, BAD_CAST "IndexedPageItemView", NULL);
  xmlNewProp(node_indexedPageItemView, BAD_CAST "BasePoint",
          BAD_CAST "Beginning");
  xmlNewProp(node_indexedPageItemView, BAD_CAST "Offset", BAD_CAST "0");
  if(count >= 0) {
    /* max uint length when printed + \0 */
    char* max_entries_returned = (char*) malloc(10 + 1);
    if(!max_entries_returned) return MAILEXCH_ERROR_INTERNAL;
    sprintf(max_entries_returned, "%d", count);
    xmlNewProp(node_indexedPageItemView, BAD_CAST "MaxEntriesReturned",
            BAD_CAST max_entries_returned);
    free(max_entries_returned);
  }

  xmlNodePtr node_parentFolderIds = xmlNewChild(node_findItem, ns_exch_messages,
          BAD_CAST "ParentFolderIds", NULL);
  if(distfolder_id != MAILEXCH_DISTFOLDER__NONE && distfolder_id >= 0 &&
     distfolder_id < mailexch_distfolder_id_name_map_length) {

    xmlNodePtr node_distinguishedFolderId = xmlNewChild(node_parentFolderIds,
            ns_exch_types, BAD_CAST "DistinguishedFolderId", NULL);
    xmlNewProp(node_distinguishedFolderId, BAD_CAST "Id",
            BAD_CAST mailexch_distfolder_id_name_map[distfolder_id]);
  } else if(folder_id != NULL) {
    xmlNodePtr node_folderId = xmlNewChild(node_parentFolderIds, ns_exch_types,
            BAD_CAST "FolderId", NULL);
    xmlNewProp(node_folderId, BAD_CAST "Id", BAD_CAST folder_id);
  }

  /* perform request */
  xmlDocPtr response; xmlNodePtr response_body;
  int result = mailexch_perform_request_xml(exch, node_findItem, &response,
          &response_body);

  if(result == MAILEXCH_NO_ERROR && response != NULL) {
    xmlChar* response_str = NULL;
    xmlDocDumpFormatMemory(response, &response_str, NULL, 0);
    puts((char*)response_str);
    xmlFree(response_str);
  }

  /* clean up */
  mailexch_release_response_xml(exch);
  return result;
}


int mailexch_prepare_for_requests(mailexch* exch) {
  mailexch_internal* internal = MAILEXCH_INTERNAL(exch);

  if(exch->state != MAILEXCH_STATE_CONNECTED) return MAILEXCH_NO_ERROR;

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

  /* parse response XML */
  int result = mailexch_save_response_xml(exch);
  /* reallocate empty response string buffer */
  if(internal->response_buffer) {
    mmap_string_free(internal->response_buffer);
    internal->response_buffer = mmap_string_sized_new(0);
  }

  /* clean up */
  if(result == MAILEXCH_NO_ERROR) {
    exch->state = MAILEXCH_STATE_READY_FOR_REQUESTS;
  } else {
    if(internal->curl_headers) {
      curl_slist_free_all(internal->curl_headers);
      internal->curl_headers = NULL;
      curl_easy_setopt(internal->curl, CURLOPT_HTTPHEADER, NULL);
    }
  }
  return result;
}
