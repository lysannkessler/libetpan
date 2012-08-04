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

#include <libxml/tree.h>
#include <libxml/parser.h>


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

  /* build request document */
  /*
    <?xml version="1.0"?>
    <soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
      <soap:Body>
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
      </soap:Body>
    </soap:Envelope>
  */

  xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
  xmlNodePtr node_envelope = xmlNewNode(NULL, BAD_CAST "Envelope");
  xmlNsPtr ns_soap = xmlNewNs(node_envelope, BAD_CAST "http://schemas.xmlsoap.org/soap/envelope/", BAD_CAST "soap");
  xmlSetNs(node_envelope, ns_soap);
  xmlDocSetRootElement(doc, node_envelope);

  xmlNodePtr node_body = xmlNewChild(node_envelope, ns_soap, BAD_CAST "Body", NULL);
  xmlNodePtr node_findItem = xmlNewChild(node_body, NULL, BAD_CAST "FindItem", NULL);
  xmlNsPtr ns_exch_messages = xmlNewNs(node_findItem, BAD_CAST "http://schemas.microsoft.com/exchange/services/2006/messages", NULL);
  xmlNsPtr ns_exch_types = xmlNewNs(node_findItem, BAD_CAST "http://schemas.microsoft.com/exchange/services/2006/types", BAD_CAST "t");
  xmlSetNs(node_findItem, ns_exch_messages);
  xmlNewProp(node_findItem, BAD_CAST "Traversal", BAD_CAST "Shallow");

  xmlNodePtr node_itemShape = xmlNewChild(node_findItem, ns_exch_messages, BAD_CAST "ItemShape", NULL);
  xmlNewChild(node_itemShape, ns_exch_types, BAD_CAST "BaseShape", BAD_CAST "IdOnly");
  xmlNodePtr node_props = xmlNewChild(node_itemShape, ns_exch_types, BAD_CAST "AdditionalProperties", NULL);
  xmlNodePtr node_fieldUri = xmlNewChild(node_props, ns_exch_types, BAD_CAST "FieldURI", NULL);
  xmlNewProp(node_fieldUri, BAD_CAST "FieldURI", BAD_CAST "item:Subject");

  xmlNodePtr node_indexedPageItemView = xmlNewChild(node_findItem, ns_exch_messages, BAD_CAST "IndexedPageItemView", NULL);
  xmlNewProp(node_indexedPageItemView, BAD_CAST "BasePoint", BAD_CAST "Beginning");
  xmlNewProp(node_indexedPageItemView, BAD_CAST "Offset", BAD_CAST "0");
  if(count >= 0) {
    /* max uint length when printed + \0 */
    char* max_entries_returned = (char*) malloc(10 + 1);
    if(!max_entries_returned) {
      xmlFreeDoc(doc);
      return MAILEXCH_ERROR_INTERNAL;
    }
    sprintf(max_entries_returned, "%d", count);
    xmlNewProp(node_indexedPageItemView, BAD_CAST "MaxEntriesReturned", BAD_CAST max_entries_returned);
    free(max_entries_returned);
  }

  xmlNodePtr node_parentFolderIds = xmlNewChild(node_findItem, ns_exch_messages, BAD_CAST "ParentFolderIds", NULL);
  if(distfolder_id != MAILEXCH_DISTFOLDER__NONE && distfolder_id >= 0 && distfolder_id < mailexch_distfolder_id_name_map_length) {
    xmlNodePtr node_distinguishedFolderId = xmlNewChild(node_parentFolderIds, ns_exch_types, BAD_CAST "DistinguishedFolderId", NULL);
    xmlNewProp(node_distinguishedFolderId, BAD_CAST "Id", BAD_CAST mailexch_distfolder_id_name_map[distfolder_id]);
  } else if(folder_id != NULL) {
    xmlNodePtr node_folderId = xmlNewChild(node_parentFolderIds, ns_exch_types, BAD_CAST "FolderId", NULL);
    xmlNewProp(node_folderId, BAD_CAST "Id", BAD_CAST folder_id);
  }

  /* dump request to buffer */
  xmlChar* request = NULL;
  xmlDocDumpFormatMemory(doc, &request, NULL, 0);
  xmlFreeDoc(doc);

  /* perform request */
  long http_response_code = 0;
  const char* response = NULL;
  int result = mailexch_perform_request(exch, request,
                                        &http_response_code, &response);
  if(result == MAILEXCH_NO_ERROR) {
    puts(response);
  }

  /* clean up */
  xmlFree(request);
  return result;
}


int mailexch_prepare_for_requests(mailexch* exch) {
  /* paranoia */
  curl_easy_setopt(exch->curl, CURLOPT_FOLLOWLOCATION, 0L);
  curl_easy_setopt(exch->curl, CURLOPT_UNRESTRICTED_AUTH, 0L);

  /* post to AsUrl */
  curl_easy_setopt(exch->curl, CURLOPT_POST, 1L);
  curl_easy_setopt(exch->curl, CURLOPT_URL, exch->connection_settings.as_url);

  /* Clear headers and set Content-Type to text/xml. */
  if(exch->curl_headers) {
    curl_slist_free_all(exch->curl_headers);
    exch->curl_headers = NULL;
  }
  exch->curl_headers = curl_slist_append(exch->curl_headers,
                                         "Content-Type: text/xml");
  curl_easy_setopt(exch->curl, CURLOPT_HTTPHEADER, exch->curl_headers);

  /* clear request string for now */
  curl_easy_setopt(exch->curl, CURLOPT_POSTFIELDS, NULL);

  /* write to response buffer */
  int result = mailexch_write_response_to_buffer(exch,
        MAILEXCH_DEFAULT_RESPONSE_BUFFER_LENGTH);

  /* clean up */
  if(result != MAILEXCH_NO_ERROR) {
    if(exch->curl_headers) {
      curl_slist_free_all(exch->curl_headers);
      exch->curl_headers = NULL;
      curl_easy_setopt(exch->curl, CURLOPT_HTTPHEADER, NULL);
    }
  }
  return result;
}

int mailexch_perform_request(mailexch* exch, const char* request,
        long* http_response_code, const char** response) {

  /* initialize output variables */
  *http_response_code = 0;
  *response = NULL;

  /* set body */
  curl_easy_setopt(exch->curl, CURLOPT_POSTFIELDS, request);

  /* clean response buffer */
  mmap_string_truncate(exch->response_buffer, 0);

  /* perform request */
  CURLcode curl_code = curl_easy_perform(exch->curl);
  int result = MAILEXCH_ERROR_CONNECT;
  if(curl_code == CURLE_OK) {
    *response = exch->response_buffer->str;

    result = MAILEXCH_ERROR_REQUEST_FAILED;
    curl_easy_getinfo (exch->curl, CURLINFO_RESPONSE_CODE, http_response_code);
    if(*http_response_code == 200)
      result = MAILEXCH_NO_ERROR;
  }

  /* clean up */
  curl_easy_setopt(exch->curl, CURLOPT_POSTFIELDS, NULL);
  return result;
}
