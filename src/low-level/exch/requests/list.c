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

#include "list.h"
#include <libetpan/mailexch_requests.h>
#include "helper.h"
#include "types_internal.h"
#include "xml.h"

#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>


mailexch_result mailexch_list(mailexch* exch,
        mailexch_distinguished_folder_id distfolder_id, const char* folder_id,
        int count, carray** list) {

  /* check parameters */
  if(list == NULL)
    return MAILEXCH_ERROR_INVALID_PARAMETER;
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

  /* configure response XML parser */
  mailexch_list_sax_context sax_context;
  mailexch_list_sax_context_init(&sax_context, count > 0 ? count : 10, list);
  if(mailexch_handle_response_xml(exch, &mailexch_list_sax_handler, &sax_context) != MAILEXCH_NO_ERROR) {
    mailexch_release_response_xml_parser(exch);
    return MAILEXCH_ERROR_INTERNAL;
  }

  /* perform request. the SAX handler will fill the list */
  int result = mailexch_perform_request_xml(exch, node_findItem);
  if(sax_context.state == MAILEXCH_LIST_SAX_CONTEXT_STATE__ERROR) {
    /* TODO set result */
    puts(">>> error");
  }

  /* clean up */
  if(result != MAILEXCH_NO_ERROR) {
    mailexch_type_item_array_free(*list);
    *list = NULL;
  }
  mailexch_release_response_xml_parser(exch);
  return result;
}


void mailexch_list_sax_context_init(mailexch_list_sax_context* context, unsigned int count, carray** list) {
  memset(context, 0, sizeof(mailexch_list_sax_context));
  context->count = count;
  context->list = list;
}


void mailexch_list_sax_handler_start_document(void* user_data) {
  mailexch_list_sax_context* context = (mailexch_list_sax_context*) user_data;
  if(context->state != MAILEXCH_LIST_SAX_CONTEXT_STATE__NONE)
    context->state = MAILEXCH_LIST_SAX_CONTEXT_STATE__ERROR;
  if(context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE__ERROR) return;

  *context->list = carray_new(context->count);

  context->state = MAILEXCH_LIST_SAX_CONTEXT_STATE_START_DOCUMENT;
}

void mailexch_list_sax_handler_end_document(void* user_data) {
  mailexch_list_sax_context* context = (mailexch_list_sax_context*) user_data;
  /* TODO check state */
  if(context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE__ERROR) return;
  context->prev_state = context->state;
  context->state = MAILEXCH_LIST_SAX_CONTEXT_STATE_END_DOCUMENT;
}

void mailexch_list_sax_handler_start_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri,
        int nb_namespaces, const xmlChar** namespaces,
        int nb_attributes, int nb_defaulted, const xmlChar** attrs) {
  UNUSED(prefix);
  UNUSED(nb_namespaces);
  UNUSED(namespaces);
  UNUSED(nb_defaulted);

  mailexch_list_sax_context* context = (mailexch_list_sax_context*) user_data;
  if(context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE__ERROR) return;

  int attr_index;

  /* TODO m:FindItemResponseMessage, m:ResponseCode, m:RootFolder,
          other item classes, multiple response messages */
  if(context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE_START_DOCUMENT &&
     xmlStrcmp(ns_uri, MAILEXCH_XML_NS_EXCH_TYPES) == 0 &&
     xmlStrcmp(localname, BAD_CAST "Items") == 0) {
    /* TODO check items_node_depth */
    context->state = MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEMS;

  } else if(context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEMS) {
    if(context->item) { /* TODO warn */ }
    if(xmlStrcmp(ns_uri, MAILEXCH_XML_NS_EXCH_TYPES) == 0 &&
       xmlStrcmp(localname, BAD_CAST "Message") == 0) {
      context->state = MAILEXCH_LIST_SAX_CONTEXT_STATE_MESSAGE;
      context->item = (mailexch_type_item*) calloc(1, sizeof(mailexch_type_message));
    } else {
      context->state = MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEM;
      context->item = (mailexch_type_item*) calloc(1, sizeof(mailexch_type_item));
    }
    context->item_node_depth = 1;

  } else if((context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE_MESSAGE ||
     context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEM) &&
     context->item_node_depth == 1 &&
     xmlStrcmp(ns_uri, MAILEXCH_XML_NS_EXCH_TYPES) == 0 &&
     xmlStrcmp(localname, BAD_CAST "ItemId") == 0) {
    /* TODO check item */
    context->item->item_id = (mailexch_type_item_id*) calloc(1, sizeof(mailexch_type_item_id));
    for(attr_index = 0; attr_index < nb_attributes; attr_index++) {
      const xmlChar* name = attrs[5 * attr_index + 0];
      const xmlChar* value = attrs[5 * attr_index + 3];
      const xmlChar* end = attrs[5 * attr_index + 4];
      if(xmlStrcmp(name, BAD_CAST "Id") == 0) {
        context->item->item_id->id = (char*) xmlStrndup(value, end - value);
      } else if(xmlStrcmp(name, BAD_CAST "ChangeKey") == 0) {
        context->item->item_id->change_key = (char*) xmlStrndup(value, end - value);
      }
      /* TODO warn for unknown attributes */
    }
    context->item_node_depth++;

  } else if((context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE_MESSAGE ||
     context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEM) &&
     context->item_node_depth == 1 &&
     xmlStrcmp(ns_uri, MAILEXCH_XML_NS_EXCH_TYPES) == 0 &&
     xmlStrcmp(localname, BAD_CAST "Subject") == 0) {
    /* TODO check item */
    context->prev_state = context->state;
    context->state = MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEM_SUBJECT;
    context->item_node_depth++;

  } else if(context->item_node_depth > 0) {
    context->item_node_depth++;

  } else {
    /* TODO warn for unknown tags */
    /* TODO go to error state for invalid state-tag combinations */
  }
}

void mailexch_list_sax_handler_end_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri) {
  UNUSED(prefix);

  mailexch_list_sax_context* context = (mailexch_list_sax_context*) user_data;
  if(context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE__ERROR) return;

  if(context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEMS &&
     xmlStrcmp(ns_uri, MAILEXCH_XML_NS_EXCH_TYPES) == 0 &&
     xmlStrcmp(localname, BAD_CAST "Items") == 0) {
    context->state = MAILEXCH_LIST_SAX_CONTEXT_STATE_START_DOCUMENT;

  } else if(context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEM_SUBJECT &&
     xmlStrcmp(ns_uri, MAILEXCH_XML_NS_EXCH_TYPES) == 0 &&
     xmlStrcmp(localname, BAD_CAST "Subject") == 0) {
    context->state = context->prev_state;
    context->prev_state = MAILEXCH_LIST_SAX_CONTEXT_STATE__NONE;

  } else {
    /* TODO warn for unknown tags */
    /* TODO go to error state for invalid state-tag combinations */
  }

  if(context->item_node_depth > 0) {
    context->item_node_depth--;
    if(context->item_node_depth == 0) {
      /* end of current item, add it to the result list */
      /* TODO check item */
      carray_add(*context->list, context->item, NULL);
      context->item = NULL;
      /* not freeing the item on purpose, because it's in the list now */
      context->state = MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEMS;
    }
  }
}

void mailexch_list_sax_handler_characters(void* user_data,
        const xmlChar* chars, int length) {

  mailexch_list_sax_context* context = (mailexch_list_sax_context*) user_data;
  if(context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE__ERROR)return;

  if(context->state == MAILEXCH_LIST_SAX_CONTEXT_STATE_ITEM_SUBJECT) {
    /* TODO check item */
    xmlChar* subject = (xmlChar*) context->item->subject;
    if(subject) {
      if(length > 0)
        subject = xmlStrncat(subject, chars, length);
    } else {
      subject = xmlStrndup(chars, length);
    }
    context->item->subject = (char*) subject;
  }
}

xmlSAXHandler mailexch_list_sax_handler = {
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
  mailexch_list_sax_handler_start_document,
  mailexch_list_sax_handler_end_document,
  NULL, /* startElement */
  NULL, /* endElement */
  NULL, /* reference */
  mailexch_list_sax_handler_characters,
  NULL, /* ignorableWhitespace */
  NULL, /* processingInstruction */
  NULL, /* comment */
  NULL, /* TODO warning */
  NULL, /* TODO error */
  NULL, /* TODO fatalError */
  NULL, /* getParameterEntity */
  NULL, /* cdataBlock */
  NULL, /* externalSubset */
  XML_SAX2_MAGIC, /* initialized */
  NULL, /* _private */
  mailexch_list_sax_handler_start_element_ns,
  mailexch_list_sax_handler_end_element_ns,
  NULL, /* serror */
};
