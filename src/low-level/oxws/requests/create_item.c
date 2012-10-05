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


oxws_result oxws_create_item(oxws* oxws, oxws_item* item, oxws_message_disposition message_disposition,
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
  if(item->class_id == OXWS_ITEM_CLASS_MESSAGE) {
    oxws_message* message = (oxws_message*) item;
    /* build message:
      <t:Message>
        <t:ItemClass>IPM.Note</t:ItemClass>
        [other properties that are set]
      </t:Message>
    */
    /* TODO check for required properties */
    xmlNodePtr node_message = xmlNewChild(node_items, ns_exch_types, BAD_CAST "Message", NULL);
    xmlChar* item_class = BAD_CAST "IPM.Note";
    if(item->item_class != NULL) item_class = BAD_CAST item->item_class;
    xmlNewChild(node_message, ns_exch_types, BAD_CAST "ItemClass", item_class);

    if(message->item.subject != NULL)
      xmlNewChild(node_message, ns_exch_types, BAD_CAST "Subject", BAD_CAST message->item.subject->str);
    if(message->item.body != NULL) {
      const xmlChar* body = message->item.body->string == NULL ? NULL : BAD_CAST message->item.body->string->str;
      xmlNodePtr node_body = xmlNewChild(node_message, ns_exch_types, BAD_CAST "Body", body);
      if(message->item.body->body_type != OXWS_BODY_TYPE__NOT_SET) {
        const xmlChar* body_type = NULL;
        if(message->item.body->body_type == OXWS_BODY_TYPE_HTML)
          body_type = BAD_CAST "HTML";
        else if(message->item.body->body_type == OXWS_BODY_TYPE_TEXT)
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
        oxws_email_address* address = (oxws_email_address*) carray_get(message->to_recipients, i);
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
  oxws_create_item_sax_context sax_context;
  if(oxws_create_item_sax_context_init(&sax_context, item) != OXWS_NO_ERROR ||
     oxws_handle_response_xml(oxws, &oxws_create_item_sax_handler, &sax_context) != OXWS_NO_ERROR) {
    oxws_release_response_xml_parser(oxws);
    return OXWS_ERROR_INTERNAL;
  }

  /* perform request */
  int result = oxws_perform_request_xml(oxws, node_create_item);
  if(sax_context.state == OXWS_CREATE_ITEM_SAX_CONTEXT_STATE__ERROR) {
    result = OXWS_ERROR_INVALID_RESPONSE;
  } else if (sax_context.state != OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_END_DOCUMENT) {
    result = OXWS_ERROR_INTERNAL;
  }

  /* clean up */
  oxws_release_response_xml_parser(oxws);
  return result;
}

oxws_result oxws_create_item_sax_context_init(oxws_create_item_sax_context* context, oxws_item* item) {
  if(context == NULL) return OXWS_ERROR_INVALID_PARAMETER;
  memset(context, 0, sizeof(oxws_create_item_sax_context));
  context->item = item;
  return OXWS_NO_ERROR;
}


/*
  SAX handlers
*/

void oxws_create_item_sax_handler_start_document(void* user_data) {
  if(user_data == NULL) return;
  oxws_create_item_sax_context* context = (oxws_create_item_sax_context*) user_data;

  if(context->state != OXWS_CREATE_ITEM_SAX_CONTEXT_STATE__NONE)
    OXWS_CREATE_ITEM_SAX_CONTEXT_SET_STATE(_ERROR);
  if(OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS_ERROR()) return;

  OXWS_CREATE_ITEM_SAX_CONTEXT_SET_STATE(START_DOCUMENT);
}

void oxws_create_item_sax_handler_end_document(void* user_data) {
  if(user_data == NULL) return;
  oxws_create_item_sax_context* context = (oxws_create_item_sax_context*) user_data;
  /* TODO check state */
  if(OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS_ERROR()) return;
  OXWS_CREATE_ITEM_SAX_CONTEXT_SET_STATE(END_DOCUMENT);
}

void oxws_create_item_sax_handler_start_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri,
        int nb_namespaces, const xmlChar** namespaces,
        int nb_attributes, int nb_defaulted, const xmlChar** attrs) {
  UNUSED(prefix);
  UNUSED(nb_namespaces); UNUSED(namespaces);
  UNUSED(nb_defaulted);


  if(user_data == NULL || localname == NULL) return;
  oxws_create_item_sax_context* context = (oxws_create_item_sax_context*) user_data;
  if(OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS_ERROR()) return;

  short inc_item_node_depth = 1;

  /* TODO go to error state for invalid state-tag combinations */
  /* TODO m:CreateItemResponseMessage, m:ResponseCode, multiple response messages */

  if(OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(START_DOCUMENT, EXCH_MESSAGES, "Items")) {
    /* TODO check items_node_depth */
    OXWS_CREATE_ITEM_SAX_CONTEXT_SET_STATE(ITEMS);

  } else if(OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS(ITEMS)) {
    if(context->item != NULL || context->item_node_depth != 0) {
      /* TODO warn */
    }
    OXWS_CREATE_ITEM_SAX_CONTEXT_SET_STATE(ITEM);
    context->item_node_depth = 1;
    inc_item_node_depth = 0; /* don't increment again on this level, only on levels below */

  /* toplevel nodes within any Item (or derived class) */
  } else if(OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS_ITEM_TOP_LEVEL()) {
    if(OXWS_CREATE_ITEM_SAX_IS_NS_NODE(EXCH_TYPES, "ItemId")) {
      xmlChar* id = NULL, *change_key = NULL;
      OXWS_CREATE_ITEM_SAX_CONTEXT_PARSE_ITEM_ID(id, change_key);
      oxws_item_set_item_id_fields(context->item, (char*)id, (char*)change_key);
      /* TODO warn if result != NO_ERROR */
      xmlFree(id); xmlFree(change_key);

    } else {
      /* unknown toplevel nodes within any Item (or derived class) */
      /* TODO warn */
    }

  } else {
    /* unknown node */
    /* TODO warn */
  }

  if(context->item_node_depth > 0 && inc_item_node_depth)
    context->item_node_depth++;
}

void oxws_create_item_sax_handler_end_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri) {
  UNUSED(prefix);

  if(user_data == NULL || localname == NULL) return;
  oxws_create_item_sax_context* context = (oxws_create_item_sax_context*) user_data;
  if(OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS_ERROR()) return;

  if(OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(START_DOCUMENT, SOAP, "Envelope")) {
    /* the end_document callback does not seem to get called. We emulate it
       using the end of the SOAP Envelope tag */
    oxws_create_item_sax_handler_end_document(user_data);

  } else if(OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(ITEMS, EXCH_MESSAGES, "Items")) {
    OXWS_CREATE_ITEM_SAX_CONTEXT_SET_STATE(START_DOCUMENT);

  } else if(OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS_ITEM_TOP_LEVEL()) {
    context->item_node_depth = 0;
    OXWS_CREATE_ITEM_SAX_CONTEXT_SET_STATE(ITEMS);

  } else {
    /* TODO warn for unknown tags */
    /* TODO go to error state for invalid state-tag combinations */
  }

  if(context->item_node_depth > 0)
    context->item_node_depth--;
}

void oxws_create_item_sax_handler_characters(void* user_data, const xmlChar* chars, int length) {
  UNUSED(length);

  if(user_data == NULL || chars == NULL) return;
  oxws_create_item_sax_context* context = (oxws_create_item_sax_context*) user_data;
  if(OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS_ERROR()) return;

  /* TODO warn for suspicious states? */
}

void oxws_create_item_sax_handler_error(void* user_data, const char* message, ...) {
  /* TODO log error message */
  UNUSED(message);

  if(user_data == NULL) return;
  oxws_create_item_sax_context* context = (oxws_create_item_sax_context*) user_data;
  OXWS_CREATE_ITEM_SAX_CONTEXT_SET_STATE(_ERROR);
}

xmlSAXHandler oxws_create_item_sax_handler = {
  NULL, /* internalSubset */
  NULL, /* isStandalone */
  NULL, /* hasInternalSubset */
  NULL, /* hasExternalSubset */
  NULL, /* resolveEntity */
  NULL, /* getEntity */
  NULL, /* entityDecl */
  NULL, /* notationDecl */
  NULL, /* attributeDecl */
  NULL, /* elementDecl */
  NULL, /* unparsedEntityDecl */
  NULL, /* setDocumentLocator */
  oxws_create_item_sax_handler_start_document,
  oxws_create_item_sax_handler_end_document,
  NULL, /* startElement */
  NULL, /* endElement */
  NULL, /* reference */
  oxws_create_item_sax_handler_characters,
  NULL, /* ignorableWhitespace */
  NULL, /* processingInstruction */
  NULL, /* comment */
  NULL, /* TODO warning */
  oxws_create_item_sax_handler_error, /* error */
  oxws_create_item_sax_handler_error, /* fatalError */
  NULL, /* getParameterEntity */
  NULL, /* cdataBlock */
  NULL, /* externalSubset */
  XML_SAX2_MAGIC, /* initialized */
  NULL, /* _private */
  oxws_create_item_sax_handler_start_element_ns,
  oxws_create_item_sax_handler_end_element_ns,
  NULL, /* serror */
};
