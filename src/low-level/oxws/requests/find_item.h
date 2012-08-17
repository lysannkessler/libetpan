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

#ifndef OXWS_REQUESTS_FIND_ITEM_H
#define OXWS_REQUESTS_FIND_ITEM_H

#ifdef __cplusplus
extern "C" {
#endif


#include <libetpan/carray.h>
#include <libetpan/oxws_types.h>
#include <libetpan/oxws_types_item.h>

#include <libxml/parser.h>


/*
  enum oxws_find_item_sax_context_state

  states supported by the SAX context object for parsing the list response

  @see oxws_list()
  @see struct oxws_find_item_sax_context
*/
enum oxws_find_item_sax_context_state {
  /* State not initialized, i.e. parsing did not begin yet */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE__NONE = 0,
  /* Error state, will not continue parsing */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE__ERROR = -1,

  /* Start of document has been parsed, *list is initialized */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_START_DOCUMENT = 1,
  /* Inside the Items tag */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_ITEMS,
  /* Entered an unknown item tag; unknown items are tags right inside the Items
     tag, but are none of the item classes with an own state. */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_ITEM,
  /* Inside a message item */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MESSAGE,

  /* Inside the ItemId of any item */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_ITEM_ITEM_ID,
  /* Inside the Size of any item */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_ITEM_SIZE,
  /* Inside the Subject tag of any item */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_ITEM_SUBJECT,

  /* Inside the From tag of a message. We could also be deeper in the graph, see
     the OXWS_FIND_ITEM_SAX_CONTEXT_STATE_INNER_MAILBOX* states */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MESSAGE_FROM,
  /* Inside the IsRead tag of a message */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MESSAGE_IS_READ,

  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_INNER_MAILBOX,
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_INNER_MAILBOX_NAME,
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_INNER_MAILBOX_EMAIL_ADDRESS,
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_INNER_MAILBOX_ROUTING_TYPE,
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_INNER_MAILBOX_MAILBOX_TYPE,
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_INNER_MAILBOX_ITEM_ID,

  /* End of document has been parsed, used to identify whether the response was
     parsed completely */
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_END_DOCUMENT,
};
typedef enum oxws_find_item_sax_context_state oxws_find_item_sax_context_state;


/*
  struct oxws_find_item_sax_context

  SAX context object used to track the status of response parsing.
  It is passed as user data to the SAX handler functions.

  @see oxws_list()
  @see oxws_find_item_sax_handler_*()
*/
struct oxws_find_item_sax_context {
  /* Expected number of items in the response. Will be used as initial list
     size */
  unsigned int count;
  /* Will receive the list containing the parsed items. Initialized on
     startDocument. Is not cleared if an error occurs later on. */
  carray** list;

  /* Helper variable containing the one previous state while inside an item
     subtag. Once we leave the subtag, state is reset to prev_state.
     This is neccessary because the subtag could be inside an item tag of any
     class.
     @see oxws_find_item_sax_context_state */
  oxws_find_item_sax_context_state prev_state;
  /* current parser state, @see oxws_find_item_sax_context_state */
  oxws_find_item_sax_context_state state;

  /* If we are currently inside an item tag, this is the item being parsed. It
     will be appended to the list once we leave the current item tag. */
  oxws_item* item;
  /* If we are inside a item tag of any class, this value counts the current
     node depth. This is neccessary so we know when to go back to the ITEMS
     state even if we parse unknown tags. */
  unsigned int item_node_depth;

  MMAPString* string;
  oxws_email_address* email_address;
};
typedef struct oxws_find_item_sax_context oxws_find_item_sax_context;

/*
  oxws_find_item_sax_context_init()

  Initialize SAX context for the list response with given list output pointer
  and initial list size. All other properties are cleared.

  @param context  [required] context to initialize
  @param count    (see oxws_find_item_sax_context.count)
  @param list     [required] (see oxws_find_item_sax_context.list)

  @return - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing
          - OXWS_NO_ERROR: success
*/
oxws_result oxws_find_item_sax_context_init(oxws_find_item_sax_context* context, unsigned int count, carray** list);


/*
  routines for SAX context state testing and modification, and context preparation
*/

#define OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(test_state) \
  (context->state == OXWS_FIND_ITEM_SAX_CONTEXT_STATE_##test_state)

#define OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ERROR() \
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(_ERROR)

#define OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ANY_ITEM() \
  (OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(ITEM) || \
   OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(MESSAGE))

#define OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ANY_ITEM_TOP_LEVEL() \
  (OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ANY_ITEM() && context->item_node_depth == 1)

#define OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS_ANY_EMAIL_ADDRESS() \
  OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(MESSAGE_FROM)

#define OXWS_FIND_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(test_state, tag_ns, tag_name) \
  (OXWS_FIND_ITEM_SAX_CONTEXT_STATE_IS(test_state) && \
  OXWS_FIND_ITEM_SAX_IS_NS_NODE(ns_uri, localname, tag_ns, tag_name))


#define OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(new_state) \
  context->state = OXWS_FIND_ITEM_SAX_CONTEXT_STATE_##new_state

#define OXWS_FIND_ITEM_SAX_CONTEXT_PUSH_STATE(new_state) \
  { \
    context->prev_state = context->state; \
    OXWS_FIND_ITEM_SAX_CONTEXT_SET_STATE(new_state); \
  }

#define OXWS_FIND_ITEM_SAX_CONTEXT_POP_STATE() \
  { \
    context->state = context->prev_state; \
    context->prev_state = OXWS_FIND_ITEM_SAX_CONTEXT_STATE__NONE; \
  }


#define OXWS_FIND_ITEM_SAX_CONTEXT_PREPARE_STRING(initial_length) \
  { \
    if(context->string != NULL) { \
      /* TODO warn */ \
      mmap_string_free(context->string); \
    } \
    context->string = mmap_string_sized_new(initial_length); \
  }

#define OXWS_FIND_ITEM_SAX_CONTEXT_PREPARE_EMAIL_ADDRESS() \
  { \
    if(context->email_address != NULL) { \
      /* TODO warn */ \
      oxws_email_address_free(context->email_address); \
    } \
    context->email_address = oxws_email_address_new(); \
  }


#define OXWS_FIND_ITEM_SAX_CONTEXT_FREE_STRING() \
  { \
    mmap_string_free(context->string); \
    context->string = NULL; \
  }

#define OXWS_FIND_ITEM_SAX_CONTEXT_ASSIGN_STRING_TO_ITEM(property) \
  { \
    /* TODO check item and string */ \
    oxws_item_set_##property(context->item, context->string); \
    /* TODO warn if result != NO_ERROR */ \
    context->string = NULL; /* not freed because it is assigned to the item now */ \
  }

#define OXWS_FIND_ITEM_SAX_CONTEXT_ASSIGN_CSTRING_TO_ITEM(property) \
  { \
    /* TODO check item and string */ \
    oxws_item_set_##property(context->item, context->string->str); \
    /* TODO warn if result != NO_ERROR */ \
    OXWS_FIND_ITEM_SAX_CONTEXT_FREE_STRING(); \
  }

#define OXWS_FIND_ITEM_SAX_CONTEXT_ASSIGN_CSTRING_TO_EMAIL_ADDRESS(property) \
  { \
    /* TODO check email_address and string */ \
    oxws_email_address_set_##property(context->email_address, context->string->str); \
    /* TODO warn if result != NO_ERROR */ \
    context->string = NULL; /* not freed because it is assigned to the address now */ \
  }

#define OXWS_FIND_ITEM_SAX_CONTEXT_ASSIGN_EMAIL_ADDRESS_TO_ITEM(property) \
  { \
    /* TODO check item and email_address */ \
    oxws_item_set_##property(context->item, context->email_address); \
    /* TODO warn if result != NO_ERROR */ \
    context->email_address = NULL; /* not freed because it is assigned to the item now */ \
  }

#define OXWS_FIND_ITEM_SAX_CONTEXT_ASSIGN_EMAIL_ADDRESS_TO_MESSAGE(property) \
  { \
    /* TODO check item and email_address */ \
    oxws_message_set_##property((oxws_message*) context->item, context->email_address); \
    /* TODO warn if result != NO_ERROR */ \
    context->email_address = NULL; /* not freed because it is assigned to the item now */ \
  }

#define OXWS_FIND_ITEM_SAX_CONTEXT_PARSE_ITEM_ID(id, change_key) \
  { \
    int attr_index; \
    for(attr_index = 0; attr_index < nb_attributes; attr_index++) { \
      const xmlChar* name = attrs[5 * attr_index + 0]; \
      const xmlChar* value = attrs[5 * attr_index + 3]; \
      const xmlChar* end = attrs[5 * attr_index + 4]; \
      if(xmlStrcmp(name, BAD_CAST "Id") == 0) { \
        id = xmlStrndup(value, end - value); \
      } else if(xmlStrcmp(name, BAD_CAST "ChangeKey") == 0) { \
        change_key = xmlStrndup(value, end - value); \
      } \
      /* TODO warn for unknown attributes */ \
    } \
  }

/*
  SAX handlers
*/

/*
  oxws_find_item_sax_handler_start_document()

  Called on document start. The current state must be _NONE.
  Initializes *list and sets state to START_DOCUMENT.

  @param user_data [required] the oxws_find_item_sax_context
*/
void oxws_find_item_sax_handler_start_document(void* user_data);

/*
  oxws_find_item_sax_handler_end_document()

  Called on document end. Sets state to END_DOCUMENT.

  @param user_data [required] the oxws_find_item_sax_context

  @note TODO check state
*/
void oxws_find_item_sax_handler_end_document(void* user_data);

/*
  oxws_find_item_sax_handler_start_element_ns()

  Called on element start. The following state transitions are supported:
  * START_DOCUMENT + t:Items -> ITEMS
  * ITEMS + t:Message -> MESSAGE; allocates message in item and sets
                         item_node_depth to 1
  * ITEMS + (any) -> ITEM; allocates item in item and sets item_node_depth to 1
  * MESSAGE/ITEM + t:ItemId -> sets the current item's item_id
  * MESSAGE/ITEM + t:Subject -> ITEM_SUBJECT; saves old state in prev_state;
                                initializes context->string
  * MESSAGE/ITEM + t:Size -> ITEM_SIZE; saves old state in prev_state;
                             initializes context->string
  Always increments item_node_depth if in MESSAGE/ITEM or beneath.

  @param user_data [required] the oxws_find_item_sax_context

  @see libxml documentation
*/
void oxws_find_item_sax_handler_start_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri,
        int nb_namespaces, const xmlChar** namespaces,
        int nb_attributes, int nb_defaulted, const xmlChar** attrs);

/*
  oxws_find_item_sax_handler_end_element_ns()

  Called on element end. The following state transitions are supported:
  * ITEMS + t:Items -> START_DOCUMENT
  * ITEM_SUBJECT + t:Subject -> restores prev_state; assigns context->string to
                                subject of context->item
  * ITEM_SIZE + t:Size -> restores prev_state; assigns context->string to size
                          of context->item
  * START_DOCUMENT + soap:Envelope -> END_DOCUMENT; this is to work around a bug
                                      where the end_document callback does not
                                      get called, but we still need to signal
                                      the end of the document
  Always decrements item_node_depth if > 0. If this results in == 0, then we
  leave the current item, restoring state to ITEMS and appending the current
  item to the result list.

  @param user_data [required] the oxws_find_item_sax_context

  @see libxml documentation
*/
void oxws_find_item_sax_handler_end_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri);

/*
  oxws_find_item_sax_handler_characters()

  Called when parsing element text. Appends the given string to context->string
  if it is initialized.

  @param user_data [required] the oxws_find_item_sax_context

  @see libxml documentation
*/
void oxws_find_item_sax_handler_characters(void* user_data, const xmlChar* chars, int length);

/*
  oxws_find_item_sax_handler_error()

  Called on parser error. Enters the _ERROR state.

  @param user_data [required] the oxws_find_item_sax_context

  @see libxml documentation

  @note TODO log error message
*/
void oxws_find_item_sax_handler_error(void* user_data, const char* message, ...);

/*
  oxws_find_item_sax_handler

  A SAX 2 handler with the startDocument, endDocument, characters, error,
  fatalError, startElementNs and endElementNs callbacks from this module.

  @see oxws_find_item_sax_handler_start_document()
  @see oxws_find_item_sax_handler_end_document()
  @see oxws_find_item_sax_handler_characters()
  @see oxws_find_item_sax_handler_error()
  @see oxws_find_item_sax_handler_start_element_ns()
  @see oxws_find_item_sax_handler_end_element_ns()
*/
extern xmlSAXHandler oxws_find_item_sax_handler;


#ifdef __cplusplus
}
#endif

#endif
