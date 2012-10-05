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

#ifndef OXWS_REQUESTS_CREATE_ITEM_H
#define OXWS_REQUESTS_CREATE_ITEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libetpan/oxws_types.h>
#include <libetpan/oxws_types_item.h>

#include <libxml/parser.h>


/*
  enum oxws_create_item_sax_context_state

  States supported by the SAX context object for parsing the create_item
  response.

  @see oxws_create_item()
  @see struct oxws_create_item_sax_context
*/
enum oxws_create_item_sax_context_state {
  /* State not initialized, i.e. parsing did not begin yet */
  OXWS_CREATE_ITEM_SAX_CONTEXT_STATE__NONE = 0,
  /* Error state, will not continue parsing */
  OXWS_CREATE_ITEM_SAX_CONTEXT_STATE__ERROR = -1,

  /* Start of document has been parsed */
  OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_START_DOCUMENT = 1,
  /* Inside the Items tag */
  OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_ITEMS,
  /* Inside any item tag */
  OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_ITEM,

  /* End of document has been parsed, used to identify whether the response was
     parsed completely */
  OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_END_DOCUMENT,
};
typedef enum oxws_create_item_sax_context_state oxws_create_item_sax_context_state;


/*
  struct oxws_create_item_sax_context

  SAX context object used to track the status of response parsing.
  It is passed as user data to the SAX handler functions.

  @see oxws_create_item()
  @see oxws_create_item_sax_handler_*()
*/
struct oxws_create_item_sax_context {
  /* the item whose id to configure upon success */
  oxws_item* item;

  /* current parser state, @see oxws_create_item_sax_context_state */
  oxws_create_item_sax_context_state state;

  /* If we are inside an item tag of any class, this value counts the current
     node depth. This is neccessary so we know when to go back to the ITEMS
     state even if we parse unknown tags. */
  unsigned int item_node_depth;
};
typedef struct oxws_create_item_sax_context oxws_create_item_sax_context;

/*
  oxws_create_item_sax_context_init()

  Initialize SAX context for the list response with given list output pointer
  and initial list size. All other properties are cleared.

  @param context  [required] context to initialize

  @return - OXWS_ERROR_INVALID_PARAMETER: a required parameter is missing
          - OXWS_NO_ERROR: success
*/
oxws_result oxws_create_item_sax_context_init(oxws_create_item_sax_context* context, oxws_item* item);


/*
  routines for SAX context state testing and modification, and context preparation
*/

#define OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS(test_state) \
  (context->state == OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_##test_state)

#define OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS_ERROR() \
  OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS(_ERROR)

#define OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS_ITEM_TOP_LEVEL() \
  (OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS(ITEM) && context->item_node_depth == 1)

#define OXWS_CREATE_ITEM_SAX_IS_NS_NODE(expected_ns_suffix, expected_name) \
  (xmlStrcmp(ns_uri, OXWS_XML_NS_##expected_ns_suffix) == 0 && xmlStrcmp(localname, BAD_CAST expected_name) == 0)

#define OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_MATCHES_TAG(test_state, tag_ns, tag_name) \
  (OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_IS(test_state) && \
  OXWS_CREATE_ITEM_SAX_IS_NS_NODE(tag_ns, tag_name))


#define OXWS_CREATE_ITEM_SAX_CONTEXT_SET_STATE(new_state) \
  context->state = OXWS_CREATE_ITEM_SAX_CONTEXT_STATE_##new_state


/* TODO duplicate of OXWS_FIND_ITEM_SAX_CONTEXT_PARSE_ITEM_ID */
#define OXWS_CREATE_ITEM_SAX_CONTEXT_PARSE_ITEM_ID(id, change_key) \
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
  oxws_create_item_sax_handler_start_document()

  Called on document start. The current state must be _NONE.
  Sets state to START_DOCUMENT.

  @param user_data [required] the oxws_create_item_sax_context
*/
void oxws_create_item_sax_handler_start_document(void* user_data);

/*
  oxws_create_item_sax_handler_end_document()

  Called on document end. Sets state to END_DOCUMENT.

  @param user_data [required] the oxws_create_item_sax_context

  @note TODO check state
*/
void oxws_create_item_sax_handler_end_document(void* user_data);

/*
  oxws_create_item_sax_handler_start_element_ns()

  Called on element start. The following state transitions are supported:
  @note TODO

  @param user_data [required] the oxws_create_item_sax_context

  @see libxml documentation
*/
void oxws_create_item_sax_handler_start_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri,
        int nb_namespaces, const xmlChar** namespaces,
        int nb_attributes, int nb_defaulted, const xmlChar** attrs);

/*
  oxws_create_item_sax_handler_end_element_ns()

  Called on element end. The following state transitions are supported:
  @note TODO
  * START_DOCUMENT + soap:Envelope -> END_DOCUMENT; this is to work around a bug
                                      where the end_document callback does not
                                      get called, but we still need to signal
                                      the end of the document

  @param user_data [required] the oxws_create_item_sax_context

  @see libxml documentation
*/
void oxws_create_item_sax_handler_end_element_ns(void* user_data,
        const xmlChar* localname, const xmlChar* prefix, const xmlChar* ns_uri);

/*
  oxws_create_item_sax_handler_characters()

  Called when parsing element text. Appends the given string to context->string
  if it is initialized.

  @param user_data [required] the oxws_create_item_sax_context

  @see libxml documentation
*/
void oxws_create_item_sax_handler_characters(void* user_data, const xmlChar* chars, int length);

/*
  oxws_create_item_sax_handler_error()

  Called on parser error. Enters the _ERROR state.

  @param user_data [required] the oxws_create_item_sax_context

  @see libxml documentation

  @note TODO log error message
*/
void oxws_create_item_sax_handler_error(void* user_data, const char* message, ...);

/*
  oxws_create_item_sax_handler

  A SAX 2 handler with the startDocument, endDocument, characters, error,
  fatalError, startElementNs and endElementNs callbacks from this module.

  @see oxws_create_item_sax_handler_start_document()
  @see oxws_create_item_sax_handler_end_document()
  @see oxws_create_item_sax_handler_characters()
  @see oxws_create_item_sax_handler_error()
  @see oxws_create_item_sax_handler_start_element_ns()
  @see oxws_create_item_sax_handler_end_element_ns()
*/
extern xmlSAXHandler oxws_create_item_sax_handler;


#ifdef __cplusplus
}
#endif

#endif
