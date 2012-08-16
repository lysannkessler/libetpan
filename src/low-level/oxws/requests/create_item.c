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

#include "create_item.h"
#include <libetpan/oxws_requests.h>
#include <libetpan/oxws_types.h>
#include "helper.h"
#include "types_internal.h"
#include "xml.h"

#include <stdlib.h>
#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>


oxws_result oxws_create_item(oxws* oxws, oxws_type_item* item, oxws_message_disposition message_disposition,
        oxws_distinguished_folder_id saved_item_distfolder_id, const char* saved_item_folder_id) {

  /* check parameters */
  if(oxws == NULL || item == NULL) {
    return OXWS_ERROR_INVALID_PARAMETER;
  }
  if(message_disposition != OXWS_MESSAGE_DISPOSITION__NONE &&
     (message_disposition < OXWS_MESSAGE_DISPOSITION__MIN || message_disposition > OXWS_MESSAGE_DISPOSITION__MAX)) {
      return OXWS_ERROR_INVALID_PARAMETER;
  }
  if(saved_item_distfolder_id != OXWS_DISTFOLDER__NONE &&
     (saved_item_distfolder_id < OXWS_DISTFOLDER__MIN || saved_item_distfolder_id > OXWS_DISTFOLDER__MAX)) {
      return OXWS_ERROR_INVALID_PARAMETER;
  }

  if(oxws_prepare_for_requests(oxws) != OXWS_NO_ERROR)
    return OXWS_ERROR_INTERNAL;
  if(oxws->state != OXWS_STATE_READY_FOR_REQUESTS)
    return OXWS_ERROR_BAD_STATE;

  /* build request body:
    <CreateItem xmlns="http://schemas.microsoft.com/exchange/services/2006/messages"
                xmlns:t="http://schemas.microsoft.com/exchange/services/2006/types"
                MessageDisposition="[message_disposition]"> <!-- optional -->
      <SavedItemFolderId> <!-- optional -->
        <t:DistinguishedFolderId Id="[saved_item_distfolder_id]"/> <!-- or -->
        <t:FolderId Id="[saved_item_folder_id]"/>
      </SavedItemFolderId>
      <Items>[item]</m:Items>
    </CreateItem>
  */

  xmlNodePtr node_create_item = NULL;
  xmlNsPtr ns_exch_messages = NULL, ns_exch_types = NULL;
  oxws_prepare_xml_request_method_node("CreateItem", &node_create_item, &ns_exch_messages, &ns_exch_types);
  if(message_disposition != OXWS_MESSAGE_DISPOSITION__NONE) {
    xmlChar* message_disposition_str = NULL;
    if(message_disposition == OXWS_MESSAGE_DISPOSITION_SAVE_ONLY)
      message_disposition_str = BAD_CAST "SaveOnly";
    if(message_disposition == OXWS_MESSAGE_DISPOSITION_SEND_ONLY)
      message_disposition_str = BAD_CAST "SendOnly";
    if(message_disposition == OXWS_MESSAGE_DISPOSITION_SEND_AND_SAVE_COPY)
      message_disposition_str = BAD_CAST "SendAndSaveCopy";
    if(message_disposition_str == NULL) {
      /* TODO warn */
    } else {
      xmlNewProp(node_create_item, BAD_CAST "MessageDisposition", message_disposition_str);
    }
  }

  if(saved_item_distfolder_id != OXWS_DISTFOLDER__NONE || saved_item_folder_id != NULL) {
    xmlNodePtr node_saved_item_folder_id = xmlNewChild(node_create_item, ns_exch_messages, BAD_CAST "SavedItemFolderId", NULL);
    if(saved_item_distfolder_id != OXWS_DISTFOLDER__NONE && saved_item_distfolder_id >= 0 &&
       saved_item_distfolder_id < oxws_distfolder_id_name_map_length) {
      xmlNodePtr node_distinguished_folder_id = xmlNewChild(node_saved_item_folder_id, ns_exch_types, BAD_CAST "DistinguishedFolderId", NULL);
      xmlNewProp(node_distinguished_folder_id, BAD_CAST "Id", BAD_CAST oxws_distfolder_id_name_map[saved_item_distfolder_id]);
    } else if(saved_item_folder_id != NULL) {
      xmlNodePtr node_folder_id = xmlNewChild(node_saved_item_folder_id, ns_exch_types, BAD_CAST "FolderId", NULL);
      xmlNewProp(node_folder_id, BAD_CAST "Id", BAD_CAST saved_item_folder_id);
    }
  }

  xmlNodePtr node_items = xmlNewChild(node_create_item, ns_exch_messages, BAD_CAST "Items", NULL);
  if(item->item_class == OXWS_TYPE_ITEM_CLASS_MESSAGE) {
    oxws_type_message* message = (oxws_type_message*) item;
    /* build message:
      <t:Message>
        <t:ItemClass>IPM.Note</t:ItemClass>
        [other properties that are set]
      </t:Message>
    */
    /* TODO check for required properties */
    xmlNodePtr node_message = xmlNewChild(node_items, ns_exch_types, BAD_CAST "Message", NULL);
    xmlNewChild(node_message, ns_exch_types, BAD_CAST "ItemClass", BAD_CAST "IPM.Note"); /* TODO use configured ItemClass property as override */

    if(message->item.subject != NULL)
      xmlNewChild(node_message, ns_exch_types, BAD_CAST "Subject", BAD_CAST message->item.subject->str);
    if(message->item.body != NULL) {
      const xmlChar* body = message->item.body->string == NULL ? NULL : BAD_CAST message->item.body->string->str;
      xmlNodePtr node_body = xmlNewChild(node_message, ns_exch_types, BAD_CAST "Body", body);
      if(message->item.body->body_type != OXWS_TYPE_BODY_TYPE__NOT_SET) {
        const xmlChar* body_type = NULL;
        if(message->item.body->body_type == OXWS_TYPE_BODY_TYPE_HTML)
          body_type = BAD_CAST "HTML";
        else if(message->item.body->body_type == OXWS_TYPE_BODY_TYPE_TEXT)
          body_type = BAD_CAST "Text";
        if(body_type != NULL) {
          xmlNewProp(node_body, BAD_CAST "BodyType", body_type);
        } else {
          /* TODO warn */
        }
      }
    }
    if(message->to_recipients != NULL) {
      xmlNodePtr node_to_recipients = xmlNewChild(node_message, ns_exch_types, BAD_CAST "ToRecipients", NULL);
      size_t i;
      for(i = 0; i < message->to_recipients->len; i++) {
        oxws_type_email_address* address = (oxws_type_email_address*) carray_get(message->to_recipients, i);
        xmlNodePtr node_mailbox = xmlNewChild(node_to_recipients, ns_exch_types, BAD_CAST "Mailbox", NULL);
        if(address->name != NULL)
          xmlNewChild(node_mailbox, ns_exch_types, BAD_CAST "Name", BAD_CAST address->name);
        if(address->email_address != NULL)
          xmlNewChild(node_mailbox, ns_exch_types, BAD_CAST "EmailAddress", BAD_CAST address->email_address);
        /* TODO serialize other properties */
      }
    }
    /* TODO cc_recipients, bcc_recipients, from, is_read */

  } else {
    /* TODO implement other classes */
    /* TODO warn */
  }

  /* configure response XML parser */
  if(oxws_handle_response_xml(oxws, NULL, NULL) != OXWS_NO_ERROR) {
    oxws_release_response_xml_parser(oxws);
    return OXWS_ERROR_INTERNAL;
  }

  /* perform request */
  int result = oxws_perform_request_xml(oxws, node_create_item);

  /* print response */
  xmlDocPtr response = oxws_get_response_xml(oxws);
  xmlChar* response_string = NULL;
  xmlDocDumpMemory(response, &response_string, NULL);
  if(response_string != NULL) {
    puts((char*) response_string);
    xmlFree(response_string);
  }

  /* clean up */
  oxws_release_response_xml_parser(oxws);
  return result;
}
