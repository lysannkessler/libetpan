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

#include "find_item.h"
#include <libetpan/oxws_requests.h>
#include "helper.h"
#include "types_internal.h"
#include "xml.h"

#include <stdlib.h>
#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>


#define OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, expected_ns_suffix, expected_name) \
  (xmlStrcmp(ns_uri, OXWS_XML_NS_##expected_ns_suffix) == 0 && xmlStrcmp(localname, BAD_CAST expected_name) == 0)


oxws_result oxws_find_item(oxws* oxws,
        oxws_distinguished_folder_id distfolder_id, const char* folder_id,
        int count, carray** list) {

  /* check parameters */
  if(oxws == NULL || list == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;
  if(distfolder_id == OXWS_DISTFOLDER__NONE && folder_id == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;
  if(distfolder_id != OXWS_DISTFOLDER__NONE &&
     (distfolder_id < OXWS_DISTFOLDER__MIN || distfolder_id > OXWS_DISTFOLDER__MAX)) {
      return OXWS_ERROR_INVALID_PARAMETER;
  }

  if(oxws_prepare_for_requests(oxws) != OXWS_NO_ERROR)
    return OXWS_ERROR_INTERNAL;
  if(oxws->state != OXWS_STATE_READY_FOR_REQUESTS)
    return OXWS_ERROR_BAD_STATE;

  /* build request body:
    <FindItem xmlns="http://schemas.microsoft.com/exchange/services/2006/messages"
              xmlns:t="http://schemas.microsoft.com/exchange/services/2006/types"
              Traversal="Shallow">
      <ItemShape>
        <t:BaseShape>Default</t:BaseShape>
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
  oxws_prepare_xml_request_method_node("FindItem", &node_findItem, &ns_exch_messages, &ns_exch_types);
  xmlNewProp(node_findItem, BAD_CAST "Traversal", BAD_CAST "Shallow");

  xmlNodePtr node_itemShape = xmlNewChild(node_findItem, ns_exch_messages, BAD_CAST "ItemShape", NULL);
  xmlNewChild(node_itemShape, ns_exch_types, BAD_CAST "BaseShape", BAD_CAST "Default");

  xmlNodePtr node_indexedPageItemView = xmlNewChild(node_findItem, ns_exch_messages, BAD_CAST "IndexedPageItemView", NULL);
  xmlNewProp(node_indexedPageItemView, BAD_CAST "BasePoint", BAD_CAST "Beginning");
  xmlNewProp(node_indexedPageItemView, BAD_CAST "Offset", BAD_CAST "0");
  if(count >= 0) {
    /* max uint length when printed + \0 */
    char* max_entries_returned = (char*) malloc(10 + 1);
    if(!max_entries_returned) return OXWS_ERROR_INTERNAL;
    sprintf(max_entries_returned, "%d", count);
    xmlNewProp(node_indexedPageItemView, BAD_CAST "MaxEntriesReturned", BAD_CAST max_entries_returned);
    free(max_entries_returned);
  }

  xmlNodePtr node_parentFolderIds = xmlNewChild(node_findItem, ns_exch_messages, BAD_CAST "ParentFolderIds", NULL);
  if(distfolder_id != OXWS_DISTFOLDER__NONE && distfolder_id >= 0 && distfolder_id < oxws_distfolder_id_name_map_length) {

    xmlNodePtr node_distinguishedFolderId = xmlNewChild(node_parentFolderIds, ns_exch_types, BAD_CAST "DistinguishedFolderId", NULL);
    xmlNewProp(node_distinguishedFolderId, BAD_CAST "Id", BAD_CAST oxws_distfolder_id_name_map[distfolder_id]);
  } else if(folder_id != NULL) {
    xmlNodePtr node_folderId = xmlNewChild(node_parentFolderIds, ns_exch_types, BAD_CAST "FolderId", NULL);
    xmlNewProp(node_folderId, BAD_CAST "Id", BAD_CAST folder_id);
  } else {
    /* TODO warn */
  }

  /* configure response XML parser */
  /* TODO free node_findItem if oxws_find_item_sax_context_init fails */
  oxws_find_item_sax_context sax_context;
  if(oxws_find_item_sax_context_init(&sax_context, count > 0 ? count : 10, list) != OXWS_NO_ERROR ||
     oxws_handle_response_xml(oxws, &oxws_find_item_sax_handler, &sax_context) != OXWS_NO_ERROR) {
    oxws_release_response_xml_parser(oxws);
    /* TODO free node */
    return OXWS_ERROR_INTERNAL;
  }

  /* perform request. the SAX handler will fill the list */
  int result = oxws_perform_request_xml(oxws, node_findItem);
  if(sax_context.state == OXWS_FIND_ITEM_SAX_CONTEXT_STATE__ERROR) {
    result = OXWS_ERROR_INVALID_RESPONSE;
  } else if (sax_context.state != OXWS_FIND_ITEM_SAX_CONTEXT_STATE_END_DOCUMENT) {
    result = OXWS_ERROR_INTERNAL;
  } else if(*list == NULL) {
    result = OXWS_ERROR_INTERNAL;
  }

  /* clean up */
  if(result != OXWS_NO_ERROR) {
    oxws_item_array_free(*list);
    *list = NULL;
  }
  oxws_release_response_xml_parser(oxws);
  return result;
}


oxws_result oxws_find_item_sax_context_init(oxws_find_item_sax_context* context, unsigned int count, carray** list) {
  if(context == NULL || list == NULL)
    return OXWS_ERROR_INVALID_PARAMETER;
  memset(context, 0, sizeof(oxws_find_item_sax_context));
  context->count = count;
  context->list = list;
  return OXWS_NO_ERROR;
}


void oxws_find_item_sax_handler_start_document(void* user_data) {
  if(user_data == NULL) return;
  oxws_find_item_sax_context* context = (oxws_find_item_sax_context*) user_data;

  if(context->state != OXWS_FIND_ITEM_SAX_CONTEXT_STATE__NONE)
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(_ERROR);
  if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ERROR()) return;

  *context->list = carray_new(context->count);

  OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(START_DOCUMENT);
}

void oxws_find_item_sax_handler_end_document(void* user_data) {
  if(user_data == NULL) return;
  oxws_find_item_sax_context* context = (oxws_find_item_sax_context*) user_data;
  /* TODO check state */
  if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ERROR()) return;
  OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(END_DOCUMENT);
}

void oxws_find_item_sax_handler_start_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri,
        int nb_namespaces, const xmlChar** namespaces,
        int nb_attributes, int nb_defaulted, const xmlChar** attrs) {
  UNUSED(prefix);
  UNUSED(nb_namespaces);
  UNUSED(namespaces);
  UNUSED(nb_defaulted);

  if(user_data == NULL || localname == NULL) return;
  oxws_find_item_sax_context* context = (oxws_find_item_sax_context*) user_data;
  if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ERROR()) return;

  /* TODO go to error state for invalid state-tag combinations */
  /* TODO m:FindItemResponseMessage, m:ResponseCode, m:RootFolder, multiple response messages */

  if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(START_DOCUMENT, EXCH_TYPES, "Items")) {
    /* TODO check items_node_depth */
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(ITEMS);

  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(ITEMS)) {
    if(context->item != NULL || context->item_node_depth != 0) {
      /* TODO warn */
    }
    if(OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "Message")) {
      OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(MESSAGE);
      context->item = (oxws_item*) oxws_message_new();
    } else {
      OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(ITEM);
      context->item = oxws_item_new();
    }
    context->item_node_depth = 0;

  /* toplevel nodes within any Item (or derived class) */
  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ANY_ITEM_TOP_LEVEL()) {
    /* TODO check item */

    if(OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "ItemId")) {
      /* TODO check item */
      xmlChar* id = NULL, *change_key = NULL;
      OXWS_FIND_ITEM_SAX_CONTEXT_PARSE_ITEM_ID(id, change_key);
      oxws_item_set_item_id_fields(context->item, (char*)id, (char*)change_key);
      /* TODO warn if result != NO_ERROR */
      xmlFree(id); xmlFree(change_key);
      OXWS_FIND_ITEM_SAX_CONTEXT_PUSH_STATE(ITEM_ITEM_ID);
    } else if(OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "Subject")) {
      OXWS_FIND_ITEM_SAX_CONTEXT_PREPARE_STRING(50);
      OXWS_FIND_ITEM_SAX_CONTEXT_PUSH_STATE(ITEM_SUBJECT);
    } else if(OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "Size")) {
      OXWS_FIND_ITEM_SAX_CONTEXT_PREPARE_STRING(6); /* assume 6 digit number, will expand on demand */
      OXWS_FIND_ITEM_SAX_CONTEXT_PUSH_STATE(ITEM_SIZE);

    /* toplevel nodes within Message */
    } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(MESSAGE)) {
      /* TODO check item class */

      if(OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "From")) {
        OXWS_FIND_ITEM_SAX_CONTEXT_PREPARE_EMAIL_ADDRESS();
        OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(MESSAGE_FROM);
      } else if(OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "IsRead")) {
        OXWS_FIND_ITEM_SAX_CONTEXT_PREPARE_STRING(5); /* true / false / 1 / 0 */
        OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(MESSAGE_IS_READ);
      } else {
        /* unknown toplevel nodes within Message */
        /* TODO warn */
      }

    } else {
      /* unknown toplevel nodes within any Item (or derived class) */
      /* TODO warn */
    }

  /* Mailbox itself and nodes within Mailbox */
  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ANY_EMAIL_ADDRESS() &&
            OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "Mailbox")) {
    OXWS_FIND_ITEM_SAX_CONTEXT_PUSH_STATE(INNER_MAILBOX);
  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(INNER_MAILBOX)) {
    if(OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "Name")) {
      OXWS_FIND_ITEM_SAX_CONTEXT_PREPARE_STRING(20);
      OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(INNER_MAILBOX_NAME);
    } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(INNER_MAILBOX) &&
       OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "EmailAddress")) {
      OXWS_FIND_ITEM_SAX_CONTEXT_PREPARE_STRING(40);
      OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(INNER_MAILBOX_EMAIL_ADDRESS);
    } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(INNER_MAILBOX) &&
       OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "RoutingType")) {
      OXWS_FIND_ITEM_SAX_CONTEXT_PREPARE_STRING(4); /* SMTP */
      OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(INNER_MAILBOX_ROUTING_TYPE);
    } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(INNER_MAILBOX) &&
       OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "MailboxType")) {
      OXWS_FIND_ITEM_SAX_CONTEXT_PREPARE_STRING(12); /* Mailbox/PublicDL/PrivateDL/Contact/PublicFolder */
      OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(INNER_MAILBOX_MAILBOX_TYPE);
    } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(INNER_MAILBOX) &&
       OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, EXCH_TYPES, "ItemId")) {
      /* TODO check email_address */
      xmlChar* id = NULL, *change_key = NULL;
      OXWS_FIND_ITEM_SAX_CONTEXT_PARSE_ITEM_ID(id, change_key);
      oxws_email_address_set_item_id_fields(context->email_address, (char*)id, (char*)change_key);
      /* TODO warn if result != NO_ERROR */
      xmlFree(id); xmlFree(change_key);
      context->prev_state = context->state;
      OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(INNER_MAILBOX_ITEM_ID);
    } else {
      /* unknown node within Mailbox */
      /* TODO warn */
    }

  } else {
    /* unknown node */
    /* TODO warn */
  }

  if(context->item != NULL)
    context->item_node_depth++;
}

void oxws_find_item_sax_handler_end_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri) {
  UNUSED(prefix);

  if(user_data == NULL || localname == NULL) return;
  oxws_find_item_sax_context* context = (oxws_find_item_sax_context*) user_data;
  if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ERROR()) return;

  if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(START_DOCUMENT, SOAP, "Envelope")) {
    /* the end_document callback does not seem to get called. We emulate it
       using the end of the SOAP Envelope tag */
    oxws_find_item_sax_handler_end_document(user_data);

  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(ITEMS, EXCH_TYPES, "Items")) {
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(START_DOCUMENT);

  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ANY_ITEM_TOP_LEVEL()) {
    /* end of current item, add it to the result list */
    if(context->item != NULL) {
      carray_add(*context->list, context->item, NULL);
      context->item = NULL; /* not freeing the item on purpose, because it's in the list now */
      context->item_node_depth = 0;
      OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(ITEMS);
    } else {
      /* no current item */
      /* TODO warn */
    }

  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(ITEM_ITEM_ID, EXCH_TYPES, "ItemId")) {
    OXWS_FIND_ITEM_SAX_CONTEXT_POP_STATE();
  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(ITEM_SUBJECT, EXCH_TYPES, "Subject")) {
    OXWS_FIND_ITEM_SAX_CONTEXT_ASSIGN_STRING_TO_ITEM(subject);
    OXWS_FIND_ITEM_SAX_CONTEXT_POP_STATE();
  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(ITEM_SIZE, EXCH_TYPES, "Size")) {
    /* TODO check context and string */
    int size = atoi(context->string->str);
    oxws_item_set_size(context->item, size);
    /* TODO warn if result != NO_ERROR */
    OXWS_FIND_ITEM_SAX_CONTEXT_FREE_STRING();
    OXWS_FIND_ITEM_SAX_CONTEXT_POP_STATE();

  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(MESSAGE_FROM, EXCH_TYPES, "From")) {
    OXWS_FIND_ITEM_SAX_CONTEXT_ASSIGN_EMAIL_ADDRESS_TO_MESSAGE(from);
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(MESSAGE);
  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(MESSAGE_IS_READ, EXCH_TYPES, "IsRead")) {
    /* TODO check item and string */
    const char* is_read = context->string->str;
    if(strcmp(is_read, "true") == 0 || strcmp(is_read, "1") == 0) {
      oxws_message_set_is_read((oxws_message*) context->item, OXWS_OPTIONAL_BOOLEAN_TRUE);
    } else if(strcmp(is_read, "false") == 0 || strcmp(is_read, "0") == 0) {
      oxws_message_set_is_read((oxws_message*) context->item, OXWS_OPTIONAL_BOOLEAN_FALSE);
    } else {
      /* TODO warn */
    }
    /* TODO warn if result != NO_ERROR */
    OXWS_FIND_ITEM_SAX_CONTEXT_FREE_STRING();
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(MESSAGE);

  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(INNER_MAILBOX, EXCH_TYPES, "Mailbox")) {
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(MESSAGE_FROM);
  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(INNER_MAILBOX_NAME, EXCH_TYPES, "Name")) {
    OXWS_FIND_ITEM_SAX_CONTEXT_ASSIGN_CSTRING_TO_EMAIL_ADDRESS(name);
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(INNER_MAILBOX);
  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(INNER_MAILBOX_EMAIL_ADDRESS, EXCH_TYPES, "EmailAddress")) {
    OXWS_FIND_ITEM_SAX_CONTEXT_ASSIGN_CSTRING_TO_EMAIL_ADDRESS(email_address);
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(INNER_MAILBOX);
  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(INNER_MAILBOX_ROUTING_TYPE, EXCH_TYPES, "RoutingType")) {
    OXWS_FIND_ITEM_SAX_CONTEXT_ASSIGN_CSTRING_TO_EMAIL_ADDRESS(routing_type);
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(INNER_MAILBOX);
  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(INNER_MAILBOX_MAILBOX_TYPE, EXCH_TYPES, "MailboxType")) {
    /* TODO check email_address and string */
    oxws_mailbox_type mailbox_type = OXWS_MAILBOX_TYPE__NOT_SET;
    if(strcmp(context->string->str, "Mailbox") == 0) mailbox_type = OXWS_MAILBOX_TYPE_MAILBOX;
    else if(strcmp(context->string->str, "PublicDL") == 0) mailbox_type = OXWS_MAILBOX_TYPE_PUBLIC_DL;
    else if(strcmp(context->string->str, "PrivateDL") == 0) mailbox_type = OXWS_MAILBOX_TYPE_PRIVATE_DL;
    else if(strcmp(context->string->str, "Contact") == 0) mailbox_type = OXWS_MAILBOX_TYPE_CONTACT;
    else if(strcmp(context->string->str, "PublicFolder") == 0) mailbox_type = OXWS_MAILBOX_TYPE_PUBLIC_FOLDER;
    if(mailbox_type == OXWS_MAILBOX_TYPE__NOT_SET) {
      /* TODO warn */
    } else {
      oxws_email_address_set_mailbox_type(context->email_address, mailbox_type);
      /* TODO warn if result != NO_ERROR */
    }
    OXWS_FIND_ITEM_SAX_CONTEXT_FREE_STRING();
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(INNER_MAILBOX);
  } else if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(INNER_MAILBOX_ITEM_ID, EXCH_TYPES, "ItemId")) {
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(INNER_MAILBOX);

  } else {
    /* TODO warn for unknown tags */
    /* TODO go to error state for invalid state-tag combinations */
  }

  if(context->item != NULL)
    context->item_node_depth--; /* TODO warn if item_node_depth == 0 */
}

void oxws_find_item_sax_handler_characters(void* user_data, const xmlChar* chars, int length) {

  if(user_data == NULL || chars == NULL) return;
  oxws_find_item_sax_context* context = (oxws_find_item_sax_context*) user_data;
  if(OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ERROR()) return;

  if(context->string != NULL) {
    mmap_string_append_len(context->string, (const char*) chars, length);
  } else {
    /* TODO warn for suspicious states? */
  }
}

void oxws_find_item_sax_handler_error(void* user_data, const char* message, ...) {
  /* TODO log error message */
  UNUSED(message);

  if(user_data == NULL) return;
  oxws_find_item_sax_context* context = (oxws_find_item_sax_context*) user_data;
  OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(_ERROR);
}

xmlSAXHandler oxws_find_item_sax_handler = {
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
  oxws_find_item_sax_handler_start_document,
  oxws_find_item_sax_handler_end_document,
  NULL, /* startElement */
  NULL, /* endElement */
  NULL, /* reference */
  oxws_find_item_sax_handler_characters,
  NULL, /* ignorableWhitespace */
  NULL, /* processingInstruction */
  NULL, /* comment */
  NULL, /* TODO warning */
  oxws_find_item_sax_handler_error, /* error */
  oxws_find_item_sax_handler_error, /* fatalError */
  NULL, /* getParameterEntity */
  NULL, /* cdataBlock */
  NULL, /* externalSubset */
  XML_SAX2_MAGIC, /* initialized */
  NULL, /* _private */
  oxws_find_item_sax_handler_start_element_ns,
  oxws_find_item_sax_handler_end_element_ns,
  NULL, /* serror */
};
